// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler rl_lowpass.wdf rl_lowpass.jai -lang jai

Params :: struct {
    R1_value : float : 1.0e+03;
    L1_value : float : 1.0e-03;
}

Impedances :: struct {
    P1_R : float;
    R1_G : float;
    P1_pr : float;
}

State :: struct {
    L1_z : float;
}

calc_impedances :: (impedances: *Impedances, fs: float, params: Params = .{}) {
    sum :: (a: $T, b: T) -> T { return a + b; }
    recip_sum :: (a: $T, b: T) -> T { return (a * b) / (a + b); }
    
    T := 1.0 / fs;
    
    // Computing impedance for: R1;
    R1_value := params.R1_value;
    R1_R := R1_value;
    R1_G := 1.0 / R1_R;
    impedances.R1_G = R1_G;
    // Computing impedance for: L1;
    L1_value := params.L1_value;
    L1_R := 2.0 * L1_value * fs;
    L1_G := 1.0 / L1_R;
    // Computing impedance for: P1;
    P1_G := R1_G + L1_G;
    P1_R := 1.0 / P1_G;
    impedances.P1_R = P1_R;
    impedances.P1_pr = R1_G * P1_R;

}

process :: (state: *State, impedances: *Impedances, Iin: float) -> float {
    L1_b := -state.L1_z; // L1 reflected
    R1_b := cast(float) 0; // R1 reflected
    P1_b := L1_b - impedances.P1_pr * (L1_b - R1_b); // P1 reflected
    Iin_a := P1_b; // Iin incident
    Iin_b := 2.0 * impedances.P1_R * Iin + Iin_a; // Iin reflected (@TODO: 2.0 * R?)
    P1_a := Iin_b; // P1 incident
    R1_a := P1_b - R1_b + P1_a; // R1 incident
    L1_a := P1_b - L1_b + P1_a; // L1 incident
    state.L1_z = L1_a; // L1 state update
    
    i_R1 := (R1_a - R1_b) * (cast(float) 0.5 * impedances.R1_G); // R1 current
    return i_R1;
}

