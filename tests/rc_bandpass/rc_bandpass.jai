// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler rc_bandpass.net rc_bandpass.jai -lang jai -netlist

Params :: struct {
    C1_value : float : 1.0e-06;
    R1_value : float : 1.0e+03;
    C2_value : float : 1.0e-06;
    R2_value : float : 1.0e+03;
}

Impedances :: struct {
    Vin_ca : float;
    Vin_ce : float;
    Vin_cr : float;
    S7_pr : float;
}

State :: struct {
    Vin_z : float;
    C2_z : float;
}

calc_impedances :: (impedances: *Impedances, fs: float, params: Params = .{}) {
    sum :: (a: $T, b: T) -> T { return a + b; }
    recip_sum :: (a: $T, b: T) -> T { return (a * b) / (a + b); }
    
    T := 1.0 / fs;
    
    // Computing impedance for: C2;
    C2_value := params.C2_value;
    C2_G := 2.0 * C2_value * fs;
    C2_R := 1.0 / C2_G;
    // Computing impedance for: R2;
    R2_value := params.R2_value;
    R2_R := R2_value;
    R2_G := 1.0 / R2_R;
    // Computing impedance for: S7;
    S7_R := C2_R + R2_R;
    S7_G := 1.0 / S7_R;
    impedances.S7_pr = C2_R * S7_G;

    
    // Computing impedance for: Vin;
    Vin_res_value := params.R1_value;
    Vin_cap_value := params.C1_value;
    Vin_Gp := (1.0 / Vin_res_value) + S7_G;
    Vin_Rp := 1.0 / Vin_Gp;
    impedances.Vin_ca = T / (T + 2.0 * Vin_Rp * Vin_cap_value);
    impedances.Vin_ce = (2.0 * Vin_Rp * Vin_cap_value) / (T + 2.0 * Vin_Rp * Vin_cap_value);
    impedances.Vin_cr = Vin_Rp / Vin_res_value;

}

process :: (state: *State, impedances: *Impedances, Vin: float) -> float {
    R2_b := cast(float) 0; // R2 reflected
    C2_b := state.C2_z; // C2 reflected
    S7_b := -(C2_b + R2_b); // S7 reflected
    Vin_a := S7_b; // Vin incident
    Vin_a_cvs := impedances.Vin_cr * Vin_a; // Vin_CVs incident
    Vin_a0a1 := 2.0 * (Vin + state.Vin_z) + Vin_a_cvs; // Vin_CVs reflected
    Vin_b_cvs := impedances.Vin_ca * Vin_a_cvs - impedances.Vin_ce * Vin_a0a1; // Vin_CVs reflected
    Vin_b := Vin_b_cvs - Vin_a_cvs; // Vin reflected
    state.Vin_z += -Vin_b_cvs - Vin_a0a1; // Vin_z state update
    S7_a := Vin_b; // S7 incident
    C2_a := C2_b - impedances.S7_pr * (S7_a - S7_b); // C2 incident
    R2_a := -S7_a - C2_b + impedances.S7_pr * (S7_a - S7_b); // R2 incident
    state.C2_z = C2_a; // C2 state update
    
    v_C2 := (C2_a + C2_b) * cast(float) 0.5; // C2 voltage
    return v_C2;
}

