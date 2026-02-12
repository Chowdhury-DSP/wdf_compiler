#pragma once

#include <cmath>
#include <wdf_lib_omega.h>

/**
 * Implementation of an ideal wave-domain diode, based on the model from
 * "An Improved and Generalized Diode Clipper Model for Wave Digital Filters" by Werner et al.
 * The Wright-Omega function is implemented using an approximation derived by
 * D'Angelo et al (see wdf_lib_omega.h for details). This implementation uses the
 * "4th-order" approximation, but depending on the usage, you may prefer to use a lower-order approximation.
 *
 * Reference: https://www.researchgate.net/publication/299514713_An_Improved_and_Generalized_Diode_Clipper_Model_for_Wave_Digital_Filters
 */
namespace wdf_lib::diode
{
struct Diode_Params
{
    float Is; // = 1.0e-9f; // saturation current
    float Vt; // = 25.85e-3f; // thermal voltage
    float nabla; // = 1.0f;
};

struct Diode_Vars
{
    float vt_recip;
    float vt_2;
    float R_Is_2;
    float R_Is_overVt;
    float logR_Is_over_vt;
};

static inline void update_vars (Diode_Vars* vars,
                                const Diode_Params* params,
                                float child_R,
                                float /*child_G*/)
{
    const auto vt_adj = params->nabla * params->Vt;
    vars->vt_2 = 2.0f * vt_adj;
    vars->vt_recip = 1.0f / vt_adj;
    vars->R_Is_2 = 2.0f * child_R * params->Is;
    vars->R_Is_overVt = child_R * params->Is * vars->vt_recip;
    vars->logR_Is_over_vt = std::log (vars->R_Is_overVt);
}

static inline float root_compute (const Diode_Vars* vars, float a)
{
    // See eqn (10) from reference paper
    const auto a_over_vt = a * vars->vt_recip;
    const auto b = a + vars->R_Is_2 - vars->vt_2 * wdf_lib::Omega::omega4 (vars->logR_Is_over_vt + a * vars->vt_recip + vars->R_Is_overVt);
    return b;
}
}
