#pragma once

#include <type_traits>

namespace wdf_lib
{
/** Useful structs for determining the internal data type of SIMD types */
namespace SampleTypeHelpers
{
    template <typename T, bool = std::is_floating_point<T>::value>
    struct ElementType
    {
        using Type = T;
    };

    template <typename T>
    struct ElementType<T, false>
    {
        using Type = typename T::value_type;
    };
} // namespace SampleTypeHelpers

/** Type alias for a SIMD numeric type */
template <typename T>
using NumericType = typename SampleTypeHelpers::ElementType<T>::Type;

/** Returns true if all the elements in a SIMD vector are equal */
inline bool all (bool x)
{
    return x;
}

/** Ternary select operation */
template <typename T>
inline T select (bool b, const T& t, const T& f)
{
    return b ? t : f;
}
} // namespace wdf_lib
