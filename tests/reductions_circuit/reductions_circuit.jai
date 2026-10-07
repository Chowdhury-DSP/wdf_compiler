// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler reductions_circuit.wdf reductions_circuit.jai -lang jai

Params :: struct {
    Rv1_value : float : 1.0e+04;
    Rv2_value : float : 5.0e+03;
    Rl_value : float : 1.0e+05;
    R7_value : float : 1.0e+04;
    C7_value : float : 1.0e-05;
    R8_value : float : 1.0e+05;
    C8_value : float : 9.999999e-08;
    R1_value : float : 1.0e+03;
    R2_value : float;
    R3_value : float : 4.0e+03;
    R4_value : float;
    R5_value : float : 3.0e+03;
    C1_value : float : 1.0e-06;
    C2_value : float;
    C3_value : float : 4.0e-09;
    C4_value : float;
    C5_value : float : 3.0e-06;
    R6_value : float : 1.0e+04;
    C6_value : float : 4.0e-06;
}

Impedances :: struct {
    Vin_ca : float;
    Vin_ce : float;
    P7_2RC_over_2RC_plus_T : float;
    S7_2RC_over_T_plus_2RC : float;
    S8_pr : float;
    Sl_pr : float;
}

State :: struct {
    P7_z : float;
    S7_z : float;
}

calc_impedances :: (impedances: *Impedances, fs: float, params: Params = .{}) {
    sum :: (a: $T, b: T) -> T { return a + b; }
    recip_sum :: (a: $T, b: T) -> T { return (a * b) / (a + b); }
    
    T := 1.0 / fs;
    
    // Computing impedance for: Rl;
    Rl_value := params.Rl_value;
    Rl_R := Rl_value;
    Rl_G := 1.0 / Rl_R;
    // Computing impedance for: P7;
    P7_res_value := recip_sum(params.R7_value, params.R8_value);
    P7_cap_value := sum(params.C7_value, params.C8_value);
    P7_2RC := 2.0 * P7_cap_value * P7_res_value;
    P7_R := P7_res_value * T / (P7_2RC + T);
    P7_G := 1.0 / P7_R;
    impedances.P7_2RC_over_2RC_plus_T = P7_2RC / (P7_2RC + T);

    // Computing impedance for: S7;
    S7_res_value := sum(recip_sum(sum(sum(params.R1_value, params.R2_value), params.R3_value), recip_sum(params.R4_value, params.R5_value)), params.R6_value);
    S7_cap_value := recip_sum(sum(recip_sum(recip_sum(params.C1_value, params.C2_value), params.C3_value), sum(params.C4_value, params.C5_value)), params.C6_value);
    S7_R := (T / (2.0 * S7_cap_value)) + S7_res_value;
    S7_G := 1.0 / S7_R;
    impedances.S7_2RC_over_T_plus_2RC = (2.0 * S7_cap_value * S7_res_value) / (2.0 * S7_cap_value * S7_res_value + T);

    // Computing impedance for: S8;
    S8_R := P7_R + S7_R;
    S8_G := 1.0 / S8_R;
    impedances.S8_pr = P7_R * S8_G;

    // Computing impedance for: Sl;
    Sl_R := Rl_R + S8_R;
    Sl_G := 1.0 / Sl_R;
    impedances.Sl_pr = Rl_R * Sl_G;

    
    // Computing impedance for: Vin;
    Vin_res_value := sum(params.Rv1_value, params.Rv2_value);
    impedances.Vin_ca = (Vin_res_value - Sl_R) / (Vin_res_value + Sl_R);
    impedances.Vin_ce = (2.0 * Sl_R) / (Vin_res_value + Sl_R);

}

process :: (state: *State, impedances: *Impedances, Vin: float) -> float {
    S7_b := -state.S7_z; // S7 reflected
    P7_b := impedances.P7_2RC_over_2RC_plus_T * state.P7_z; // P7 reflected
    S8_b := -(P7_b + S7_b); // S8 reflected
    Rl_b := cast(float) 0; // Rl reflected
    Sl_b := -(Rl_b + S8_b); // Sl reflected
    Vin_a := Sl_b; // Vin incident
    Vin_b := impedances.Vin_ca * Vin_a + impedances.Vin_ce * Vin; // Vin reflected
    Sl_a := Vin_b; // Sl incident
    Rl_a := Rl_b - impedances.Sl_pr * (Sl_a - Sl_b); // Rl incident
    S8_a := -Sl_a - Rl_b + impedances.Sl_pr * (Sl_a - Sl_b); // S8 incident
    P7_a := P7_b - impedances.S8_pr * (S8_a - S8_b); // P7 incident
    S7_a := -S8_a - P7_b + impedances.S8_pr * (S8_a - S8_b); // S7 incident
    state.P7_z = P7_b + P7_a - state.P7_z; // P7 state update
    state.S7_z = -S7_a + impedances.S7_2RC_over_T_plus_2RC * (S7_a + state.S7_z); // S7 state update
    
    v_Rl := (Rl_a + Rl_b) * cast(float) 0.5; // Rl voltage
    return v_Rl;
}

