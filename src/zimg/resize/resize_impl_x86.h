#pragma once

#ifdef ZIMG_X86

#ifndef ZIMG_RESIZE_RESIZE_IMPL_X86_H_
#define ZIMG_RESIZE_RESIZE_IMPL_X86_H_

namespace zimg {;

enum class CPUClass;
enum class PixelType;

namespace graph {;

class ImageFilter;

} // namespace graph


namespace resize {;

struct FilterContext;

graph::ImageFilter *create_resize_impl_h_sse(const FilterContext &context, unsigned height, PixelType type, unsigned depth);

graph::ImageFilter *create_resize_impl_v_sse(const FilterContext &context, unsigned width, PixelType type, unsigned depth);

graph::ImageFilter *create_resize_impl_v_sse2(const FilterContext &context, unsigned width, PixelType type, unsigned depth);

graph::ImageFilter *create_resize_impl_h_x86(const FilterContext &context, unsigned height, PixelType type, unsigned depth, CPUClass cpu);

graph::ImageFilter *create_resize_impl_v_x86(const FilterContext &context, unsigned width, PixelType type, unsigned depth, CPUClass cpu);

} // namespace resize
} // namespace zimg

#endif // ZIMG_RESIZE_RESIZE_IMPL_X86_H_

#endif // ZIMG_X86
