// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler analog_eq.wdf analog_eq.jai -lang jai

#load "custom_analog_eq_rtype.jai";
#load "custom_nonlinear_inductor.jai";

Params :: struct {
    Ra_value : float : 6.2e+02;
    P_treble_boost_plus_value : float;
    P_treble_boost_minus_value : float;
    R_treble_boost_bw_value : float;
    C_treble_boost_value : float;
    L_treble_boost_params: L_Params;
    Rlc_value : float : 1.0e+03;
    P_low_cut_value : float;
    C_low_cut_value : float;
    P_treble_cut_plus_value : float;
    P_treble_cut_minus_value : float;
    Rc_value : float : 7.5e+01;
    C_treble_cut_value : float;
    P_low_boost_value : float;
    C_low_boost_value : float;
    Rb_value : float : 1.0e+04;
    Rload_value : float : 1.0e+06;
    R_params: R_Params;
}

Impedances :: struct {
    Pb_2RC_over_2RC_plus_T : float;
    Streble_cut_2RC_over_T_plus_2RC : float;
    Pbass_boost_2RC_over_2RC_plus_T : float;
    Streble_boost_1_pr : float;
    Streble_boost_pr : float;
    Sb_pr : float;
    Ptreble_boost_pr : float;
    Ptreble_cut_pr : float;
    R_vars: R_Vars;
    Svin_pr : float;
    SR_pr : float;
    Pvin_pr : float;
}

State :: struct {
    C_treble_boost_z : float;
    L_treble_boost_state: L_State;
    Pb_z : float;
    Streble_cut_z : float;
    Pbass_boost_z : float;
}

calc_impedances :: (impedances: *Impedances, fs: float, params: Params = .{}) {
    sum :: (a: $T, b: T) -> T { return a + b; }
    recip_sum :: (a: $T, b: T) -> T { return (a * b) / (a + b); }
    
    T := 1.0 / fs;
    
    // Computing impedance for: Ra;
    Ra_value := params.Ra_value;
    Ra_R := Ra_value;
    Ra_G := 1.0 / Ra_R;
    // Computing impedance for: P_treble_boost_plus;
    P_treble_boost_plus_value := params.P_treble_boost_plus_value;
    P_treble_boost_plus_R := P_treble_boost_plus_value;
    P_treble_boost_plus_G := 1.0 / P_treble_boost_plus_R;
    // Computing impedance for: P_treble_boost_minus;
    P_treble_boost_minus_value := params.P_treble_boost_minus_value;
    P_treble_boost_minus_R := P_treble_boost_minus_value;
    P_treble_boost_minus_G := 1.0 / P_treble_boost_minus_R;
    // Computing impedance for: R_treble_boost_bw;
    R_treble_boost_bw_value := params.R_treble_boost_bw_value;
    R_treble_boost_bw_R := R_treble_boost_bw_value;
    R_treble_boost_bw_G := 1.0 / R_treble_boost_bw_R;
    // Computing impedance for: C_treble_boost;
    C_treble_boost_value := params.C_treble_boost_value;
    C_treble_boost_G := 2.0 * C_treble_boost_value * fs;
    C_treble_boost_R := 1.0 / C_treble_boost_G;
    // Computing impedance for: L_treble_boost;
    L_treble_boost_R_calc := update_vars(params.L_treble_boost_params, fs);
    L_treble_boost_R := L_treble_boost_R_calc;
    L_treble_boost_G := 1.0 / L_treble_boost_R;
    // Computing impedance for: Rlc;
    Rlc_value := params.Rlc_value;
    Rlc_R := Rlc_value;
    Rlc_G := 1.0 / Rlc_R;
    // Computing impedance for: Pb;
    Pb_res_value := params.P_low_cut_value;
    Pb_cap_value := params.C_low_cut_value;
    Pb_2RC := 2.0 * Pb_cap_value * Pb_res_value;
    Pb_R := Pb_res_value * T / (Pb_2RC + T);
    Pb_G := 1.0 / Pb_R;
    impedances.Pb_2RC_over_2RC_plus_T = Pb_2RC / (Pb_2RC + T);

    // Computing impedance for: P_treble_cut_plus;
    P_treble_cut_plus_value := params.P_treble_cut_plus_value;
    P_treble_cut_plus_R := P_treble_cut_plus_value;
    P_treble_cut_plus_G := 1.0 / P_treble_cut_plus_R;
    // Computing impedance for: P_treble_cut_minus;
    P_treble_cut_minus_value := params.P_treble_cut_minus_value;
    P_treble_cut_minus_R := P_treble_cut_minus_value;
    P_treble_cut_minus_G := 1.0 / P_treble_cut_minus_R;
    // Computing impedance for: Streble_cut;
    Streble_cut_res_value := params.Rc_value;
    Streble_cut_cap_value := params.C_treble_cut_value;
    Streble_cut_R := (T / (2.0 * Streble_cut_cap_value)) + Streble_cut_res_value;
    Streble_cut_G := 1.0 / Streble_cut_R;
    impedances.Streble_cut_2RC_over_T_plus_2RC = (2.0 * Streble_cut_cap_value * Streble_cut_res_value) / (2.0 * Streble_cut_cap_value * Streble_cut_res_value + T);

    // Computing impedance for: Pbass_boost;
    Pbass_boost_res_value := params.P_low_boost_value;
    Pbass_boost_cap_value := params.C_low_boost_value;
    Pbass_boost_2RC := 2.0 * Pbass_boost_cap_value * Pbass_boost_res_value;
    Pbass_boost_R := Pbass_boost_res_value * T / (Pbass_boost_2RC + T);
    Pbass_boost_G := 1.0 / Pbass_boost_R;
    impedances.Pbass_boost_2RC_over_2RC_plus_T = Pbass_boost_2RC / (Pbass_boost_2RC + T);

    // Computing impedance for: Rb;
    Rb_value := params.Rb_value;
    Rb_R := Rb_value;
    Rb_G := 1.0 / Rb_R;
    // Computing impedance for: Rload;
    Rload_value := params.Rload_value;
    Rload_R := Rload_value;
    Rload_G := 1.0 / Rload_R;
    // Computing impedance for: Streble_boost_1;
    Streble_boost_1_R := C_treble_boost_R + L_treble_boost_R;
    Streble_boost_1_G := 1.0 / Streble_boost_1_R;
    impedances.Streble_boost_1_pr = C_treble_boost_R * Streble_boost_1_G;

    // Computing impedance for: Streble_boost;
    Streble_boost_R := R_treble_boost_bw_R + Streble_boost_1_R;
    Streble_boost_G := 1.0 / Streble_boost_R;
    impedances.Streble_boost_pr = R_treble_boost_bw_R * Streble_boost_G;

    // Computing impedance for: Sb;
    Sb_R := Rlc_R + Pb_R;
    Sb_G := 1.0 / Sb_R;
    impedances.Sb_pr = Rlc_R * Sb_G;

    // Computing impedance for: Ptreble_boost;
    Ptreble_boost_G := P_treble_boost_minus_G + Streble_boost_G;
    Ptreble_boost_R := 1.0 / Ptreble_boost_G;
    impedances.Ptreble_boost_pr = P_treble_boost_minus_G * Ptreble_boost_R;

    // Computing impedance for: Ptreble_cut;
    Ptreble_cut_G := P_treble_cut_minus_G + Streble_cut_G;
    Ptreble_cut_R := 1.0 / Ptreble_cut_G;
    impedances.Ptreble_cut_pr = P_treble_cut_minus_G * Ptreble_cut_R;

    // Computing impedance for: R;
    R_R_calc := update_vars(*impedances.R_vars, params.R_params, Sb_R, Sb_G, P_treble_cut_plus_R, P_treble_cut_plus_G, Ptreble_cut_R, Ptreble_cut_G, Pbass_boost_R, Pbass_boost_G, Rb_R, Rb_G, Rload_R, Rload_G);
    R_R := R_R_calc;
    R_G := 1.0 / R_R;
    // Computing impedance for: Svin;
    Svin_R := P_treble_boost_plus_R + Ptreble_boost_R;
    Svin_G := 1.0 / Svin_R;
    impedances.Svin_pr = P_treble_boost_plus_R * Svin_G;

    // Computing impedance for: SR;
    SR_R := Svin_R + R_R;
    SR_G := 1.0 / SR_R;
    impedances.SR_pr = Svin_R * SR_G;

    // Computing impedance for: Pvin;
    Pvin_G := Ra_G + SR_G;
    Pvin_R := 1.0 / Pvin_G;
    impedances.Pvin_pr = Ra_G * Pvin_R;

}

process :: (state: *State, impedances: *Impedances, Vin: float) -> float {
    Rload_b := cast(float) 0; // Rload reflected
    Rb_b := cast(float) 0; // Rb reflected
    Pbass_boost_b := impedances.Pbass_boost_2RC_over_2RC_plus_T * state.Pbass_boost_z; // Pbass_boost reflected
    Streble_cut_b := -state.Streble_cut_z; // Streble_cut reflected
    P_treble_cut_minus_b := cast(float) 0; // P_treble_cut_minus reflected
    Ptreble_cut_b := Streble_cut_b - impedances.Ptreble_cut_pr * (Streble_cut_b - P_treble_cut_minus_b); // Ptreble_cut reflected
    P_treble_cut_plus_b := cast(float) 0; // P_treble_cut_plus reflected
    Pb_b := impedances.Pb_2RC_over_2RC_plus_T * state.Pb_z; // Pb reflected
    Rlc_b := cast(float) 0; // Rlc reflected
    Sb_b := -(Rlc_b + Pb_b); // Sb reflected
    R_ins := float.[Sb_b, P_treble_cut_plus_b, Ptreble_cut_b, Pbass_boost_b, Rb_b, Rload_b] #align 16; // R inputs
    R_b := reflected(*impedances.R_vars, R_ins.data); // R reflected
    L_treble_boost_b :=  reflected(*state.L_treble_boost_state); // L_treble_boost reflected
    C_treble_boost_b := state.C_treble_boost_z; // C_treble_boost reflected
    Streble_boost_1_b := -(C_treble_boost_b + L_treble_boost_b); // Streble_boost_1 reflected
    R_treble_boost_bw_b := cast(float) 0; // R_treble_boost_bw reflected
    Streble_boost_b := -(R_treble_boost_bw_b + Streble_boost_1_b); // Streble_boost reflected
    P_treble_boost_minus_b := cast(float) 0; // P_treble_boost_minus reflected
    Ptreble_boost_b := Streble_boost_b - impedances.Ptreble_boost_pr * (Streble_boost_b - P_treble_boost_minus_b); // Ptreble_boost reflected
    P_treble_boost_plus_b := cast(float) 0; // P_treble_boost_plus reflected
    Svin_b := -(P_treble_boost_plus_b + Ptreble_boost_b); // Svin reflected
    SR_b := -(Svin_b + R_b); // SR reflected
    Ra_b := cast(float) 0; // Ra reflected
    Pvin_b := SR_b - impedances.Pvin_pr * (SR_b - Ra_b); // Pvin reflected
    Vin_a := Pvin_b; // Vin incident
    Vin_b := -Vin_a + 2.0 * Vin; // Vin reflected
    Pvin_a := Vin_b; // Pvin incident
    Ra_a := Pvin_b - Ra_b + Pvin_a; // Ra incident
    SR_a := Pvin_b - SR_b + Pvin_a; // SR incident
    Svin_a := Svin_b - impedances.SR_pr * (SR_a - SR_b); // Svin incident
    R_a := -SR_a - Svin_b + impedances.SR_pr * (SR_a - SR_b); // R incident
    P_treble_boost_plus_a := P_treble_boost_plus_b - impedances.Svin_pr * (Svin_a - Svin_b); // P_treble_boost_plus incident
    Ptreble_boost_a := -Svin_a - P_treble_boost_plus_b + impedances.Svin_pr * (Svin_a - Svin_b); // Ptreble_boost incident
    P_treble_boost_minus_a := Ptreble_boost_b - P_treble_boost_minus_b + Ptreble_boost_a; // P_treble_boost_minus incident
    Streble_boost_a := Ptreble_boost_b - Streble_boost_b + Ptreble_boost_a; // Streble_boost incident
    R_treble_boost_bw_a := R_treble_boost_bw_b - impedances.Streble_boost_pr * (Streble_boost_a - Streble_boost_b); // R_treble_boost_bw incident
    Streble_boost_1_a := -Streble_boost_a - R_treble_boost_bw_b + impedances.Streble_boost_pr * (Streble_boost_a - Streble_boost_b); // Streble_boost_1 incident
    C_treble_boost_a := C_treble_boost_b - impedances.Streble_boost_1_pr * (Streble_boost_1_a - Streble_boost_1_b); // C_treble_boost incident
    L_treble_boost_a := -Streble_boost_1_a - C_treble_boost_b + impedances.Streble_boost_1_pr * (Streble_boost_1_a - Streble_boost_1_b); // L_treble_boost incident
    state.C_treble_boost_z = C_treble_boost_a; // C_treble_boost state update
    incident(*state.L_treble_boost_state, L_treble_boost_a); // L_treble_boost incident
    R_outs: [6] float = --- #align 16; // 6 outputs 
    incident(*impedances.R_vars, R_a, R_ins.data, R_outs.data); // R compute
    Sb_a := R_outs[0];
    P_treble_cut_plus_a := R_outs[1];
    Ptreble_cut_a := R_outs[2];
    Pbass_boost_a := R_outs[3];
    Rb_a := R_outs[4];
    Rload_a := R_outs[5];
    Rlc_a := Rlc_b - impedances.Sb_pr * (Sb_a - Sb_b); // Rlc incident
    Pb_a := -Sb_a - Rlc_b + impedances.Sb_pr * (Sb_a - Sb_b); // Pb incident
    state.Pb_z = Pb_b + Pb_a - state.Pb_z; // Pb state update
    P_treble_cut_minus_a := Ptreble_cut_b - P_treble_cut_minus_b + Ptreble_cut_a; // P_treble_cut_minus incident
    Streble_cut_a := Ptreble_cut_b - Streble_cut_b + Ptreble_cut_a; // Streble_cut incident
    state.Streble_cut_z = -Streble_cut_a + impedances.Streble_cut_2RC_over_T_plus_2RC * (Streble_cut_a + state.Streble_cut_z); // Streble_cut state update
    state.Pbass_boost_z = Pbass_boost_b + Pbass_boost_a - state.Pbass_boost_z; // Pbass_boost state update
    
    v_Rload := (Rload_a + Rload_b) * cast(float) 0.5; // Rload voltage
    return v_Rload;
}

