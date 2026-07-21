#pragma once

#include <math.h>
#include "wdf_lib_omega_c.h"

/**
 * Plain-C port of wdf_lib_diode_pair.h, for use by the wdf_compiler C backend.
 * Implementation of an ideal wave-domain diode pair, based on the model from
 * "An Improved and Generalized Diode Clipper Model for Wave Digital Filters" by Werner et al.
 *
 * Reference: https://www.researchgate.net/publication/299514713_An_Improved_and_Generalized_Diode_Clipper_Model_for_Wave_Digital_Filters
 */

struct wdf_lib_diode_pair_Diode_Pair_Params
{
    float Is; // = 1.0e-9f; // saturation current
    float Vt; // = 25.85e-3f; // thermal voltage
    float nabla; // = 1.0f;
};

struct wdf_lib_diode_pair_Diode_Pair_Vars
{
    float vt_recip;
    float vt_2;
    float logR_Is_over_vt;
};

static inline void wdf_lib_diode_pair_update_vars (struct wdf_lib_diode_pair_Diode_Pair_Vars* vars,
                                                    const struct wdf_lib_diode_pair_Diode_Pair_Params* params,
                                                    float child_R,
                                                    float child_G)
{
    const float vt_adj = params->nabla * params->Vt;
    vars->vt_2 = 2.0f * vt_adj;
    vars->vt_recip = 1.0f / vt_adj;
    vars->logR_Is_over_vt = logf (child_R * params->Is * vars->vt_recip);
}

static inline float wdf_lib_diode_pair_root_compute (const struct wdf_lib_diode_pair_Diode_Pair_Vars* vars, float a)
{
    // See eqn (39) from reference paper
    const float lambda = a > 0.0f ? 1.0f : (a < 0.0f ? -1.0f : 0.0f);
    const float lambda_a_over_vt = lambda * a * vars->vt_recip;
    const float b = a - vars->vt_2 * lambda * (wdf_lib_omega_omega4 (vars->logR_Is_over_vt + lambda_a_over_vt)
                                              - wdf_lib_omega_omega4 (vars->logR_Is_over_vt - lambda_a_over_vt));
    return b;
}
