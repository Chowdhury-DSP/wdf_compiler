/**
 * Implementation of an ideal wave-domain diode, based on the model from
 * "An Improved and Generalized Diode Clipper Model for Wave Digital Filters" by Werner et al.
 * The Wright-Omega function is implemented using an approximation derived by
 * D'Angelo et al (see wdf_lib_omega.h for details). This implementation uses the
 * "4th-order" approximation, but depending on the usage, you may prefer to use a lower-order approximation.
 *
 * Reference: https://www.researchgate.net/publication/299514713_An_Improved_and_Generalized_Diode_Clipper_Model_for_Wave_Digital_Filters
 */

Diode_Params :: struct {
    Is : float = 1.0e-9; // saturation current
    Vt : float = 25.85e-3; // thermal voltage
    nabla : float = 1.0;
}

Diode_Vars :: struct {
    vt_recip: float;
    vt_2: float;
    R_Is_2: float;
    R_Is_overVt: float;
    logR_Is_over_vt: float;
}

update_vars :: (vars: *Diode_Vars,
                params: Diode_Params,
                child_R: float,
                child_G: float) {
    vt_adj := params.nabla * params.Vt;
    vars.vt_2 = 2.0 * vt_adj;
    vars.vt_recip = 1.0 / vt_adj;
    vars.R_Is_2 = 2.0 * child_R * params.Is;
    vars.R_Is_overVt = child_R * params.Is * vars.vt_recip;
    vars.logR_Is_over_vt = Math.log (vars.R_Is_overVt);
}

root_compute :: (vars: *Diode_Vars, a: float) -> float {
    // See eqn (10) from reference paper
    a_over_vt := a * vars.vt_recip;
    omega := Omega.omega4 (vars.logR_Is_over_vt + a * vars.vt_recip + vars.R_Is_overVt);
    b := a + vars.R_Is_2 - vars.vt_2 * omega;
    return b;
}

Omega :: #import,file "wdf_lib_omega.jai";
Math :: #import "Math";
