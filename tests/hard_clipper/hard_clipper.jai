// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler hard_clipper.wdf hard_clipper.jai -lang jai

#load "custom_hard_clipper.jai";

Params :: struct {
    C1_value : float;
    Vin_res_value : float = 1.0e-6;
    HC_params: HC_Params;
}

Impedances :: struct {
    P1_pr : float;
    HC_vars: HC_Vars;
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
    // Computing impedance for: Vin;
    Vin_res_value := params.Vin_res_value;
    Vin_R := Vin_res_value;
    Vin_G := 1.0 / Vin_R;
    // Computing impedance for: P1;
    P1_G := C1_G + Vin_G;
    P1_R := 1.0 / P1_G;
    impedances.P1_pr = C1_G * P1_R;

    // Computing impedance for: HC;
    update_vars(*impedances.HC_vars, params.HC_params, P1_R, P1_G);
}

process :: (state: *State, impedances: *Impedances, Vin: float) -> float {
    Vin_b := Vin; // Vin reflected
    C1_b := state.C1_z; // C1 reflected
    P1_b := Vin_b - impedances.P1_pr * (Vin_b - C1_b); // P1 reflected
    P1_a := root_compute(*impedances.HC_vars, P1_b); // P1 incident
    C1_a := P1_b - C1_b + P1_a; // C1 incident
    Vin_a := P1_b - Vin_b + P1_a; // Vin incident
    state.C1_z = C1_a; // C1 state update
    
    v_C1 := (C1_a + C1_b) * cast(float) 0.5; // C1 voltage
    return v_C1;
}

