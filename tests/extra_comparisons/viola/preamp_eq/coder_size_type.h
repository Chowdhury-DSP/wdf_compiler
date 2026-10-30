/* Copyright 2026 The MathWorks, Inc. */
#ifndef _mw_coder_size_type_h
#define _mw_coder_size_type_h

#ifndef CODER_ARRAY_SIZE_TYPE_DEFINED
#if __cplusplus >= 201103L
namespace coder {
using SizeType = int;
}
#else
namespace coder {
typedef int SizeType;
}
#endif
#endif

#ifndef CODER_ARRAY_EMPTY_DEFINED
#define CODER_ARRAY_EMPTY_DEFINED

namespace coder {
struct empty {};
} // namespace coder
#endif
#endif
