// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler reductions_circuit2.wdf reductions_circuit2.jai -lang jai

Params :: struct {
    Vb_v_value : float : 1.5e+00;
    Rl1_value : float : 1.0e+03;
    L1_value : float : 2.0e-01;
    Rl2_value : float : 1.0e+04;
    Rp_value : float : 4.0e+03;
    Vcc_v_value : float : 1.2e+00;
    Vcc_cap_value : float;
    C1_value : float : 1.0e-08;
    Cp2_value : float : 7.0e-09;
    Vin_res_value : float : 5.0e+03;
    R1_value : float : 1.0e+04;
    R2_value : float;
    Cp_value : float : 4.0e-06;
    Rp2_value : float : 7.0e+03;
    Rl_v_value : float : -1.1e+00;
    Rl_res_value : float : 1.0e+05;
    Rl_cap_value : float : 1.0e-05;
}

Impedances :: struct {
    Vb_V : float : Params.Vb_v_value;
    Sr_Ri : float;
    S4_V : float : Params.Vcc_v_value;
    S8_T_over_2RC : float;
    Rl_V : float : Params.Rl_v_value;
    Rl_T_over_2RC : float;
    S7_pr : float;
    S6_pr : float;
    S3_pr : float;
    Sl_pr : float;
    Pr_pr : float;
}

State :: struct {
    Sr_z : float;
    S4_z : float;
    S4_v1 : float;
    Cp2_z : float;
    S8_z : float;
    Rl_z : float;
}

calc_impedances :: (impedances: *Impedances, fs: float, params: Params = .{}) {
    sum :: (a: $T, b: T) -> T { return a + b; }
    recip_sum :: (a: $T, b: T) -> T { return (a * b) / (a + b); }
    
    T := 1.0 / fs;
    
    // Computing impedance for: Sr;
    Sr_res_value := sum(params.Rl1_value, params.Rl2_value);
    Sr_ind_value := params.L1_value;
    Sr_R := 2.0 * Sr_ind_value * fs + Sr_res_value;
    Sr_G := 1.0 / Sr_R;
    impedances.Sr_Ri = Sr_res_value * Sr_G;

    // Computing impedance for: Rp;
    Rp_value := params.Rp_value;
    Rp_R := Rp_value;
    Rp_G := 1.0 / Rp_R;
    // Computing impedance for: S4;
    S4_cap_value := recip_sum(params.Vcc_cap_value, params.C1_value);
    S4_G := 2.0 * S4_cap_value * fs;
    S4_R := 1.0 / S4_G;
    // Computing impedance for: Cp2;
    Cp2_value := params.Cp2_value;
    Cp2_G := 2.0 * Cp2_value * fs;
    Cp2_R := 1.0 / Cp2_G;
    // Computing impedance for: S8;
    S8_res_value := sum(sum(sum(params.Vin_res_value, params.R1_value), params.R2_value), params.Rp2_value);
    S8_cap_value := params.Cp_value;
    S8_R := (T / (2.0 * S8_cap_value)) + S8_res_value;
    S8_G := 1.0 / S8_R;
    impedances.S8_T_over_2RC = T / (2.0 * S8_cap_value * S8_res_value);

    // Computing impedance for: Rl;
    Rl_res_value := params.Rl_res_value;
    Rl_cap_value := params.Rl_cap_value;
    Rl_R := (T / (2.0 * Rl_cap_value)) + Rl_res_value;
    Rl_G := 1.0 / Rl_R;
    impedances.Rl_T_over_2RC = T / (2.0 * Rl_cap_value * Rl_res_value);

    // Computing impedance for: S7;
    S7_R := Cp2_R + S8_R;
    S7_G := 1.0 / S7_R;
    impedances.S7_pr = Cp2_R * S7_G;

    // Computing impedance for: S6;
    S6_R := Rp_R + S4_R;
    S6_G := 1.0 / S6_R;
    impedances.S6_pr = Rp_R * S6_G;

    // Computing impedance for: S3;
    S3_R := S6_R + S7_R;
    S3_G := 1.0 / S3_R;
    impedances.S3_pr = S6_R * S3_G;

    // Computing impedance for: Sl;
    Sl_R := S3_R + Rl_R;
    Sl_G := 1.0 / Sl_R;
    impedances.Sl_pr = S3_R * Sl_G;

    // Computing impedance for: Pr;
    Pr_R := Sr_R + Sl_R;
    Pr_G := 1.0 / Pr_R;
    impedances.Pr_pr = Sr_R * Pr_G;

}

process :: (state: *State, impedances: *Impedances, S8: float) -> float {
    Rl_b := -(state.Rl_z + impedances.Rl_V); // Rl reflected
    S8_b := -(state.S8_z + -(S8)); // S8 reflected
    Cp2_b := state.Cp2_z; // Cp2 reflected
    S7_b := -(Cp2_b + S8_b); // S7 reflected
    S4_b := state.S4_z + -(impedances.S4_V) - state.S4_v1; // S4 reflected
    state.S4_v1 = -(impedances.S4_V); // S4 state update
    Rp_b := cast(float) 0; // Rp reflected
    S6_b := -(Rp_b + S4_b); // S6 reflected
    S3_b := -(S6_b + S7_b); // S3 reflected
    Sl_b := -(S3_b + Rl_b); // Sl reflected
    Sr_b := state.Sr_z; // Sr reflected
    Pr_b := -(Sr_b + Sl_b); // Pr reflected
    Vb_a := Pr_b; // Vb incident
    Vb_b := -Vb_a + 2.0 * impedances.Vb_V; // Vb reflected
    Pr_a := Vb_b; // Pr incident
    Sr_a := Sr_b - impedances.Pr_pr * (Pr_a - Pr_b); // Sr incident
    Sl_a := -Pr_a - Sr_b + impedances.Pr_pr * (Pr_a - Pr_b); // Sl incident
    state.Sr_z = -Sr_a + impedances.Sr_Ri * (Sr_a - state.Sr_z); // Sr state update
    S3_a := S3_b - impedances.Sl_pr * (Sl_a - Sl_b); // S3 incident
    Rl_a := -Sl_a - S3_b + impedances.Sl_pr * (Sl_a - Sl_b); // Rl incident
    S6_a := S6_b - impedances.S3_pr * (S3_a - S3_b); // S6 incident
    S7_a := -S3_a - S6_b + impedances.S3_pr * (S3_a - S3_b); // S7 incident
    Rp_a := Rp_b - impedances.S6_pr * (S6_a - S6_b); // Rp incident
    S4_a := -S6_a - Rp_b + impedances.S6_pr * (S6_a - S6_b); // S4 incident
    state.S4_z = S4_a; // S4 state update
    Cp2_a := Cp2_b - impedances.S7_pr * (S7_a - S7_b); // Cp2 incident
    S8_a := -S7_a - Cp2_b + impedances.S7_pr * (S7_a - S7_b); // S8 incident
    state.Cp2_z = Cp2_a; // Cp2 state update
    state.S8_z -= impedances.S8_T_over_2RC * (S8_a - S8_b); // S8 state update
    state.Rl_z -= impedances.Rl_T_over_2RC * (Rl_a - Rl_b); // Rl state update
    
    v_Rl := (Rl_a + Rl_b) * cast(float) 0.5; // Rl voltage
    return v_Rl;
}

