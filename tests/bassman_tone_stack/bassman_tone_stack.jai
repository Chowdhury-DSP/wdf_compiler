// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler bassman_tone_stack.net bassman_tone_stack.jai -lang jai -netlist

#load "R14_rtype.jai";

Params :: struct {
    V1_res_value : float = 1.0e-6;
    R3m_value : float;
    R4_value : float : 5.6e+04;
    C2_value : float : 2.0e-08;
    C3_value : float : 2.0e-08;
    R1m_value : float;
    C1_value : float : 2.5e-10;
    R1p_value : float;
    R2_value : float;
    R3p_value : float;
    R14_params: rtype_R14_R_Params;
}

Impedances :: struct {
    S11_pr : float;
    S12_pr : float;
    S10_pr : float;
    R14_vars: rtype_R14_R_Vars;
}

State :: struct {
    C2_z : float;
    C3_z : float;
    C1_z : float;
}

calc_impedances :: (impedances: *Impedances, fs: float, params: Params = .{}) {
    sum :: (a: $T, b: T) -> T { return a + b; }
    recip_sum :: (a: $T, b: T) -> T { return (a * b) / (a + b); }
    
    T := 1.0 / fs;
    
    // Computing impedance for: V1;
    V1_res_value := params.V1_res_value;
    V1_R := V1_res_value;
    V1_G := 1.0 / V1_R;
    // Computing impedance for: R3m;
    R3m_value := params.R3m_value;
    R3m_R := R3m_value;
    R3m_G := 1.0 / R3m_R;
    // Computing impedance for: R4;
    R4_value := params.R4_value;
    R4_R := R4_value;
    R4_G := 1.0 / R4_R;
    // Computing impedance for: C2;
    C2_value := params.C2_value;
    C2_G := 2.0 * C2_value * fs;
    C2_R := 1.0 / C2_G;
    // Computing impedance for: C3;
    C3_value := params.C3_value;
    C3_G := 2.0 * C3_value * fs;
    C3_R := 1.0 / C3_G;
    // Computing impedance for: R1m;
    R1m_value := params.R1m_value;
    R1m_R := R1m_value;
    R1m_G := 1.0 / R1m_R;
    // Computing impedance for: C1;
    C1_value := params.C1_value;
    C1_G := 2.0 * C1_value * fs;
    C1_R := 1.0 / C1_G;
    // Computing impedance for: R1p;
    R1p_value := params.R1p_value;
    R1p_R := R1p_value;
    R1p_G := 1.0 / R1p_R;
    // Computing impedance for: S13;
    S13_value := sum(params.R2_value, params.R3p_value);
    S13_R := S13_value;
    S13_G := 1.0 / S13_R;
    // Computing impedance for: S11;
    S11_R := C1_R + R1p_R;
    S11_G := 1.0 / S11_R;
    impedances.S11_pr = C1_R * S11_G;

    // Computing impedance for: S12;
    S12_R := R1m_R + S11_R;
    S12_G := 1.0 / S12_R;
    impedances.S12_pr = R1m_R * S12_G;

    // Computing impedance for: S10;
    S10_R := V1_R + R3m_R;
    S10_G := 1.0 / S10_R;
    impedances.S10_pr = V1_R * S10_G;

    // Computing impedance for: R14;
    rtype_R14_update_vars(*impedances.R14_vars, params.R14_params, S10_R, S10_G, R4_R, R4_G, C2_R, C2_G, C3_R, C3_G, S12_R, S12_G, S13_R, S13_G);
}

process :: (state: *State, impedances: *Impedances, V1: float) -> float, float, float, float {
    S13_b := cast(float) 0; // S13 reflected
    R1p_b := cast(float) 0; // R1p reflected
    C1_b := state.C1_z; // C1 reflected
    S11_b := -(C1_b + R1p_b); // S11 reflected
    R1m_b := cast(float) 0; // R1m reflected
    S12_b := -(R1m_b + S11_b); // S12 reflected
    C3_b := state.C3_z; // C3 reflected
    C2_b := state.C2_z; // C2 reflected
    R4_b := cast(float) 0; // R4 reflected
    R3m_b := cast(float) 0; // R3m reflected
    V1_b := V1; // V1 reflected
    S10_b := -(V1_b + R3m_b); // S10 reflected
    R14_ins := float.[S10_b, R4_b, C2_b, C3_b, S12_b, S13_b] #align 16; // R14 inputs
    R14_outs: [6] float = --- #align 16; // R14 outputs 
    rtype_R14_compute(*impedances.R14_vars, R14_ins.data, R14_outs.data); // R14 compute
    S10_a := R14_outs[0];
    R4_a := R14_outs[1];
    C2_a := R14_outs[2];
    C3_a := R14_outs[3];
    S12_a := R14_outs[4];
    S13_a := R14_outs[5];
    V1_a := V1_b - impedances.S10_pr * (S10_a - S10_b); // V1 incident
    R3m_a := -S10_a - V1_b + impedances.S10_pr * (S10_a - S10_b); // R3m incident
    state.C2_z = C2_a; // C2 state update
    state.C3_z = C3_a; // C3 state update
    R1m_a := R1m_b - impedances.S12_pr * (S12_a - S12_b); // R1m incident
    S11_a := -S12_a - R1m_b + impedances.S12_pr * (S12_a - S12_b); // S11 incident
    C1_a := C1_b - impedances.S11_pr * (S11_a - S11_b); // C1 incident
    R1p_a := -S11_a - C1_b + impedances.S11_pr * (S11_a - S11_b); // R1p incident
    state.C1_z = C1_a; // C1 state update
    
    v_C1 := (C1_a + C1_b) * cast(float) 0.5; // C1 voltage
    v_R1p := (R1p_a + R1p_b) * cast(float) 0.5; // R1p voltage
    v_R1m := (R1m_a + R1m_b) * cast(float) 0.5; // R1m voltage
    v_R3m := (R3m_a + R3m_b) * cast(float) 0.5; // R3m voltage
    return v_C1, v_R1p, v_R1m, v_R3m;
}

