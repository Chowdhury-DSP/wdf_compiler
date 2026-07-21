#pragma once

// #include <stdint.h>

/**
 * Plain-C port of the Wright Omega approximations from wdf_lib_omega.h,
 * for use by the wdf_compiler C backend. Only the pieces needed to
 * evaluate omega4() (float-only) are ported.
 *
 * Reference: https://www.dafx.de/paper-archive/2019/DAFx2019_paper_5.pdf
 */

static inline float wdf_lib_omega_estrin3 (float a3, float a2, float a1, float a0, float x)
{
    const float t1 = a0 + a1 * x;
    const float t0 = a2 + a3 * x;
    return t1 + t0 * (x * x);
}

static inline float wdf_lib_omega_log2_approx (float x)
{
    return wdf_lib_omega_estrin3 (0.1640425613334452f, -1.098865286222744f, 3.148297929334117f, -2.213475204444817f, x);
}

static inline float wdf_lib_omega_pow2_approx (float x)
{
    return wdf_lib_omega_estrin3 (0.07944154167983575f, 0.2274112777602189f, 0.6931471805599453f, 1.0f, x);
}

static inline float wdf_lib_omega_log_approx (float x)
{
    union { int32_t i; float f; } v;
    v.f = x;
    int32_t ex = v.i & 0x7f800000;
    int32_t e = (ex >> 23) - 127;
    v.i = (v.i - ex) | 0x3f800000;

    return 0.693147180559945f * ((float) e + wdf_lib_omega_log2_approx (v.f));
}

static inline float wdf_lib_omega_exp_approx (float x)
{
    x = -126.0f > (1.442695040888963f * x) ? -126.0f : (1.442695040888963f * x);

    union { int32_t i; float f; } v;

    const int32_t xi = (int32_t) x;
    const int32_t l = x < 0.0f ? xi - 1 : xi;
    const float f = x - (float) l;
    v.i = (l + 127) << 23;

    return v.f * wdf_lib_omega_pow2_approx (f);
}

/** Third-order approximation of the Wright Omega function */
static inline float wdf_lib_omega_omega3 (float x)
{
    const float x1 = -3.341459552768620f;
    const float x2 = 8.0f;
    const float a = -1.314293149877800e-3f;
    const float b = 4.775931364975583e-2f;
    const float c = 3.631952663804445e-1f;
    const float d = 6.313183464296682e-1f;

    if (x < x1) return 0.0f;
    if (x < x2) return wdf_lib_omega_estrin3 (a, b, c, d, x);
    return x - wdf_lib_omega_log_approx (x);
}

/** Fourth-order approximation of the Wright Omega function */
static inline float wdf_lib_omega_omega4 (float x)
{
    const float y = wdf_lib_omega_omega3 (x);
    return y - (y - wdf_lib_omega_exp_approx (x - y)) / (y + 1.0f);
}
