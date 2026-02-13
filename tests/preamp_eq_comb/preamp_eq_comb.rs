// Auto-generated with wdf_compiler version 1.0.0.630eddd.
// Command: wdf_compiler preamp_eq_comb.wdf preamp_eq_comb.rs -lang rust

#[derive(Default,Copy,Clone)]
pub struct Params {
    pub v_in_v_value : f32, // = 0.0e+00
    pub lfc_cap_cap_value : f32, // = 4.7e-09
    pub r_series_value : f32, // = 1.0e+04
    pub hfl_res_cap_res_value : f32, // = 8.0e+04
    pub hfl_res_cap_cap_value : f32, // = 2.7e-09
    pub hfc_cap_value : f32, // = 3.3e-06
    pub r_shunt_value : f32, // = 4.674e+03
    pub lfl_res_cap_res_value : f32, // = 1.5e+04
    pub lfl_res_cap_cap_value : f32, // = 1.5e-07
}

#[derive(Default,Copy,Clone)]
pub struct Impedances {
    pub v_in_V : f32, // = Params.v_in_v_value
    pub hfl_res_cap_2RC_over_T_plus_2RC : f32,
    pub lfl_res_cap_2RC_over_2RC_plus_T : f32,
    pub s_s_pr : f32,
    pub p_s_pr : f32,
    pub p_p_pr : f32,
    pub s_o_pr : f32,
    pub s_i_pr : f32,
}

#[derive(Default,Copy,Clone)]
pub struct State {
    pub lfc_cap_z : f32,
    pub lfc_cap_v1 : f32,
    pub hfl_res_cap_z : f32,
    pub hfc_cap_z : f32,
    pub lfl_res_cap_z : f32,
}

#[allow(unused)]
pub fn calc_impedances(impedances: &mut Impedances, fs: f32, params: Params) {
    fn sum(a: f32, b: f32) -> f32 { a + b }
    fn recip_sum(a: f32, b: f32) -> f32 { (a * b) / (a + b) }
    
    let T = 1.0 / fs;
    
    // Computing impedance for: lfc_cap;
    let lfc_cap_cap_value = params.lfc_cap_cap_value;
    let lfc_cap_G = 2.0 * lfc_cap_cap_value * fs;
    let lfc_cap_R = 1.0 / lfc_cap_G;
    // Computing impedance for: r_series;
    let r_series_value = params.r_series_value;
    let r_series_R = r_series_value;
    let r_series_G = 1.0 / r_series_R;
    // Computing impedance for: hfl_res_cap;
    let hfl_res_cap_res_value = params.hfl_res_cap_res_value;
    let hfl_res_cap_cap_value = params.hfl_res_cap_cap_value;
    let hfl_res_cap_R = (T / (2.0 * hfl_res_cap_cap_value)) + hfl_res_cap_res_value;
    let hfl_res_cap_G = 1.0 / hfl_res_cap_R;
    impedances.hfl_res_cap_2RC_over_T_plus_2RC = (2.0 * hfl_res_cap_cap_value * hfl_res_cap_res_value) / (2.0 * hfl_res_cap_cap_value * hfl_res_cap_res_value + T);

    // Computing impedance for: hfc_cap;
    let hfc_cap_value = params.hfc_cap_value;
    let hfc_cap_G = 2.0 * hfc_cap_value * fs;
    let hfc_cap_R = 1.0 / hfc_cap_G;
    // Computing impedance for: r_shunt;
    let r_shunt_value = params.r_shunt_value;
    let r_shunt_R = r_shunt_value;
    let r_shunt_G = 1.0 / r_shunt_R;
    // Computing impedance for: lfl_res_cap;
    let lfl_res_cap_res_value = params.lfl_res_cap_res_value;
    let lfl_res_cap_cap_value = params.lfl_res_cap_cap_value;
    let lfl_res_cap_2RC = 2.0 * lfl_res_cap_cap_value * lfl_res_cap_res_value;
    let lfl_res_cap_R = lfl_res_cap_res_value * T / (lfl_res_cap_2RC + T);
    let lfl_res_cap_G = 1.0 / lfl_res_cap_R;
    impedances.lfl_res_cap_2RC_over_2RC_plus_T = lfl_res_cap_2RC / (lfl_res_cap_2RC + T);

    // Computing impedance for: s_s;
    let s_s_R = r_shunt_R + lfl_res_cap_R;
    let s_s_G = 1.0 / s_s_R;
    impedances.s_s_pr = r_shunt_R * s_s_G;

    // Computing impedance for: p_s;
    let p_s_G = hfc_cap_G + s_s_G;
    let p_s_R = 1.0 / p_s_G;
    impedances.p_s_pr = hfc_cap_G * p_s_R;

    // Computing impedance for: p_p;
    let p_p_G = r_series_G + hfl_res_cap_G;
    let p_p_R = 1.0 / p_p_G;
    impedances.p_p_pr = r_series_G * p_p_R;

    // Computing impedance for: s_o;
    let s_o_R = p_p_R + p_s_R;
    let s_o_G = 1.0 / s_o_R;
    impedances.s_o_pr = p_p_R * s_o_G;

    // Computing impedance for: s_i;
    let s_i_R = lfc_cap_R + s_o_R;
    let s_i_G = 1.0 / s_i_R;
    impedances.s_i_pr = lfc_cap_R * s_i_G;

    
    // Computing voltage for: v_in;
    impedances.v_in_V = params.v_in_v_value;

}

#[allow(unused)]
pub fn process(state: &mut State, impedances: &Impedances, lfc_cap: f32) -> f32 {
    let lfl_res_cap_b : f32 = impedances.lfl_res_cap_2RC_over_2RC_plus_T * state.lfl_res_cap_z; // lfl_res_cap reflected
    let r_shunt_b : f32 = 0 as f32; // r_shunt reflected
    let s_s_b : f32 = -(r_shunt_b + lfl_res_cap_b); // s_s reflected
    let hfc_cap_b : f32 = state.hfc_cap_z; // hfc_cap reflected
    let p_s_b : f32 = s_s_b - impedances.p_s_pr * (s_s_b - hfc_cap_b); // p_s reflected
    let hfl_res_cap_b : f32 = -state.hfl_res_cap_z; // hfl_res_cap reflected
    let r_series_b : f32 = 0 as f32; // r_series reflected
    let p_p_b : f32 = hfl_res_cap_b - impedances.p_p_pr * (hfl_res_cap_b - r_series_b); // p_p reflected
    let s_o_b : f32 = -(p_p_b + p_s_b); // s_o reflected
    let lfc_cap_b : f32 = state.lfc_cap_z + lfc_cap - state.lfc_cap_v1; // lfc_cap reflected
    state.lfc_cap_v1 = lfc_cap; // lfc_cap state update
    let s_i_b : f32 = -(lfc_cap_b + s_o_b); // s_i reflected
    let v_in_a : f32 = s_i_b; // v_in incident
    let v_in_b : f32 = -v_in_a + 2.0 * impedances.v_in_V; // v_in reflected
    let s_i_a : f32 = v_in_b; // s_i incident
    let lfc_cap_a : f32 = lfc_cap_b - impedances.s_i_pr * (s_i_a - s_i_b); // lfc_cap incident
    let s_o_a : f32 = -s_i_a - lfc_cap_b + impedances.s_i_pr * (s_i_a - s_i_b); // s_o incident
    state.lfc_cap_z = lfc_cap_a; // lfc_cap state update
    let p_p_a : f32 = p_p_b - impedances.s_o_pr * (s_o_a - s_o_b); // p_p incident
    let p_s_a : f32 = -s_o_a - p_p_b + impedances.s_o_pr * (s_o_a - s_o_b); // p_s incident
    let r_series_a : f32 = p_p_b - r_series_b + p_p_a; // r_series incident
    let hfl_res_cap_a : f32 = p_p_b - hfl_res_cap_b + p_p_a; // hfl_res_cap incident
    state.hfl_res_cap_z = -hfl_res_cap_a + impedances.hfl_res_cap_2RC_over_T_plus_2RC * (hfl_res_cap_a + state.hfl_res_cap_z); // hfl_res_cap state update
    let hfc_cap_a : f32 = p_s_b - hfc_cap_b + p_s_a; // hfc_cap incident
    let s_s_a : f32 = p_s_b - s_s_b + p_s_a; // s_s incident
    state.hfc_cap_z = hfc_cap_a; // hfc_cap state update
    let r_shunt_a : f32 = r_shunt_b - impedances.s_s_pr * (s_s_a - s_s_b); // r_shunt incident
    let lfl_res_cap_a : f32 = -s_s_a - r_shunt_b + impedances.s_s_pr * (s_s_a - s_s_b); // lfl_res_cap incident
    state.lfl_res_cap_z = lfl_res_cap_b + lfl_res_cap_a - state.lfl_res_cap_z; // lfl_res_cap state update
    
    let v_hfc_cap : f32 = (hfc_cap_a + hfc_cap_b) * (0.5 as f32); // hfc_cap voltage
    return v_hfc_cap;
}

