// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler baxandall_eq.wdf baxandall_eq.jai -lang jai

#load "custom_baxandall_rtype.jai";

Params :: struct {
    Ca_value : float : 1.0e-06;
    S4_res_value : float;
    S4_cap_value : float : 6.4e-09;
    Rl_value : float : 1.0e+06;
    S5_res_value : float;
    S5_cap_value : float : 6.4e-08;
    Resc_value : float : 1.0e+04;
    Resb_value : float : 1.0e+03;
    P3_res_value : float;
    P3_cap_value : float : 2.2e-07;
    Resa_value : float : 1.0e+04;
    P2_res_value : float;
    P2_cap_value : float : 2.2e-08;
    R_params: R_Params;
}

Impedances :: struct {
    Vin_ca : float;
    Vin_ce : float;
    S4_2RC_over_T_plus_2RC : float;
    S5_2RC_over_T_plus_2RC : float;
    P3_2RC_over_2RC_plus_T : float;
    P2_2RC_over_2RC_plus_T : float;
    S3_pr : float;
    S2_pr : float;
    P1_pr : float;
    R_vars: R_Vars;
}

State :: struct {
    Vin_z : float;
    S4_z : float;
    S5_z : float;
    P3_z : float;
    P2_z : float;
}

calc_impedances :: (impedances: *Impedances, fs: float, params: Params = .{}) {
    sum :: (a: $T, b: T) -> T { return a + b; }
    recip_sum :: (a: $T, b: T) -> T { return (a * b) / (a + b); }
    
    T := 1.0 / fs;
    
    // Computing impedance for: S4;
    S4_res_value := params.S4_res_value;
    S4_cap_value := params.S4_cap_value;
    S4_R := (T / (2.0 * S4_cap_value)) + S4_res_value;
    S4_G := 1.0 / S4_R;
    impedances.S4_2RC_over_T_plus_2RC = (2.0 * S4_cap_value * S4_res_value) / (2.0 * S4_cap_value * S4_res_value + T);

    // Computing impedance for: Rl;
    Rl_value := params.Rl_value;
    Rl_R := Rl_value;
    Rl_G := 1.0 / Rl_R;
    // Computing impedance for: S5;
    S5_res_value := params.S5_res_value;
    S5_cap_value := params.S5_cap_value;
    S5_R := (T / (2.0 * S5_cap_value)) + S5_res_value;
    S5_G := 1.0 / S5_R;
    impedances.S5_2RC_over_T_plus_2RC = (2.0 * S5_cap_value * S5_res_value) / (2.0 * S5_cap_value * S5_res_value + T);

    // Computing impedance for: Resc;
    Resc_value := params.Resc_value;
    Resc_R := Resc_value;
    Resc_G := 1.0 / Resc_R;
    // Computing impedance for: Resb;
    Resb_value := params.Resb_value;
    Resb_R := Resb_value;
    Resb_G := 1.0 / Resb_R;
    // Computing impedance for: P3;
    P3_res_value := params.P3_res_value;
    P3_cap_value := params.P3_cap_value;
    P3_2RC := 2.0 * P3_cap_value * P3_res_value;
    P3_R := P3_res_value * T / (P3_2RC + T);
    P3_G := 1.0 / P3_R;
    impedances.P3_2RC_over_2RC_plus_T = P3_2RC / (P3_2RC + T);

    // Computing impedance for: Resa;
    Resa_value := params.Resa_value;
    Resa_R := Resa_value;
    Resa_G := 1.0 / Resa_R;
    // Computing impedance for: P2;
    P2_res_value := params.P2_res_value;
    P2_cap_value := params.P2_cap_value;
    P2_2RC := 2.0 * P2_cap_value * P2_res_value;
    P2_R := P2_res_value * T / (P2_2RC + T);
    P2_G := 1.0 / P2_R;
    impedances.P2_2RC_over_2RC_plus_T = P2_2RC / (P2_2RC + T);

    // Computing impedance for: S3;
    S3_R := Resb_R + P3_R;
    S3_G := 1.0 / S3_R;
    impedances.S3_pr = Resb_R * S3_G;

    // Computing impedance for: S2;
    S2_R := Resa_R + P2_R;
    S2_G := 1.0 / S2_R;
    impedances.S2_pr = Resa_R * S2_G;

    // Computing impedance for: P1;
    P1_G := Rl_G + S5_G;
    P1_R := 1.0 / P1_G;
    impedances.P1_pr = Rl_G * P1_R;

    // Computing impedance for: R;
    R_R_calc := update_vars(*impedances.R_vars, params.R_params, S4_R, S4_G, P1_R, P1_G, Resc_R, Resc_G, S3_R, S3_G, S2_R, S2_G);
    R_R := R_R_calc;
    R_G := 1.0 / R_R;
    
    // Computing impedance for: Vin;
    Vin_cap_value := params.Ca_value;
    impedances.Vin_ca = T / (T + 2.0 * R_R * Vin_cap_value);
    impedances.Vin_ce = (2.0 * R_R * Vin_cap_value) / (T + 2.0 * R_R * Vin_cap_value);

}

process :: (state: *State, impedances: *Impedances, Vin: float) -> float {
    P2_b := impedances.P2_2RC_over_2RC_plus_T * state.P2_z; // P2 reflected
    Resa_b := cast(float) 0; // Resa reflected
    S2_b := -(Resa_b + P2_b); // S2 reflected
    P3_b := impedances.P3_2RC_over_2RC_plus_T * state.P3_z; // P3 reflected
    Resb_b := cast(float) 0; // Resb reflected
    S3_b := -(Resb_b + P3_b); // S3 reflected
    Resc_b := cast(float) 0; // Resc reflected
    S5_b := -state.S5_z; // S5 reflected
    Rl_b := cast(float) 0; // Rl reflected
    P1_b := S5_b - impedances.P1_pr * (S5_b - Rl_b); // P1 reflected
    S4_b := -state.S4_z; // S4 reflected
    R_ins := float.[S4_b, P1_b, Resc_b, S3_b, S2_b] #align 16; // R inputs
    R_b := reflected(*impedances.R_vars, R_ins.data); // R reflected
    Vin_a := R_b; // Vin incident
    Vin_a0a1 := 2.0 * (Vin + state.Vin_z) + Vin_a; // Vin reflected
    Vin_b := impedances.Vin_ca * Vin_a - impedances.Vin_ce * Vin_a0a1; // Vin reflected
    state.Vin_z += -Vin_b - Vin_a0a1; // Vin_z state update
    R_a := Vin_b; // R incident
    R_outs: [5] float = --- #align 16; // 5 outputs 
    incident(*impedances.R_vars, R_a, R_ins.data, R_outs.data); // R compute
    S4_a := R_outs[0];
    P1_a := R_outs[1];
    Resc_a := R_outs[2];
    S3_a := R_outs[3];
    S2_a := R_outs[4];
    state.S4_z = -S4_a + impedances.S4_2RC_over_T_plus_2RC * (S4_a + state.S4_z); // S4 state update
    Rl_a := P1_b - Rl_b + P1_a; // Rl incident
    S5_a := P1_b - S5_b + P1_a; // S5 incident
    state.S5_z = -S5_a + impedances.S5_2RC_over_T_plus_2RC * (S5_a + state.S5_z); // S5 state update
    Resb_a := Resb_b - impedances.S3_pr * (S3_a - S3_b); // Resb incident
    P3_a := -S3_a - Resb_b + impedances.S3_pr * (S3_a - S3_b); // P3 incident
    state.P3_z = P3_b + P3_a - state.P3_z; // P3 state update
    Resa_a := Resa_b - impedances.S2_pr * (S2_a - S2_b); // Resa incident
    P2_a := -S2_a - Resa_b + impedances.S2_pr * (S2_a - S2_b); // P2 incident
    state.P2_z = P2_b + P2_a - state.P2_z; // P2 state update
    
    v_Rl := (Rl_a + Rl_b) * cast(float) 0.5; // Rl voltage
    return v_Rl;
}

