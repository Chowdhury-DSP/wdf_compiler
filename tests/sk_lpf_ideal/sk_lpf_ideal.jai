// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler sk_lpf_ideal.net sk_lpf_ideal.jai -lang jai -netlist

#load "R9_rtype.jai";

Params :: struct {
    R1_value : float : 1.0e+04;
    Rload_value : float : 1.0e+05;
    R2_value : float : 1.0e+04;
    C1_value : float : 9.999999e-08;
    C2_value : float : 9.999999e-08;
    Rf_value : float : 8.0e+03;
    Rg_value : float : 6.0e+03;
    R9_params: rtype_R9_R_Params;
}

Impedances :: struct {
    Vin_ca : float;
    Vin_ce : float;
    R9_vars: rtype_R9_R_Vars;
}

State :: struct {
    C1_z : float;
    C2_z : float;
}

calc_impedances :: (impedances: *Impedances, fs: float, params: Params = .{}) {
    sum :: (a: $T, b: T) -> T { return a + b; }
    recip_sum :: (a: $T, b: T) -> T { return (a * b) / (a + b); }
    
    T := 1.0 / fs;
    
    // Computing impedance for: Rload;
    Rload_value := params.Rload_value;
    Rload_R := Rload_value;
    Rload_G := 1.0 / Rload_R;
    // Computing impedance for: R2;
    R2_value := params.R2_value;
    R2_R := R2_value;
    R2_G := 1.0 / R2_R;
    // Computing impedance for: C1;
    C1_value := params.C1_value;
    C1_G := 2.0 * C1_value * fs;
    C1_R := 1.0 / C1_G;
    // Computing impedance for: C2;
    C2_value := params.C2_value;
    C2_G := 2.0 * C2_value * fs;
    C2_R := 1.0 / C2_G;
    // Computing impedance for: Rf;
    Rf_value := params.Rf_value;
    Rf_R := Rf_value;
    Rf_G := 1.0 / Rf_R;
    // Computing impedance for: Rg;
    Rg_value := params.Rg_value;
    Rg_R := Rg_value;
    Rg_G := 1.0 / Rg_R;
    // Computing impedance for: R9;
    R9_R_calc := rtype_R9_update_vars(*impedances.R9_vars, params.R9_params, Rload_R, Rload_G, R2_R, R2_G, C1_R, C1_G, C2_R, C2_G, Rf_R, Rf_G, Rg_R, Rg_G);
    R9_R := R9_R_calc;
    R9_G := 1.0 / R9_R;
    
    // Computing impedance for: Vin;
    Vin_res_value := params.R1_value;
    impedances.Vin_ca = (Vin_res_value - R9_R) / (Vin_res_value + R9_R);
    impedances.Vin_ce = (2.0 * R9_R) / (Vin_res_value + R9_R);

}

process :: (state: *State, impedances: *Impedances, Vin: float) -> float {
    Rg_b := cast(float) 0; // Rg reflected
    Rf_b := cast(float) 0; // Rf reflected
    C2_b := state.C2_z; // C2 reflected
    C1_b := state.C1_z; // C1 reflected
    R2_b := cast(float) 0; // R2 reflected
    Rload_b := cast(float) 0; // Rload reflected
    R9_ins := float.[Rload_b, R2_b, C1_b, C2_b, Rf_b, Rg_b] #align 16; // R9 inputs
    R9_b := rtype_R9_reflected(*impedances.R9_vars, R9_ins.data); // R9 reflected
    Vin_a := R9_b; // Vin incident
    Vin_b := impedances.Vin_ca * Vin_a + impedances.Vin_ce * -(Vin); // Vin reflected
    R9_a := Vin_b; // R9 incident
    R9_outs: [6] float = --- #align 16; // 6 outputs 
    rtype_R9_incident(*impedances.R9_vars, R9_a, R9_ins.data, R9_outs.data); // R9 compute
    Rload_a := R9_outs[0];
    R2_a := R9_outs[1];
    C1_a := R9_outs[2];
    C2_a := R9_outs[3];
    Rf_a := R9_outs[4];
    Rg_a := R9_outs[5];
    state.C1_z = C1_a; // C1 state update
    state.C2_z = C2_a; // C2 state update
    
    v_Rload := (Rload_a + Rload_b) * cast(float) 0.5; // Rload voltage
    return v_Rload;
}

