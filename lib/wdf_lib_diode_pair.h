#pragma once

#include <cmath>
#include <wdf_lib_omega.h>

/**
 * Implementation of an ideal wave-domain diode pair, based on the model from
 * "An Improved and Generalized Diode Clipper Model for Wave Digital Filters" by Werner et al.
 * The Wright-Omega function is implemented using an approximation derived by
 * D'Angelo et al (see wdf_lib_omega.h for details). This implementation uses the
 * "4th-order" approximation, but depending on the usage, you may prefer to use a lower-order approximation.
 *
 * Reference: https://www.researchgate.net/publication/299514713_An_Improved_and_Generalized_Diode_Clipper_Model_for_Wave_Digital_Filters
 */
namespace wdf_lib::diode_pair
{
struct Diode_Pair_Params
{
    float Is = 1.0e-9f; // saturation current
    float Vt = 25.85e-3f; // thermal voltage
    float nabla = 1.0f;
};

struct Diode_Pair_Vars
{
    float vt_recip;
    float vt_2;
    float logR_Is_over_vt;
};

static inline void update_vars (Diode_Pair_Vars& vars,
                                const Diode_Pair_Params& params,
                                float child_R,
                                [[maybe_unused]] float child_G)
{
    const auto vt_adj = params.nabla * params.Vt;
    vars.vt_2 = 2.0f * vt_adj;
    vars.vt_recip = 1.0f / vt_adj;
    vars.logR_Is_over_vt = std::log (child_R * params.Is * vars.vt_recip);
}

static inline float root_compute (const Diode_Pair_Vars& vars, float a)
{
    // See eqn (39) from reference paper
    const auto lambda = a >= 0.0f ? 1.0f : 0.0f;
    const auto lambda_a_over_vt = lambda * a * vars.vt_recip;
    const auto b = a - vars.vt_2 * lambda * (wdf_lib::Omega::omega4 (vars.logR_Is_over_vt + lambda_a_over_vt)
                                           - wdf_lib::Omega::omega4 (vars.logR_Is_over_vt - lambda_a_over_vt));
    return b;
}
}
