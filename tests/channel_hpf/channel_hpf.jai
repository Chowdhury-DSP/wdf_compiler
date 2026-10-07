// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler channel_hpf.wdf channel_hpf.jai -lang jai

Params :: struct {
    Sw1_closed : float;
    R8_value : float : 1.0e+04;
    Lh_value : float : 3.0e+00;
    Ll_value : float : 7.0e+00;
    C17_Vin_cap_value : float : 4.7e-07;
    C18_value : float : 4.7e-07;
    R9_value : float : 5.1e+03;
}

Impedances :: struct {
    Sw1_closed : float;
    S2_pr : float;
    P3_pr : float;
    P2_pr : float;
    S1_pr : float;
    P1_pr : float;
}

State :: struct {
    Lh_z : float;
    Ll_z : float;
    C17_Vin_z : float;
    C17_Vin_v1 : float;
    C18_z : float;
}

calc_impedances :: (impedances: *Impedances, fs: float, params: Params = .{}) {
    sum :: (a: $T, b: T) -> T { return a + b; }
    recip_sum :: (a: $T, b: T) -> T { return (a * b) / (a + b); }
    
    T := 1.0 / fs;
    
    // Computing impedance for: R8;
    R8_value := params.R8_value;
    R8_R := R8_value;
    R8_G := 1.0 / R8_R;
    // Computing impedance for: Lh;
    Lh_value := params.Lh_value;
    Lh_R := 2.0 * Lh_value * fs;
    Lh_G := 1.0 / Lh_R;
    // Computing impedance for: Ll;
    Ll_value := params.Ll_value;
    Ll_R := 2.0 * Ll_value * fs;
    Ll_G := 1.0 / Ll_R;
    // Computing impedance for: C17_Vin;
    C17_Vin_cap_value := params.C17_Vin_cap_value;
    C17_Vin_G := 2.0 * C17_Vin_cap_value * fs;
    C17_Vin_R := 1.0 / C17_Vin_G;
    // Computing impedance for: C18;
    C18_value := params.C18_value;
    C18_G := 2.0 * C18_value * fs;
    C18_R := 1.0 / C18_G;
    // Computing impedance for: R9;
    R9_value := params.R9_value;
    R9_R := R9_value;
    R9_G := 1.0 / R9_R;
    // Computing impedance for: S2;
    S2_R := C18_R + R9_R;
    S2_G := 1.0 / S2_R;
    impedances.S2_pr = C18_R * S2_G;

    // Computing impedance for: P3;
    P3_G := C17_Vin_G + S2_G;
    P3_R := 1.0 / P3_G;
    impedances.P3_pr = C17_Vin_G * P3_R;

    // Computing impedance for: P2;
    P2_G := R8_G + Lh_G;
    P2_R := 1.0 / P2_G;
    impedances.P2_pr = R8_G * P2_R;

    // Computing impedance for: S1;
    S1_R := Ll_R + P3_R;
    S1_G := 1.0 / S1_R;
    impedances.S1_pr = Ll_R * S1_G;

    // Computing impedance for: P1;
    P1_G := P2_G + S1_G;
    P1_R := 1.0 / P1_G;
    impedances.P1_pr = P2_G * P1_R;

    
    // Computing state for: Sw1;
    impedances.Sw1_closed = params.Sw1_closed;

}

process :: (state: *State, impedances: *Impedances, C17_Vin: float) -> float {
    R9_b := cast(float) 0; // R9 reflected
    C18_b := state.C18_z; // C18 reflected
    S2_b := -(C18_b + R9_b); // S2 reflected
    C17_Vin_b := state.C17_Vin_z + C17_Vin - state.C17_Vin_v1; // C17_Vin reflected
    state.C17_Vin_v1 = C17_Vin; // C17_Vin state update
    P3_b := S2_b - impedances.P3_pr * (S2_b - C17_Vin_b); // P3 reflected
    Ll_b := -state.Ll_z; // Ll reflected
    S1_b := -(Ll_b + P3_b); // S1 reflected
    Lh_b := -state.Lh_z; // Lh reflected
    R8_b := cast(float) 0; // R8 reflected
    P2_b := Lh_b - impedances.P2_pr * (Lh_b - R8_b); // P2 reflected
    P1_b := S1_b - impedances.P1_pr * (S1_b - P2_b); // P1 reflected
    Sw1_a := P1_b; // Sw1 incident
    Sw1_b := ifx impedances.Sw1_closed != 0 then -Sw1_a else Sw1_a; // Sw1 reflected
    P1_a := Sw1_b; // P1 incident
    P2_a := P1_b - P2_b + P1_a; // P2 incident
    S1_a := P1_b - S1_b + P1_a; // S1 incident
    R8_a := P2_b - R8_b + P2_a; // R8 incident
    Lh_a := P2_b - Lh_b + P2_a; // Lh incident
    state.Lh_z = Lh_a; // Lh state update
    Ll_a := Ll_b - impedances.S1_pr * (S1_a - S1_b); // Ll incident
    P3_a := -S1_a - Ll_b + impedances.S1_pr * (S1_a - S1_b); // P3 incident
    state.Ll_z = Ll_a; // Ll state update
    C17_Vin_a := P3_b - C17_Vin_b + P3_a; // C17_Vin incident
    S2_a := P3_b - S2_b + P3_a; // S2 incident
    state.C17_Vin_z = C17_Vin_a; // C17_Vin state update
    C18_a := C18_b - impedances.S2_pr * (S2_a - S2_b); // C18 incident
    R9_a := -S2_a - C18_b + impedances.S2_pr * (S2_a - S2_b); // R9 incident
    state.C18_z = C18_a; // C18 state update
    
    v_R9 := (R9_a + R9_b) * cast(float) 0.5; // R9 voltage
    return v_R9;
}

