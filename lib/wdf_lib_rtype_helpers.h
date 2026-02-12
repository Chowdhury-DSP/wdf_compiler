#pragma once

#if defined(__AVX2__)
#include <immintrin.h>
#elif defined(__SSE2__)
#include <immintrin.h>
#elif defined(__ARM_NEON__)
#include <arm_neon.h>
#endif

#include <utility>

namespace wdf_lib
{
template <typename T1, typename T2>
constexpr T1 ceiling_divide (T1 num, T2 den)
{
    return (num + (T1) den - 1) / (T1) den;
}

template <typename T1, typename T2>
constexpr T1 pad_to_multiple (T1 value, T2 multiple)
{
    return ceiling_divide (value, multiple) * (T1) multiple;
}

#define WDF_LIB_RESTRICT __restrict__

template <int num_ports, int num_ports_padded>
inline static void unaligned_matmul (const float* WDF_LIB_RESTRICT S,
                                     const float* WDF_LIB_RESTRICT a,
                                     float* WDF_LIB_RESTRICT b)
{
    for (int c = 0; c < num_ports; ++c)
    {
        b[c] = S[c] * a[0];
        for (int r = 1; r < num_ports; ++r)
            b[c] += S[r * num_ports_padded + c] * a[r];
    }
}

// S: array of size (num_ports * num_ports_padded)
// a: array of size (num_ports)
// b: array of size (num_ports_padded)
template <int num_ports, int num_ports_padded = num_ports>
inline static void aligned_matmul (const float* WDF_LIB_RESTRICT S,
                                   const float* WDF_LIB_RESTRICT a,
                                   float* WDF_LIB_RESTRICT b)
{
// #if defined(__AVX2__)
    // @TODO: what's the right way to support AVX here?
#if defined(__SSE2__) // SSE
static constexpr int simd_size = 4;
static_assert (num_ports_padded % simd_size == 0, "num_ports_padded must be a multiple of 4!");

for (int c = 0; c < num_ports_padded; c += simd_size)
{
    auto S_r = _mm_load_ps (S + c);
    auto a_r = _mm_set1_ps (a[0]);
    auto b_c = _mm_mul_ps (a_r, S_r);
    for (int r = 1; r < num_ports; ++r)
    {
        S_r = _mm_load_ps (S + r * num_ports_padded + c);
        a_r = _mm_set1_ps (a[r]);
        b_c = _mm_add_ps (b_c, _mm_mul_ps (a_r, S_r));
    }
    _mm_store_ps (b + c, b_c);
}

#elif defined(__ARM_NEON__) // NEON
    // @TODO: we should re-arrange the matrix data so this can be done more optimally

    static constexpr int simd_size = 4;
    static_assert (num_ports_padded % simd_size == 0, "num_ports_padded must be a multiple of 4!");

    for (int c = 0; c < num_ports_padded; c += simd_size)
    {
        auto S_r = vld1q_f32 (S + c);
        auto a_r = vld1q_dup_f32 (a);
        auto b_c = vmulq_f32 (a_r, S_r);
        for (int r = 1; r < num_ports; ++r)
        {
            S_r = vld1q_f32 (S + r * num_ports_padded + c);
            a_r = vld1q_dup_f32 (a + r);
            b_c = vfmaq_f32 (b_c, a_r, S_r);
        }
        vst1q_f32 (b + c, b_c);
    }
#else // No SIMD
    unaligned_matmul (S, a, b);
#endif
}
}
