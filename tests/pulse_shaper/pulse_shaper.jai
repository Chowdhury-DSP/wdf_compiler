// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler pulse_shaper.wdf pulse_shaper.jai -lang jai

#load "wdf_lib_diode_omega3_test.jai";

Params :: struct {
    r162_value : float : 4.7e+03;
    c40_value : float : 1.5e-08;
    c40_alpha : float : 2.9e-02;
    r163_value : float : 1.0e+05;
    Vs_res_value : float : 1.0e-09;
    d53_params: Diode_Params;
}

Impedances :: struct {
    c40_b_coeff : float;
    c40_a_coeff : float;
    P1_pr : float;
    S1_pr : float;
    P2_pr : float;
    d53_vars: Diode_Vars;
}

State :: struct {
    c40_z : float;
    c40_b1 : float;
}

calc_impedances :: (impedances: *Impedances, fs: float, params: Params = .{}) {
    sum :: (a: $T, b: T) -> T { return a + b; }
    recip_sum :: (a: $T, b: T) -> T { return (a * b) / (a + b); }
    
    T := 1.0 / fs;
    
    // Computing impedance for: r162;
    r162_value := params.r162_value;
    r162_R := r162_value;
    r162_G := 1.0 / r162_R;
    // Computing impedance for: c40;
    impedances.c40_b_coeff = (1 - params.c40_alpha) * 0.5;
    impedances.c40_a_coeff = (1 + params.c40_alpha) * 0.5;
    c40_G := (1 + params.c40_alpha) * params.c40_value * fs;
    c40_R := 1.0 / c40_G;
    // Computing impedance for: r163;
    r163_value := params.r163_value;
    r163_R := r163_value;
    r163_G := 1.0 / r163_R;
    // Computing impedance for: Vs;
    Vs_res_value := params.Vs_res_value;
    Vs_R := Vs_res_value;
    Vs_G := 1.0 / Vs_R;
    // Computing impedance for: P1;
    P1_G := c40_G + r163_G;
    P1_R := 1.0 / P1_G;
    impedances.P1_pr = c40_G * P1_R;

    // Computing impedance for: S1;
    S1_R := P1_R + Vs_R;
    S1_G := 1.0 / S1_R;
    impedances.S1_pr = P1_R * S1_G;

    // Computing impedance for: I1;
    I1_R := r162_R;
    I1_G := 1.0 / I1_R;
    // Computing impedance for: P2;
    P2_G := I1_G + S1_G;
    P2_R := 1.0 / P2_G;
    impedances.P2_pr = I1_G * P2_R;

    // Computing impedance for: d53;
    update_vars(*impedances.d53_vars, params.d53_params, P2_R, P2_G);
}

process :: (state: *State, impedances: *Impedances, Vs: float) -> float {
    Vs_b := Vs; // Vs reflected
    r163_b := cast(float) 0; // r163 reflected
    c40_b := impedances.c40_b_coeff * state.c40_b1 + impedances.c40_a_coeff * state.c40_z; // c40 reflected
    state.c40_b1 = c40_b; // c40 state update
    P1_b := r163_b - impedances.P1_pr * (r163_b - c40_b); // P1 reflected
    S1_b := -(P1_b + Vs_b); // S1 reflected
    r162_b := cast(float) 0; // r162 reflected
    I1_b := -r162_b; // I1 reflected
    P2_b := S1_b - impedances.P2_pr * (S1_b - I1_b); // P2 reflected
    P2_a := root_compute(*impedances.d53_vars, P2_b); // P2 incident
    I1_a := P2_b - I1_b + P2_a; // I1 incident
    S1_a := P2_b - S1_b + P2_a; // S1 incident
    r162_a := -I1_a; // r162 incident
    P1_a := P1_b - impedances.S1_pr * (S1_a - S1_b); // P1 incident
    Vs_a := -S1_a - P1_b + impedances.S1_pr * (S1_a - S1_b); // Vs incident
    c40_a := P1_b - c40_b + P1_a; // c40 incident
    r163_a := P1_b - r163_b + P1_a; // r163 incident
    state.c40_z = c40_a; // c40 state update
    
    v_r162 := (r162_a + r162_b) * cast(float) 0.5; // r162 voltage
    return v_r162;
}

