// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler rc_lowpass.net rc_lowpass.jai -lang jai -netlist

Params :: struct {
    R1_value : float : 1.0e+03;
    C1_value : float : 1.0e-06;
}

Impedances :: struct {
    V1_ca : float;
    V1_ce : float;
}

State :: struct {
    C1_z : float;
}

calc_impedances :: (impedances: *Impedances, fs: float, params: Params = .{}) {
    sum :: (a: $T, b: T) -> T { return a + b; }
    recip_sum :: (a: $T, b: T) -> T { return (a * b) / (a + b); }
    
    T := 1.0 / fs;
    
    // Computing impedance for: C1;
    C1_value := params.C1_value;
    C1_G := 2.0 * C1_value * fs;
    C1_R := 1.0 / C1_G;
    
    // Computing impedance for: V1;
    V1_res_value := params.R1_value;
    impedances.V1_ca = (V1_res_value - C1_R) / (V1_res_value + C1_R);
    impedances.V1_ce = (2.0 * C1_R) / (V1_res_value + C1_R);

}

process :: (state: *State, impedances: *Impedances, V1: float) -> float {
    C1_b := state.C1_z; // C1 reflected
    V1_a := C1_b; // V1 incident
    V1_b := impedances.V1_ca * V1_a + impedances.V1_ce * -(V1); // V1 reflected
    C1_a := V1_b; // C1 incident
    state.C1_z = C1_a; // C1 state update
    
    v_C1 := (C1_a + C1_b) * cast(float) 0.5; // C1 voltage
    return v_C1;
}

