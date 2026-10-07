// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler rc_bandpass_var.wdf rc_bandpass_var.jai -lang jai

Params :: struct {
    C1_value : float;
    R1_value : float;
    R2_value : float;
    C2_value : float;
}

Impedances :: struct {
    C1_R : float;
    R1_G : float;
    S2_G : float;
    S2_pr : float;
    P1_pr : float;
    S1_pr : float;
}

State :: struct {
    C1_z : float;
    C2_z : float;
}

calc_impedances :: (impedances: *Impedances, fs: float, params: Params = .{}) {
    sum :: (a: $T, b: T) -> T { return a + b; }
    recip_sum :: (a: $T, b: T) -> T { return (a * b) / (a + b); }
    
    T := 1.0 / fs;
    
    // Computing impedance for: C1;
    C1_value := params.C1_value;
    C1_G := 2.0 * C1_value * fs;
    C1_R := 1.0 / C1_G;
    impedances.C1_R = C1_R;
    // Computing impedance for: R1;
    R1_value := params.R1_value;
    R1_R := R1_value;
    R1_G := 1.0 / R1_R;
    impedances.R1_G = R1_G;
    // Computing impedance for: R2;
    R2_value := params.R2_value;
    R2_R := R2_value;
    R2_G := 1.0 / R2_R;
    // Computing impedance for: C2;
    C2_value := params.C2_value;
    C2_G := 2.0 * C2_value * fs;
    C2_R := 1.0 / C2_G;
    // Computing impedance for: S2;
    S2_R := R2_R + C2_R;
    S2_G := 1.0 / S2_R;
    impedances.S2_G = S2_G;
    impedances.S2_pr = R2_R * S2_G;

    // Computing impedance for: P1;
    P1_G := R1_G + S2_G;
    P1_R := 1.0 / P1_G;
    impedances.P1_pr = R1_G * P1_R;

    // Computing impedance for: S1;
    S1_R := C1_R + P1_R;
    S1_G := 1.0 / S1_R;
    impedances.S1_pr = C1_R * S1_G;

    // Computing impedance for: I1;
    I1_R := S1_R;
    I1_G := 1.0 / I1_R;
}

calc_impedances_lpf1 :: (impedances: *Impedances, fs: float, params: Params) {
    sum :: (a: $T, b: T) -> T { return a + b; }
    recip_sum :: (a: $T, b: T) -> T { return (a * b) / (a + b); }
    
    T := 1.0 / fs;
    
    // Computing impedance for: C1;
    C1_value := params.C1_value;
    C1_G := 2.0 * C1_value * fs;
    C1_R := 1.0 / C1_G;
    // Computing impedance for: R1;
    R1_value := params.R1_value;
    R1_R := R1_value;
    R1_G := 1.0 / R1_R;
    S2_G := impedances.S2_G;
    // Computing impedance for: P1;
    P1_G := R1_G + S2_G;
    P1_R := 1.0 / P1_G;
    impedances.P1_pr = R1_G * P1_R;

    // Computing impedance for: S1;
    S1_R := C1_R + P1_R;
    S1_G := 1.0 / S1_R;
    impedances.S1_pr = C1_R * S1_G;

    // Computing impedance for: I1;
    I1_R := S1_R;
    I1_G := 1.0 / I1_R;
}

calc_impedances_lpf2 :: (impedances: *Impedances, fs: float, params: Params) {
    sum :: (a: $T, b: T) -> T { return a + b; }
    recip_sum :: (a: $T, b: T) -> T { return (a * b) / (a + b); }
    
    T := 1.0 / fs;
    
    // Computing impedance for: R2;
    R2_value := params.R2_value;
    R2_R := R2_value;
    R2_G := 1.0 / R2_R;
    // Computing impedance for: C2;
    C2_value := params.C2_value;
    C2_G := 2.0 * C2_value * fs;
    C2_R := 1.0 / C2_G;
    // Computing impedance for: S2;
    S2_R := R2_R + C2_R;
    S2_G := 1.0 / S2_R;
    impedances.S2_G = S2_G;
    impedances.S2_pr = R2_R * S2_G;

    R1_G := impedances.R1_G;
    // Computing impedance for: P1;
    P1_G := R1_G + S2_G;
    P1_R := 1.0 / P1_G;
    impedances.P1_pr = R1_G * P1_R;

    C1_R := impedances.C1_R;
    // Computing impedance for: S1;
    S1_R := C1_R + P1_R;
    S1_G := 1.0 / S1_R;
    impedances.S1_pr = C1_R * S1_G;

    // Computing impedance for: I1;
    I1_R := S1_R;
    I1_G := 1.0 / I1_R;
}

process :: (state: *State, impedances: *Impedances, Vin: float) -> float {
    C2_b := state.C2_z; // C2 reflected
    R2_b := cast(float) 0; // R2 reflected
    S2_b := -(R2_b + C2_b); // S2 reflected
    R1_b := cast(float) 0; // R1 reflected
    P1_b := S2_b - impedances.P1_pr * (S2_b - R1_b); // P1 reflected
    C1_b := state.C1_z; // C1 reflected
    S1_b := -(C1_b + P1_b); // S1 reflected
    I1_b := -S1_b; // I1 reflected
    Vin_a := I1_b; // Vin incident
    Vin_b := -Vin_a + 2.0 * Vin; // Vin reflected
    I1_a := Vin_b; // I1 incident
    S1_a := -I1_a; // S1 incident
    C1_a := C1_b - impedances.S1_pr * (S1_a - S1_b); // C1 incident
    P1_a := -S1_a - C1_b + impedances.S1_pr * (S1_a - S1_b); // P1 incident
    state.C1_z = C1_a; // C1 state update
    R1_a := P1_b - R1_b + P1_a; // R1 incident
    S2_a := P1_b - S2_b + P1_a; // S2 incident
    R2_a := R2_b - impedances.S2_pr * (S2_a - S2_b); // R2 incident
    C2_a := -S2_a - R2_b + impedances.S2_pr * (S2_a - S2_b); // C2 incident
    state.C2_z = C2_a; // C2 state update
    
    v_C2 := (C2_a + C2_b) * cast(float) 0.5; // C2 voltage
    return v_C2;
}

