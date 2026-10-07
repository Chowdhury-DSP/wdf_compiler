// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler rc_lowpass_2ins.wdf rc_lowpass_2ins.jai -lang jai

Params :: struct {
    R1_res_value : float : 1.0e+03;
    C1_value : float : 1.0e-06;
}

Impedances :: struct {
    S1_pr : float;
}

State :: struct {
    C1_z : float;
}

calc_impedances :: (impedances: *Impedances, fs: float, params: Params = .{}) {
    sum :: (a: $T, b: T) -> T { return a + b; }
    recip_sum :: (a: $T, b: T) -> T { return (a * b) / (a + b); }
    
    T := 1.0 / fs;
    
    // Computing impedance for: R1;
    R1_res_value := params.R1_res_value;
    R1_R := R1_res_value;
    R1_G := 1.0 / R1_R;
    // Computing impedance for: C1;
    C1_value := params.C1_value;
    C1_G := 2.0 * C1_value * fs;
    C1_R := 1.0 / C1_G;
    // Computing impedance for: S1;
    S1_R := R1_R + C1_R;
    S1_G := 1.0 / S1_R;
    impedances.S1_pr = R1_R * S1_G;

}

process :: (state: *State, impedances: *Impedances, Vin: float, R1: float) -> float {
    C1_b := state.C1_z; // C1 reflected
    R1_b := R1; // R1 reflected
    S1_b := -(R1_b + C1_b); // S1 reflected
    Vin_a := S1_b; // Vin incident
    Vin_b := -Vin_a + 2.0 * Vin; // Vin reflected
    S1_a := Vin_b; // S1 incident
    R1_a := R1_b - impedances.S1_pr * (S1_a - S1_b); // R1 incident
    C1_a := -S1_a - R1_b + impedances.S1_pr * (S1_a - S1_b); // C1 incident
    state.C1_z = C1_a; // C1 state update
    
    v_C1 := (C1_a + C1_b) * cast(float) 0.5; // C1 voltage
    return v_C1;
}

