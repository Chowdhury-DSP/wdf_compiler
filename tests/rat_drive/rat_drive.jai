// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler rat_drive.net rat_drive.jai -lang jai -netlist

#load "../../lib/wdf_lib_diode_pair.jai";
#load "R26_rtype.jai";

Params :: struct {
    R6_value : float : 1.0e+03;
    C7_value : float : 4.7e-06;
    R3_value : float : 1.0e+03;
    Vplus_v_value : float;
    Vplus_res_value : float : 1.0e+06;
    Vin_cap_value : float : 2.2e-08;
    C2_value : float : 1.0e-09;
    R4_value : float : 4.7e+01;
    C5_value : float : 2.2e-06;
    R5_value : float : 5.6e+02;
    C6_value : float : 4.7e-06;
    Rd_value : float;
    C4_value : float : 1.0e-10;
    R26_params: rtype_R26_R_Params;
    XP_params: Diode_Pair_Params;
}

Impedances :: struct {
    S24_2RC_over_T_plus_2RC : float;
    Vplus_V : float;
    S21_2RC_over_T_plus_2RC : float;
    S22_2RC_over_T_plus_2RC : float;
    P15_2RC_over_2RC_plus_T : float;
    P18_pr : float;
    S19_pr : float;
    P20_pr : float;
    P23_pr : float;
    R26_vars: rtype_R26_R_Vars;
    S25_pr : float;
    XP_vars: Diode_Pair_Vars;
}

State :: struct {
    S24_z : float;
    Vin_z : float;
    Vin_v1 : float;
    C2_z : float;
    S21_z : float;
    S22_z : float;
    P15_z : float;
}

calc_impedances :: (impedances: *Impedances, fs: float, params: Params = .{}) {
    sum :: (a: $T, b: T) -> T { return a + b; }
    recip_sum :: (a: $T, b: T) -> T { return (a * b) / (a + b); }
    
    T := 1.0 / fs;
    
    // Computing impedance for: S24;
    S24_res_value := params.R6_value;
    S24_cap_value := params.C7_value;
    S24_R := (T / (2.0 * S24_cap_value)) + S24_res_value;
    S24_G := 1.0 / S24_R;
    impedances.S24_2RC_over_T_plus_2RC = (2.0 * S24_cap_value * S24_res_value) / (2.0 * S24_cap_value * S24_res_value + T);

    // Computing impedance for: R3;
    R3_value := params.R3_value;
    R3_R := R3_value;
    R3_G := 1.0 / R3_R;
    // Computing voltage for: Vplus;
    impedances.Vplus_V = params.Vplus_v_value;

    // Computing impedance for: Vplus;
    Vplus_res_value := params.Vplus_res_value;
    Vplus_R := Vplus_res_value;
    Vplus_G := 1.0 / Vplus_R;
    // Computing impedance for: Vin;
    Vin_cap_value := params.Vin_cap_value;
    Vin_G := 2.0 * Vin_cap_value * fs;
    Vin_R := 1.0 / Vin_G;
    // Computing impedance for: C2;
    C2_value := params.C2_value;
    C2_G := 2.0 * C2_value * fs;
    C2_R := 1.0 / C2_G;
    // Computing impedance for: S21;
    S21_res_value := params.R4_value;
    S21_cap_value := params.C5_value;
    S21_R := (T / (2.0 * S21_cap_value)) + S21_res_value;
    S21_G := 1.0 / S21_R;
    impedances.S21_2RC_over_T_plus_2RC = (2.0 * S21_cap_value * S21_res_value) / (2.0 * S21_cap_value * S21_res_value + T);

    // Computing impedance for: S22;
    S22_res_value := params.R5_value;
    S22_cap_value := params.C6_value;
    S22_R := (T / (2.0 * S22_cap_value)) + S22_res_value;
    S22_G := 1.0 / S22_R;
    impedances.S22_2RC_over_T_plus_2RC = (2.0 * S22_cap_value * S22_res_value) / (2.0 * S22_cap_value * S22_res_value + T);

    // Computing impedance for: P15;
    P15_res_value := params.Rd_value;
    P15_cap_value := params.C4_value;
    P15_2RC := 2.0 * P15_cap_value * P15_res_value;
    P15_R := P15_res_value * T / (P15_2RC + T);
    P15_G := 1.0 / P15_R;
    impedances.P15_2RC_over_2RC_plus_T = P15_2RC / (P15_2RC + T);

    // Computing impedance for: P18;
    P18_G := Vplus_G + Vin_G;
    P18_R := 1.0 / P18_G;
    impedances.P18_pr = Vplus_G * P18_R;

    // Computing impedance for: S19;
    S19_R := R3_R + P18_R;
    S19_G := 1.0 / S19_R;
    impedances.S19_pr = R3_R * S19_G;

    // Computing impedance for: P20;
    P20_G := S19_G + C2_G;
    P20_R := 1.0 / P20_G;
    impedances.P20_pr = S19_G * P20_R;

    // Computing impedance for: P23;
    P23_G := S21_G + S22_G;
    P23_R := 1.0 / P23_G;
    impedances.P23_pr = S21_G * P23_R;

    // Computing impedance for: R26;
    R26_R_calc := rtype_R26_update_vars(*impedances.R26_vars, params.R26_params, P20_R, P20_G, P23_R, P23_G, P15_R, P15_G);
    R26_R := R26_R_calc;
    R26_G := 1.0 / R26_R;
    // Computing impedance for: S25;
    S25_R := S24_R + R26_R;
    S25_G := 1.0 / S25_R;
    impedances.S25_pr = S24_R * S25_G;

    // Computing impedance for: XP;
    update_vars(*impedances.XP_vars, params.XP_params, S25_R, S25_G);
}

process :: (state: *State, impedances: *Impedances, Vin: float) -> float {
    P15_b := impedances.P15_2RC_over_2RC_plus_T * state.P15_z; // P15 reflected
    S22_b := -state.S22_z; // S22 reflected
    S21_b := -state.S21_z; // S21 reflected
    P23_b := S22_b - impedances.P23_pr * (S22_b - S21_b); // P23 reflected
    C2_b := state.C2_z; // C2 reflected
    Vin_b := state.Vin_z + Vin - state.Vin_v1; // Vin reflected
    state.Vin_v1 = Vin; // Vin state update
    Vplus_b := impedances.Vplus_V; // Vplus reflected
    P18_b := Vin_b - impedances.P18_pr * (Vin_b - Vplus_b); // P18 reflected
    R3_b := cast(float) 0; // R3 reflected
    S19_b := -(R3_b + P18_b); // S19 reflected
    P20_b := C2_b - impedances.P20_pr * (C2_b - S19_b); // P20 reflected
    R26_ins := float.[P20_b, P23_b, P15_b] #align 16; // R26 inputs
    R26_b := rtype_R26_reflected(*impedances.R26_vars, R26_ins.data); // R26 reflected
    S24_b := -state.S24_z; // S24 reflected
    S25_b := -(S24_b + R26_b); // S25 reflected
    S25_a := root_compute(*impedances.XP_vars, S25_b); // S25 incident
    S24_a := S24_b - impedances.S25_pr * (S25_a - S25_b); // S24 incident
    R26_a := -S25_a - S24_b + impedances.S25_pr * (S25_a - S25_b); // R26 incident
    state.S24_z = -S24_a + impedances.S24_2RC_over_T_plus_2RC * (S24_a + state.S24_z); // S24 state update
    R26_outs: [3] float = --- #align 16; // 3 outputs 
    rtype_R26_incident(*impedances.R26_vars, R26_a, R26_ins.data, R26_outs.data); // R26 compute
    P20_a := R26_outs[0];
    P23_a := R26_outs[1];
    P15_a := R26_outs[2];
    S19_a := P20_b - S19_b + P20_a; // S19 incident
    C2_a := P20_b - C2_b + P20_a; // C2 incident
    R3_a := R3_b - impedances.S19_pr * (S19_a - S19_b); // R3 incident
    P18_a := -S19_a - R3_b + impedances.S19_pr * (S19_a - S19_b); // P18 incident
    Vplus_a := P18_b - Vplus_b + P18_a; // Vplus incident
    Vin_a := P18_b - Vin_b + P18_a; // Vin incident
    state.Vin_z = Vin_a; // Vin state update
    state.C2_z = C2_a; // C2 state update
    S21_a := P23_b - S21_b + P23_a; // S21 incident
    S22_a := P23_b - S22_b + P23_a; // S22 incident
    state.S21_z = -S21_a + impedances.S21_2RC_over_T_plus_2RC * (S21_a + state.S21_z); // S21 state update
    state.S22_z = -S22_a + impedances.S22_2RC_over_T_plus_2RC * (S22_a + state.S22_z); // S22 state update
    state.P15_z = P15_b + P15_a - state.P15_z; // P15 state update
    
    v_S25 := (S25_a + S25_b) * cast(float) 0.5; // S25 voltage
    return v_S25;
}

