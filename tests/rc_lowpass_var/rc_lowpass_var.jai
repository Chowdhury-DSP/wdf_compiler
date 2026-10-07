// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler rc_lowpass_var.wdf rc_lowpass_var.jai -lang jai

Params :: struct {
    Vplus_v_value : float : 1.0e+01;
    Vin_res_value : float : 1.0e+03;
    Vminus_v_value : float : -5.0e+00;
    Vminus_res_value : float = 1.0e-6;
    R1_v_value : float;
    R1_res_value : float = 1.0e-6;
    C1_value : float;
}

Impedances :: struct {
    Vplus_V : float : Params.Vplus_v_value;
    Vminus_V : float : Params.Vminus_v_value;
    R1_V : float;
    S3_pr : float;
    S2_pr : float;
    S1_pr : float;
}

State :: struct {
    C1_z : float;
}

calc_impedances :: (impedances: *Impedances, fs: float, params: Params = .{}) {
    sum :: (a: $T, b: T) -> T { return a + b; }
    recip_sum :: (a: $T, b: T) -> T { return (a * b) / (a + b); }
    
    T := 1.0 / fs;
    
    // Computing impedance for: Vin;
    Vin_res_value := params.Vin_res_value;
    Vin_R := Vin_res_value;
    Vin_G := 1.0 / Vin_R;
    // Computing impedance for: Vminus;
    Vminus_res_value := params.Vminus_res_value;
    Vminus_R := Vminus_res_value;
    Vminus_G := 1.0 / Vminus_R;
    // Computing voltage for: R1;
    impedances.R1_V = params.R1_v_value;

    // Computing impedance for: R1;
    R1_res_value := params.R1_res_value;
    R1_R := R1_res_value;
    R1_G := 1.0 / R1_R;
    // Computing impedance for: C1;
    C1_value := params.C1_value;
    C1_G := 2.0 * C1_value * fs;
    C1_R := 1.0 / C1_G;
    // Computing impedance for: S3;
    S3_R := R1_R + C1_R;
    S3_G := 1.0 / S3_R;
    impedances.S3_pr = R1_R * S3_G;

    // Computing impedance for: S2;
    S2_R := Vminus_R + S3_R;
    S2_G := 1.0 / S2_R;
    impedances.S2_pr = Vminus_R * S2_G;

    // Computing impedance for: S1;
    S1_R := Vin_R + S2_R;
    S1_G := 1.0 / S1_R;
    impedances.S1_pr = Vin_R * S1_G;

}

process :: (state: *State, impedances: *Impedances, Vin: float) -> float {
    C1_b := state.C1_z; // C1 reflected
    R1_b := impedances.R1_V; // R1 reflected
    S3_b := -(R1_b + C1_b); // S3 reflected
    Vminus_b := impedances.Vminus_V; // Vminus reflected
    S2_b := -(Vminus_b + S3_b); // S2 reflected
    Vin_b := Vin; // Vin reflected
    S1_b := -(Vin_b + S2_b); // S1 reflected
    Vplus_a := S1_b; // Vplus incident
    Vplus_b := -Vplus_a + 2.0 * impedances.Vplus_V; // Vplus reflected
    S1_a := Vplus_b; // S1 incident
    Vin_a := Vin_b - impedances.S1_pr * (S1_a - S1_b); // Vin incident
    S2_a := -S1_a - Vin_b + impedances.S1_pr * (S1_a - S1_b); // S2 incident
    Vminus_a := Vminus_b - impedances.S2_pr * (S2_a - S2_b); // Vminus incident
    S3_a := -S2_a - Vminus_b + impedances.S2_pr * (S2_a - S2_b); // S3 incident
    R1_a := R1_b - impedances.S3_pr * (S3_a - S3_b); // R1 incident
    C1_a := -S3_a - R1_b + impedances.S3_pr * (S3_a - S3_b); // C1 incident
    state.C1_z = C1_a; // C1 state update
    
    v_C1 := (C1_a + C1_b) * cast(float) 0.5; // C1 voltage
    return v_C1;
}

