// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler rc_lowpass_double.wdf rc_lowpass_double.jai -lang jai -dtype float64

Params :: struct {
    R1_value : float64 : 1.0e+03;
    C1_value : float64 : 1.0e-06;
}

Impedances :: struct {
    Vin_ca : float64;
    Vin_ce : float64;
}

State :: struct {
    C1_z : float64;
}

calc_impedances :: (impedances: *Impedances, fs: float64, params: Params = .{}) {
    sum :: (a: $T, b: T) -> T { return a + b; }
    recip_sum :: (a: $T, b: T) -> T { return (a * b) / (a + b); }
    
    T := 1.0 / fs;
    
    // Computing impedance for: C1;
    C1_value := params.C1_value;
    C1_G := 2.0 * C1_value * fs;
    C1_R := 1.0 / C1_G;
    
    // Computing impedance for: Vin;
    Vin_res_value := params.R1_value;
    impedances.Vin_ca = (Vin_res_value - C1_R) / (Vin_res_value + C1_R);
    impedances.Vin_ce = (2.0 * C1_R) / (Vin_res_value + C1_R);

}

process :: (state: *State, impedances: *Impedances, Vin: float64) -> float64 {
    C1_b := state.C1_z; // C1 reflected
    Vin_a := C1_b; // Vin incident
    Vin_b := impedances.Vin_ca * Vin_a + impedances.Vin_ce * -(Vin); // Vin reflected
    C1_a := Vin_b; // C1 incident
    state.C1_z = C1_a; // C1 state update
    
    v_C1 := (C1_a + C1_b) * cast(float64) 0.5; // C1 voltage
    return v_C1;
}

