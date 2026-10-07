// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler preamp_eq_comb.wdf preamp_eq_comb.jai -lang jai

Params :: struct {
    v_in_v_value : float : 0.0e+00;
    lfc_cap_cap_value : float : 4.7e-09;
    r_series_value : float : 1.0e+04;
    hfl_res_cap_res_value : float : 8.0e+04;
    hfl_res_cap_cap_value : float : 2.7e-09;
    hfc_cap_value : float : 3.3e-06;
    r_shunt_value : float : 4.674e+03;
    lfl_res_cap_res_value : float : 1.5e+04;
    lfl_res_cap_cap_value : float : 1.5e-07;
}

Impedances :: struct {
    v_in_V : float : Params.v_in_v_value;
    hfl_res_cap_2RC_over_T_plus_2RC : float;
    lfl_res_cap_2RC_over_2RC_plus_T : float;
    s_s_pr : float;
    p_s_pr : float;
    p_p_pr : float;
    s_o_pr : float;
    s_i_pr : float;
}

State :: struct {
    lfc_cap_z : float;
    lfc_cap_v1 : float;
    hfl_res_cap_z : float;
    hfc_cap_z : float;
    lfl_res_cap_z : float;
}

calc_impedances :: (impedances: *Impedances, fs: float, params: Params = .{}) {
    sum :: (a: $T, b: T) -> T { return a + b; }
    recip_sum :: (a: $T, b: T) -> T { return (a * b) / (a + b); }
    
    T := 1.0 / fs;
    
    // Computing impedance for: lfc_cap;
    lfc_cap_cap_value := params.lfc_cap_cap_value;
    lfc_cap_G := 2.0 * lfc_cap_cap_value * fs;
    lfc_cap_R := 1.0 / lfc_cap_G;
    // Computing impedance for: r_series;
    r_series_value := params.r_series_value;
    r_series_R := r_series_value;
    r_series_G := 1.0 / r_series_R;
    // Computing impedance for: hfl_res_cap;
    hfl_res_cap_res_value := params.hfl_res_cap_res_value;
    hfl_res_cap_cap_value := params.hfl_res_cap_cap_value;
    hfl_res_cap_R := (T / (2.0 * hfl_res_cap_cap_value)) + hfl_res_cap_res_value;
    hfl_res_cap_G := 1.0 / hfl_res_cap_R;
    impedances.hfl_res_cap_2RC_over_T_plus_2RC = (2.0 * hfl_res_cap_cap_value * hfl_res_cap_res_value) / (2.0 * hfl_res_cap_cap_value * hfl_res_cap_res_value + T);

    // Computing impedance for: hfc_cap;
    hfc_cap_value := params.hfc_cap_value;
    hfc_cap_G := 2.0 * hfc_cap_value * fs;
    hfc_cap_R := 1.0 / hfc_cap_G;
    // Computing impedance for: r_shunt;
    r_shunt_value := params.r_shunt_value;
    r_shunt_R := r_shunt_value;
    r_shunt_G := 1.0 / r_shunt_R;
    // Computing impedance for: lfl_res_cap;
    lfl_res_cap_res_value := params.lfl_res_cap_res_value;
    lfl_res_cap_cap_value := params.lfl_res_cap_cap_value;
    lfl_res_cap_2RC := 2.0 * lfl_res_cap_cap_value * lfl_res_cap_res_value;
    lfl_res_cap_R := lfl_res_cap_res_value * T / (lfl_res_cap_2RC + T);
    lfl_res_cap_G := 1.0 / lfl_res_cap_R;
    impedances.lfl_res_cap_2RC_over_2RC_plus_T = lfl_res_cap_2RC / (lfl_res_cap_2RC + T);

    // Computing impedance for: s_s;
    s_s_R := r_shunt_R + lfl_res_cap_R;
    s_s_G := 1.0 / s_s_R;
    impedances.s_s_pr = r_shunt_R * s_s_G;

    // Computing impedance for: p_s;
    p_s_G := hfc_cap_G + s_s_G;
    p_s_R := 1.0 / p_s_G;
    impedances.p_s_pr = hfc_cap_G * p_s_R;

    // Computing impedance for: p_p;
    p_p_G := r_series_G + hfl_res_cap_G;
    p_p_R := 1.0 / p_p_G;
    impedances.p_p_pr = r_series_G * p_p_R;

    // Computing impedance for: s_o;
    s_o_R := p_p_R + p_s_R;
    s_o_G := 1.0 / s_o_R;
    impedances.s_o_pr = p_p_R * s_o_G;

    // Computing impedance for: s_i;
    s_i_R := lfc_cap_R + s_o_R;
    s_i_G := 1.0 / s_i_R;
    impedances.s_i_pr = lfc_cap_R * s_i_G;

}

process :: (state: *State, impedances: *Impedances, lfc_cap: float) -> float {
    lfl_res_cap_b := impedances.lfl_res_cap_2RC_over_2RC_plus_T * state.lfl_res_cap_z; // lfl_res_cap reflected
    r_shunt_b := cast(float) 0; // r_shunt reflected
    s_s_b := -(r_shunt_b + lfl_res_cap_b); // s_s reflected
    hfc_cap_b := state.hfc_cap_z; // hfc_cap reflected
    p_s_b := s_s_b - impedances.p_s_pr * (s_s_b - hfc_cap_b); // p_s reflected
    hfl_res_cap_b := -state.hfl_res_cap_z; // hfl_res_cap reflected
    r_series_b := cast(float) 0; // r_series reflected
    p_p_b := hfl_res_cap_b - impedances.p_p_pr * (hfl_res_cap_b - r_series_b); // p_p reflected
    s_o_b := -(p_p_b + p_s_b); // s_o reflected
    lfc_cap_b := state.lfc_cap_z + lfc_cap - state.lfc_cap_v1; // lfc_cap reflected
    state.lfc_cap_v1 = lfc_cap; // lfc_cap state update
    s_i_b := -(lfc_cap_b + s_o_b); // s_i reflected
    v_in_a := s_i_b; // v_in incident
    v_in_b := -v_in_a + 2.0 * impedances.v_in_V; // v_in reflected
    s_i_a := v_in_b; // s_i incident
    lfc_cap_a := lfc_cap_b - impedances.s_i_pr * (s_i_a - s_i_b); // lfc_cap incident
    s_o_a := -s_i_a - lfc_cap_b + impedances.s_i_pr * (s_i_a - s_i_b); // s_o incident
    state.lfc_cap_z = lfc_cap_a; // lfc_cap state update
    p_p_a := p_p_b - impedances.s_o_pr * (s_o_a - s_o_b); // p_p incident
    p_s_a := -s_o_a - p_p_b + impedances.s_o_pr * (s_o_a - s_o_b); // p_s incident
    r_series_a := p_p_b - r_series_b + p_p_a; // r_series incident
    hfl_res_cap_a := p_p_b - hfl_res_cap_b + p_p_a; // hfl_res_cap incident
    state.hfl_res_cap_z = -hfl_res_cap_a + impedances.hfl_res_cap_2RC_over_T_plus_2RC * (hfl_res_cap_a + state.hfl_res_cap_z); // hfl_res_cap state update
    hfc_cap_a := p_s_b - hfc_cap_b + p_s_a; // hfc_cap incident
    s_s_a := p_s_b - s_s_b + p_s_a; // s_s incident
    state.hfc_cap_z = hfc_cap_a; // hfc_cap state update
    r_shunt_a := r_shunt_b - impedances.s_s_pr * (s_s_a - s_s_b); // r_shunt incident
    lfl_res_cap_a := -s_s_a - r_shunt_b + impedances.s_s_pr * (s_s_a - s_s_b); // lfl_res_cap incident
    state.lfl_res_cap_z = lfl_res_cap_b + lfl_res_cap_a - state.lfl_res_cap_z; // lfl_res_cap state update
    
    v_hfc_cap := (hfc_cap_a + hfc_cap_b) * cast(float) 0.5; // hfc_cap voltage
    return v_hfc_cap;
}

