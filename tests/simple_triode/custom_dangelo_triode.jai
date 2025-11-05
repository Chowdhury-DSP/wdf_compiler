
// Reference: https://dafx.de/paper-archive/2023/DAFx23_paper_15.pdf

Triode_Params :: struct {
    kp : float32 = 1.014e-5;
    kpg : float32 = 1.076e-5;
    kp2 : float32 = 5.498e-8;
}

Triode_Vars :: struct {
    kp: float32;
    kpg: float32;
    kp2: float32;
    bk_bp: float32;
    bp_ap_0: float32;
    bp_ak_0: float32;
    k_bp_s: float32;
    k_eta: float32;
    k_delta: float32;
}

update_vars ::  (vars: *Triode_Vars,
                 params: Triode_Params,
                 R0g: float32,
                 G0g: float32,
                 R0k: float32,
                 G0k: float32,
                 R0p: float32,
                 G0p: float32)
{
    vars.kp = params.kp;
    vars.kpg = params.kpg;
    vars.kp2 = params.kp2;
    vars.bk_bp = R0k * G0p;
    bp_k := 1.0 / (R0p + R0k);
    vars.bp_ap_0 = bp_k * (R0k - R0p);
    vars.bp_ak_0 = bp_k * (R0p + R0p);
    vars.k_eta = 1.0 / (vars.bk_bp * (0.5 * vars.kpg + vars.kp2) + vars.kp2);
    vars.k_bp_s = vars.k_eta * sqrt ((vars.kp2 + vars.kp2) * G0p);
    vars.k_delta = vars.kp2 * vars.k_eta * vars.k_eta / (R0p + R0p);
}

root_compute :: (vars: *Triode_Vars, a: *float32, b: *float32)
{
    ag := a[0];
    ak := a[1];
    ap := a[2];

    v1 := 0.5 * ap;
    v2 := ak + v1 * vars.bk_bp;
    alpha := vars.kpg * (ag - v2) + vars.kp;
    beta := vars.kp2 * (v1 - v2);
    eta := vars.k_eta * (beta + beta + alpha);
    v3 := eta + vars.k_delta;
    delta := ap + v3;

    bg: float32;
    bk: float32;
    bp: float32;
    Vpk: float32;
    if (delta >= 0.0)
    {
        bp = vars.k_bp_s * sqrt (delta) - v3 - vars.k_delta;
        d := vars.bk_bp * (ap - bp);
        bk = ak + d;
        Vpk2 := ap + bp - ak - bk;

        if (vars.kpg * (ag - ak - 0.5 * d) + vars.kp2 * Vpk2 + vars.kp < 0.0)
        {
            bp = ap;
            bk = ak;
            Vpk = ap - ak;
        }
        else
        {
            Vpk = 0.5 * Vpk2;
        }
    }
    else
    {
        bp = ap;
        bk = ak;
        Vpk = ap - ak;
    }

    if (Vpk < 0.0)
        bp = vars.bp_ap_0 * ap + vars.bp_ak_0 * ak;

    bg = ag;

    b[0] = bg;
    b[1] = bk;
    b[2] = bp;
}
