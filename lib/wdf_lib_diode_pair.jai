/**
 * Implementation of an ideal wave-domain diode pair, based on the model from
 * "An Improved and Generalized Diode Clipper Model for Wave Digital Filters" by Werner et al.
 * The Wright-Omega function is implemented using an approximation derived by
 * D'Angelo et al (see wdf_lib_omega.h for details). This implementation uses the
 * "4th-order" approximation, but depending on the usage, you may prefer to use a lower-order approximation.
 *
 * Reference: https://www.researchgate.net/publication/299514713_An_Improved_and_Generalized_Diode_Clipper_Model_for_Wave_Digital_Filters
 */

Diode_Pair_Params :: struct {
    Is : float = 1.0e-9; // saturation current
    Vt : float = 25.85e-3; // thermal voltage
    nabla : float = 1.0;
}

Diode_Pair_Vars :: struct {
    vt_recip: float;
    vt_2: float;
    logR_Is_over_vt: float;
}

update_vars :: (vars: *Diode_Pair_Vars,
                params: Diode_Pair_Params,
                child_R: float,
                child_G: float) {
    vt_adj := params.nabla * params.Vt;
    vars.vt_2 = 2.0 * vt_adj;
    vars.vt_recip = 1.0 / vt_adj;
    vars.logR_Is_over_vt = Math.log (child_R * params.Is * vars.vt_recip);
}

root_compute :: (vars: *Diode_Pair_Vars, a: float) -> float {
    // See eqn (39) from reference paper
    lambda : float = ifx a >= 0 then 1.0 else 0.0;
    lambda_a_over_vt := lambda * a * vars.vt_recip;
    b := a - vars.vt_2 * lambda * (Omega.omega4 (vars.logR_Is_over_vt + lambda_a_over_vt)
                                 - Omega.omega4 (vars.logR_Is_over_vt - lambda_a_over_vt));
    return b;
}

Omega :: #import,file "wdf_lib_omega.jai";
Math :: #import "Math";
