#pragma once

#include <cmath>

namespace custom_dp
{
/**
 * Useful approximations for evaluating the Wright Omega function.
 *
 * This approach was devloped by Stefano D'Angelo, and adapted from his
 * original implementation under the MIT license.
 * - Paper: https://www.dafx.de/paper-archive/2019/DAFx2019_paper_5.pdf
 * - Original Source: https://www.dangelo.audio/dafx2019-omega.html
 */
namespace Omega
{
/**
 * Evaluates a polynomial of a given order, using Estrin's scheme.
 * Coefficients should be given in the form { a_n, a_n-1, ..., a_1, a_0 }
 * https://en.wikipedia.org/wiki/Estrin%27s_scheme
 */
template <int ORDER, typename T, typename X>
inline typename std::enable_if<(ORDER == 1), decltype (T {} * X {})>::type estrin (const T (&coeffs)[ORDER + 1], const X& x)
{
    return coeffs[1] + coeffs[0] * x;
}

template <int ORDER, typename T, typename X>
inline typename std::enable_if<(ORDER > 1), decltype (T {} * X {})>::type estrin (const T (&coeffs)[ORDER + 1], const X& x)
{
    decltype (T {} * X {}) temp[ORDER / 2 + 1];
    for (int n = ORDER; n >= 0; n -= 2)
        temp[n / 2] = coeffs[n] + coeffs[n - 1] * x;

    temp[0] = (ORDER % 2 == 0) ? coeffs[0] : temp[0];

    return estrin<ORDER / 2> (temp, x * x); // recurse!
}

/** approximation for log_2(x), optimized on the range [1, 2] */
template <typename T>
constexpr T log2_approx (T x)
{
    constexpr auto alpha = (T) 0.1640425613334452;
    constexpr auto beta = (T) -1.098865286222744;
    constexpr auto gamma = (T) 3.148297929334117;
    constexpr auto zeta = (T) -2.213475204444817;

    return estrin<3> ({ alpha, beta, gamma, zeta }, x);
}

/** approximation for log(x) */
template <typename T>
constexpr T log_approx (T x);

/** approximation for log(x) (32-bit) */
template <>
constexpr float log_approx (float x)
{
    union
    {
        int32_t i;
        float f;
    } v {};
    v.f = x;
    int32_t ex = v.i & 0x7f800000;
    int32_t e = (ex >> 23) - 127;
    v.i = (v.i - ex) | 0x3f800000;

    return 0.693147180559945f * ((float) e + log2_approx<float> (v.f));
}

/** approximation for 2^x, optimized on the range [0, 1] */
template <typename T>
constexpr T pow2_approx (T x)
{
    constexpr auto alpha = (T) 0.07944154167983575;
    constexpr auto beta = (T) 0.2274112777602189;
    constexpr auto gamma = (T) 0.6931471805599453;
    constexpr auto zeta = (T) 1.0;

    return estrin<3> ({ alpha, beta, gamma, zeta }, x);
}

/** approximation for exp(x) */
template <typename T>
T exp_approx (T x);

/** approximation for exp(x) (32-bit) */
template <>
constexpr float exp_approx (float x)
{
    x = std::max (-126.0f, 1.442695040888963f * x);

    union
    {
        int32_t i;
        float f;
    } v {};

    auto xi = (int32_t) x;
    int32_t l = x < 0.0f ? xi - 1 : xi;
    float f = x - (float) l;
    v.i = (l + 127) << 23;

    return v.f * pow2_approx<float> (f);
}

/** Second-order approximation of the Wright Omega functions */
template <typename T>
constexpr T omega2 (T x)
{
    constexpr auto x1 = (NumericType<T>) -3.684303659906469;
    constexpr auto x2 = (NumericType<T>) 1.972967391708859;
    constexpr auto a = (NumericType<T>) 9.451797158780131e-3;
    constexpr auto b = (NumericType<T>) 1.126446405111627e-1;
    constexpr auto c = (NumericType<T>) 4.451353886588814e-1;
    constexpr auto d = (NumericType<T>) 5.836596684310648e-1;

    return select (x < x1, (T) 0, select (x > x2, x, estrin<3> ({ a, b, c, d }, x)));
}

/** Third-order approximation of the Wright Omega functions */
template <typename T>
constexpr T omega3 (T x)
{
    constexpr auto x1 = (NumericType<T>) -3.341459552768620;
    constexpr auto x2 = (NumericType<T>) 8.0;
    constexpr auto a = (NumericType<T>) -1.314293149877800e-3;
    constexpr auto b = (NumericType<T>) 4.775931364975583e-2;
    constexpr auto c = (NumericType<T>) 3.631952663804445e-1;
    constexpr auto d = (NumericType<T>) 6.313183464296682e-1;

    return select (x < x1, (T) 0, select (x < x2, estrin<3> ({ a, b, c, d }, x), x - log_approx<T> (x)));
}

/** Fourth-order approximation of the Wright Omega functions */
template <typename T>
constexpr T omega4 (T x)
{
    const auto y = omega3<T> (x);
    return y - (y - exp_approx<T> (x - y)) / (y + (T) 1);
}
} // namespace Omega

struct DP_Params
{
    float Is = 1.0e-9f; // saturation current
    float Vt = 25.8e-3f; // thermal voltage
    float nabla = 1.0f;
};

struct DP_Vars
{
    float vt_recip;
    float vt_2;
    float logR_Is_over_vt;
};

static inline void update_vars (DP_Vars& vars,
                                const DP_Params& params,
                                float child_R,
                                [[maybe_unused]] float child_G)
{
    const auto vt_adj = params.nabla * params.Vt;
    vars.vt_2 = 2.0f * vt_adj;
    vars.vt_recip = 1.0f / vt_adj;
    vars.logR_Is_over_vt = std::log (child_R * params.Is * vars.vt_recip);
}

static inline float root_compute (const DP_Vars& vars, float a)
{
    // See eqn (39) from reference paper:
    // https://www.researchgate.net/publication/299514713_An_Improved_and_Generalized_Diode_Clipper_Model_for_Wave_Digital_Filters
    const auto lamba = a >= 0.0f ? 1.0f : 0.0f;
    const auto lambda_a_over_vt = lambda * a * vars.vt_recip;
    const auto b = a - vars.vt_2 * lambda * (Omega::omega4 (vars.logR_Is_over_vt + lambda_a_over_vt)
                                           - Omega::omega4 (vars.logR_Is_over_vt - lambda_a_over_vt));
    return b;
}
}
