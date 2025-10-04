#pragma once

#include <cmath>

namespace dangelo_triode
{
// Reference: https://dafx.de/paper-archive/2023/DAFx23_paper_15.pdf

struct Triode_Params
{
    float kp = 1.014e-5f;
    float kpg = 1.076e-5f;
    float kp2 = 5.498e-8f;
};

struct Triode_Vars
{
    float kp;
    float kpg;
    float kp2;
    float bk_bp;
    float bp_ap_0;
    float bp_ak_0;
    float k_bp_s;
    float k_eta;
    float k_delta;
};

static inline void update_vars (Triode_Vars& vars,
                                const Triode_Params& params,
                                float R0g,
                                float G0g,
                                float R0k,
                                float G0k,
                                float R0p,
                                float G0p)
{
    vars.kp = params.kp;
    vars.kpg = params.kpg;
    vars.kp2 = params.kp2;
    vars.bk_bp = R0k * G0p;
    const auto bp_k = 1.0f / (R0p + R0k);
    vars.bp_ap_0 = bp_k * (R0k - R0p);
    vars.bp_ak_0 = bp_k * (R0p + R0p);
    vars.k_eta = 1.0f / (vars.bk_bp * (0.5f * vars.kpg + vars.kp2) + vars.kp2);
    vars.k_bp_s = vars.k_eta * std::sqrt ((vars.kp2 + vars.kp2) * G0p);
    vars.k_delta = vars.kp2 * vars.k_eta * vars.k_eta / (R0p + R0p);
}

static inline void root_compute (const Triode_Vars& vars, const float* a, float* b)
{
    const auto ag = a[0];
    const auto ak = a[1];
    const auto ap = a[2];

    const auto v1 = 0.5f * ap;
    const auto v2 = ak + v1 * vars.bk_bp;
    const auto alpha = vars.kpg * (ag - v2) + vars.kp;
    const auto beta = vars.kp2 * (v1 - v2);
    const auto eta = vars.k_eta * (beta + beta + alpha);
    const auto v3 = eta + vars.k_delta;
    const auto delta = ap + v3;

    float bg, bk, bp, Vpk;
    if (delta >= 0.0f)
    {
        bp = vars.k_bp_s * std::sqrt (delta) - v3 - vars.k_delta;
        const auto d = vars.bk_bp * (ap - bp);
        bk = ak + d;
        const auto Vpk2 = ap + bp - ak - bk;

        if (vars.kpg * (ag - ak - 0.5f * d) + vars.kp2 * Vpk2 + vars.kp < 0.0f)
        {
            bp = ap;
            bk = ak;
            Vpk = ap - ak;
        }
        else
        {
            Vpk = 0.5f * Vpk2;
        }
    }
    else
    {
        bp = ap;
        bk = ak;
        Vpk = ap - ak;
    }

    if (Vpk < 0.0f)
        bp = vars.bp_ap_0 * ap + vars.bp_ak_0 * ak;

    bg = ag;

    b[0] = bg;
    b[1] = bk;
    b[2] = bp;
}
}
