// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler diode_clipper.net diode_clipper.jai -lang jai -netlist

#load "../../lib/wdf_lib_diode_pair.jai";

Params :: struct {
    C1_value : float : 1.0e-06;
    Vin_res_value : float : 1.0e+03;
    DP_params: Diode_Pair_Params;
}

Impedances :: struct {
    P4_pr : float;
    DP_vars: Diode_Pair_Vars;
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
    // Computing impedance for: P4;
    P4_G := C1_G + Vin_G;
    P4_R := 1.0 / P4_G;
    impedances.P4_pr = C1_G * P4_R;

    // Computing impedance for: DP;
    update_vars(*impedances.DP_vars, params.DP_params, P4_R, P4_G);
}

process :: (state: *State, impedances: *Impedances, Vin: float) -> float {
    Vin_b := Vin; // Vin reflected
    C1_b := state.C1_z; // C1 reflected
    P4_b := Vin_b - impedances.P4_pr * (Vin_b - C1_b); // P4 reflected
    P4_a := root_compute(*impedances.DP_vars, P4_b); // P4 incident
    C1_a := P4_b - C1_b + P4_a; // C1 incident
    Vin_a := P4_b - Vin_b + P4_a; // Vin incident
    state.C1_z = C1_a; // C1 state update
    
    v_C1 := (C1_a + C1_b) * cast(float) 0.5; // C1 voltage
    return v_C1;
}

