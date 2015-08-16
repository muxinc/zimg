#include <algorithm>
#include <memory>
#include "Common/align.h"
#include "Common/alloc.h"
#include "Common/linebuffer.h"
#include "Common/pair_filter.h"
#include "Common/pixel.h"
#include "resize2.h"
#include "resize_impl2.h"

namespace zimg {;
namespace resize {;

namespace {;

bool resize_h_first(double xscale, double yscale)
{
	double h_first_cost = std::max(xscale, 1.0) * 2.0 + xscale * std::max(yscale, 1.0);
	double v_first_cost = std::max(yscale, 1.0) + yscale * std::max(xscale, 1.0) * 2.0;

	return h_first_cost < v_first_cost;
}

class CopyFilter : public ZimgFilter {
	PixelType m_type;
public:
	explicit CopyFilter(PixelType type) :
		m_type{ type }
	{
	}

	ZimgFilterFlags get_flags() const override
	{
		ZimgFilterFlags flags{};

		flags.same_row = 1;
		flags.in_place = 1;

		return flags;
	}

	void process(void *, const ZimgImageBuffer *src, const ZimgImageBuffer *dst, void *, unsigned i, unsigned left, unsigned right) const override
	{
		LineBuffer<void> src_buf{ reinterpret_cast<char *>(src->data[0]) + left * pixel_size(m_type), right - left, (unsigned)src->stride[0], src->mask[0] };
		LineBuffer<void> dst_buf{ reinterpret_cast<char *>(dst->data[0]) + left * pixel_size(m_type), right - left, (unsigned)dst->stride[0], dst->mask[0] };

		copy_buffer_lines(src_buf, dst_buf, pixel_size(m_type) * (right - left), i, i + 1);
	}
};

} // namespace


Resize2::Resize2(const Filter &filter, PixelType type, int src_width, int src_height, int dst_width, int dst_height,
                 double shift_w, double shift_h, double subwidth, double subheight, CPUClass cpu)
try
{
	bool skip_h = (src_width == dst_width && shift_w == 0 && subwidth == src_width);
	bool skip_v = (src_height == dst_height && shift_h == 0 && subheight == src_height);

	if (skip_h && skip_v) {
		m_impl.reset(new CopyFilter{ type });
	} else if (skip_h) {
		m_impl.reset(create_resize_impl2(filter, type, false, src_height, dst_height, dst_height, shift_h, subheight, cpu));
	} else if (skip_v) {
		m_impl.reset(create_resize_impl2(filter, type, true, src_width, dst_width, dst_height, shift_w, subwidth, cpu));
	} else {
		bool h_first = resize_h_first((double)dst_width / src_width, (double)dst_height / src_height);
		PairFilter::builder builder{};
		std::unique_ptr<IZimgFilter> stage1;
		std::unique_ptr<IZimgFilter> stage2;
		unsigned tmp_width;
		unsigned tmp_height;

		if (h_first) {
			stage1.reset(create_resize_impl2(filter, type, true, src_width, dst_width, src_height, shift_w, subwidth, cpu));
			stage2.reset(create_resize_impl2(filter, type, false, src_height, dst_height, dst_height, shift_h, subheight, cpu));

			tmp_width = dst_width;
			tmp_height = src_height;
		} else {
			stage1.reset(create_resize_impl2(filter, type, false, src_height, dst_height, dst_height, shift_h, subheight, cpu));
			stage2.reset(create_resize_impl2(filter, type, true, src_width, dst_width, dst_height, shift_w, subwidth, cpu));

			tmp_width = src_width;
			tmp_height = dst_height;
		}

		builder.first = stage1.get();
		builder.second = stage2.get();
		builder.first_width = tmp_width;
		builder.first_height = tmp_height;
		builder.first_pixel = type;
		builder.second_width = dst_width;
		builder.second_height = dst_height;
		builder.second_pixel = type;

		m_impl.reset(new PairFilter{ builder });
		stage1.release();
		stage2.release();
	}
} catch (const std::bad_alloc &) {
	throw ZimgOutOfMemory{};
}

ZimgFilterFlags Resize2::get_flags() const
{
	return m_impl->get_flags();
}

IZimgFilter::pair_unsigned Resize2::get_required_row_range(unsigned i) const
{
	return m_impl->get_required_row_range(i);
}

IZimgFilter::pair_unsigned Resize2::get_required_col_range(unsigned left, unsigned right) const
{
	return m_impl->get_required_col_range(left, right);
}

unsigned Resize2::get_simultaneous_lines() const
{
	return m_impl->get_simultaneous_lines();
}

unsigned Resize2::get_max_buffering() const
{
	return m_impl->get_max_buffering();
}

size_t Resize2::get_context_size() const
{
	return m_impl->get_context_size();
}

size_t Resize2::get_tmp_size(unsigned left, unsigned right) const
{
	return m_impl->get_tmp_size(left, right);
}

void Resize2::init_context(void *ctx) const
{
	m_impl->init_context(ctx);
}

void Resize2::process(void *ctx, const ZimgImageBuffer *src, const ZimgImageBuffer *dst, void *tmp, unsigned i, unsigned left, unsigned right) const
{
	m_impl->process(ctx, src, dst, tmp, i, left, right);
}

} // namespace resize
} // namespace zimg
