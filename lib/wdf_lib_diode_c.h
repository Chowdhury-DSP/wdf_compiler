#pragma once

#include <math.h>
#include "wdf_lib_omega_c.h"

/**
 * Plain-C port of wdf_lib_diode.h, for use by the wdf_compiler C backend.
 * Implementation of an ideal wave-domain diode, based on the model from
 * "An Improved and Generalized Diode Clipper Model for Wave Digital Filters" by Werner et al.
 *
 * Reference: https://www.researchgate.net/publication/299514713_An_Improved_and_Generalized_Diode_Clipper_Model_for_Wave_Digital_Filters
 */

struct wdf_lib_diode_Diode_Params
{
    float Is; // = 1.0e-9f; // saturation current
    float Vt; // = 25.85e-3f; // thermal voltage
    float nabla; // = 1.0f;
};

struct wdf_lib_diode_Diode_Vars
{
    float vt_recip;
    float vt_2;
    float R_Is_2;
    float R_Is_overVt;
    float logR_Is_over_vt;
};

static inline void wdf_lib_diode_update_vars (struct wdf_lib_diode_Diode_Vars* vars,
                                              const struct wdf_lib_diode_Diode_Params* params,
                                              float child_R,
                                              float child_G)
{
    const float vt_adj = params->nabla * params->Vt;
    vars->vt_2 = 2.0f * vt_adj;
    vars->vt_recip = 1.0f / vt_adj;
    vars->R_Is_2 = 2.0f * child_R * params->Is;
    vars->R_Is_overVt = child_R * params->Is * vars->vt_recip;
    vars->logR_Is_over_vt = logf (vars->R_Is_overVt);
}

static inline float wdf_lib_diode_root_compute (const struct wdf_lib_diode_Diode_Vars* vars, float a)
{
    // See eqn (10) from reference paper
    const float b = a + vars->R_Is_2 - vars->vt_2 * wdf_lib_omega_omega4 (vars->logR_Is_over_vt + a * vars->vt_recip + vars->R_Is_overVt);
    return b;
}
