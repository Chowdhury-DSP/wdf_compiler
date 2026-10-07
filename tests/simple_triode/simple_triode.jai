// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler simple_triode.wdf simple_triode.jai -lang jai

#load "custom_dangelo_triode.jai";

Params :: struct {
    Vin_res_value : float : 2.5e+04;
    Rk_value : float : 2.0e+02;
    Vplus_v_value : float : 2.0e+02;
    Vplus_res_value : float : 4.7e+04;
    Rl_value : float : 3.3e+05;
    Cp_value : float : 4.7e-08;
    Triode_params: Triode_Params;
}

Impedances :: struct {
    Vplus_V : float : Params.Vplus_v_value;
    Sp_pr : float;
    Pp_pr : float;
    Triode_vars: Triode_Vars;
}

State :: struct {
    Cp_z : float;
}

calc_impedances :: (impedances: *Impedances, fs: float, params: Params = .{}) {
    sum :: (a: $T, b: T) -> T { return a + b; }
    recip_sum :: (a: $T, b: T) -> T { return (a * b) / (a + b); }
    
    T := 1.0 / fs;
    
    // Computing impedance for: Vin;
    Vin_res_value := params.Vin_res_value;
    Vin_R := Vin_res_value;
    Vin_G := 1.0 / Vin_R;
    // Computing impedance for: Rk;
    Rk_value := params.Rk_value;
    Rk_R := Rk_value;
    Rk_G := 1.0 / Rk_R;
    // Computing impedance for: Vplus;
    Vplus_res_value := params.Vplus_res_value;
    Vplus_R := Vplus_res_value;
    Vplus_G := 1.0 / Vplus_R;
    // Computing impedance for: Rl;
    Rl_value := params.Rl_value;
    Rl_R := Rl_value;
    Rl_G := 1.0 / Rl_R;
    // Computing impedance for: Cp;
    Cp_value := params.Cp_value;
    Cp_G := 2.0 * Cp_value * fs;
    Cp_R := 1.0 / Cp_G;
    // Computing impedance for: Sp;
    Sp_R := Rl_R + Cp_R;
    Sp_G := 1.0 / Sp_R;
    impedances.Sp_pr = Rl_R * Sp_G;

    // Computing impedance for: Pp;
    Pp_G := Vplus_G + Sp_G;
    Pp_R := 1.0 / Pp_G;
    impedances.Pp_pr = Vplus_G * Pp_R;

    // Computing impedance for: Triode;
    update_vars(*impedances.Triode_vars, params.Triode_params, Vin_R, Vin_G, Rk_R, Rk_G, Pp_R, Pp_G);
}

process :: (state: *State, impedances: *Impedances, Vin: float) -> float {
    Cp_b := state.Cp_z; // Cp reflected
    Rl_b := cast(float) 0; // Rl reflected
    Sp_b := -(Rl_b + Cp_b); // Sp reflected
    Vplus_b := impedances.Vplus_V; // Vplus reflected
    Pp_b := Sp_b - impedances.Pp_pr * (Sp_b - Vplus_b); // Pp reflected
    Rk_b := cast(float) 0; // Rk reflected
    Vin_b := Vin; // Vin reflected
    Triode_ins := float.[Vin_b, Rk_b, Pp_b] #align 16; // Triode inputs
    Triode_outs: [3] float = --- #align 16; // Triode outputs 
    root_compute(*impedances.Triode_vars, Triode_ins.data, Triode_outs.data); // Triode compute
    Vin_a := Triode_outs[0];
    Rk_a := Triode_outs[1];
    Pp_a := Triode_outs[2];
    Vplus_a := Pp_b - Vplus_b + Pp_a; // Vplus incident
    Sp_a := Pp_b - Sp_b + Pp_a; // Sp incident
    Rl_a := Rl_b - impedances.Sp_pr * (Sp_a - Sp_b); // Rl incident
    Cp_a := -Sp_a - Rl_b + impedances.Sp_pr * (Sp_a - Sp_b); // Cp incident
    state.Cp_z = Cp_a; // Cp state update
    
    v_Rl := (Rl_a + Rl_b) * cast(float) 0.5; // Rl voltage
    return v_Rl;
}

