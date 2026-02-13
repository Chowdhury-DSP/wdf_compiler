// Auto-generated with wdf_compiler version 1.0.0.30554a3.
// Command: wdf_compiler bassman_tone_stack.wdf bassman_tone_stack.rs -lang rust

use crate::custom_bassman_rtype;

#[derive(Default,Copy,Clone)]
pub struct Params {
    pub Vin_Res3m_res_value : f32,
    pub Res2_Res3p_value : f32,
    pub Res1p_Res1m_value : f32,
    pub Cap1_value : f32, // = 2.5e-10
    pub Cap2_value : f32, // = 2.0e-08
    pub Res4_value : f32, // = 5.6e+04
    pub Cap3_value : f32, // = 2.0e-08
    pub R_params: custom_bassman_rtype::R_Params,
}

#[derive(Default,Copy,Clone)]
pub struct Impedances {
    pub Res1p_Res1m_Cap1_2RC_over_T_plus_2RC : f32,
    pub R_vars: custom_bassman_rtype::R_Vars,
}

#[derive(Default,Copy,Clone)]
pub struct State {
    pub Res1p_Res1m_Cap1_z : f32,
    pub Cap2_z : f32,
    pub Cap3_z : f32,
}

#[allow(unused)]
pub fn calc_impedances(impedances: &mut Impedances, fs: f32, params: Params) {
    fn sum(a: f32, b: f32) -> f32 { a + b }
    fn recip_sum(a: f32, b: f32) -> f32 { (a * b) / (a + b) }
    
    let T = 1.0 / fs;
    
    // Computing impedance for: Vin_Res3m;
    let Vin_Res3m_res_value = params.Vin_Res3m_res_value;
    let Vin_Res3m_R = Vin_Res3m_res_value;
    let Vin_Res3m_G = 1.0 / Vin_Res3m_R;
    // Computing impedance for: Res2_Res3p;
    let Res2_Res3p_value = params.Res2_Res3p_value;
    let Res2_Res3p_R = Res2_Res3p_value;
    let Res2_Res3p_G = 1.0 / Res2_Res3p_R;
    // Computing impedance for: Res1p_Res1m_Cap1;
    let Res1p_Res1m_Cap1_res_value = params.Res1p_Res1m_value;
    let Res1p_Res1m_Cap1_cap_value = params.Cap1_value;
    let Res1p_Res1m_Cap1_R = (T / (2.0 * Res1p_Res1m_Cap1_cap_value)) + Res1p_Res1m_Cap1_res_value;
    let Res1p_Res1m_Cap1_G = 1.0 / Res1p_Res1m_Cap1_R;
    impedances.Res1p_Res1m_Cap1_2RC_over_T_plus_2RC = (2.0 * Res1p_Res1m_Cap1_cap_value * Res1p_Res1m_Cap1_res_value) / (2.0 * Res1p_Res1m_Cap1_cap_value * Res1p_Res1m_Cap1_res_value + T);

    // Computing impedance for: Cap2;
    let Cap2_value = params.Cap2_value;
    let Cap2_G = 2.0 * Cap2_value * fs;
    let Cap2_R = 1.0 / Cap2_G;
    // Computing impedance for: Res4;
    let Res4_value = params.Res4_value;
    let Res4_R = Res4_value;
    let Res4_G = 1.0 / Res4_R;
    // Computing impedance for: Cap3;
    let Cap3_value = params.Cap3_value;
    let Cap3_G = 2.0 * Cap3_value * fs;
    let Cap3_R = 1.0 / Cap3_G;
    // Computing impedance for: R;
    custom_bassman_rtype::update_vars(&mut impedances.R_vars, params.R_params, Vin_Res3m_R, Vin_Res3m_G, Res2_Res3p_R, Res2_Res3p_G, Res1p_Res1m_Cap1_R, Res1p_Res1m_Cap1_G, Cap2_R, Cap2_G, Res4_R, Res4_G, Cap3_R, Cap3_G);
}

#[allow(unused)]
pub fn process(state: &mut State, impedances: &Impedances, Vin_Res3m: f32) -> (f32, f32) {
    let Cap3_b : f32 = state.Cap3_z; // Cap3 reflected
    let Res4_b : f32 = 0 as f32; // Res4 reflected
    let Cap2_b : f32 = state.Cap2_z; // Cap2 reflected
    let Res1p_Res1m_Cap1_b : f32 = -state.Res1p_Res1m_Cap1_z; // Res1p_Res1m_Cap1 reflected
    let Res2_Res3p_b : f32 = 0 as f32; // Res2_Res3p reflected
    let Vin_Res3m_b : f32 = Vin_Res3m; // Vin_Res3m reflected
    let R_ins : [f32; 6] = [Vin_Res3m_b, Res2_Res3p_b, Res1p_Res1m_Cap1_b, Cap2_b, Res4_b, Cap3_b]; // R inputs
    let mut R_outs : [f32; 6] = [0 as f32; 6]; // R outputs 
    custom_bassman_rtype::root_compute(&impedances.R_vars, &R_ins, &mut R_outs); // R compute
    let Vin_Res3m_a = R_outs[0];
    let Res2_Res3p_a = R_outs[1];
    let Res1p_Res1m_Cap1_a = R_outs[2];
    let Cap2_a = R_outs[3];
    let Res4_a = R_outs[4];
    let Cap3_a = R_outs[5];
    state.Res1p_Res1m_Cap1_z = -Res1p_Res1m_Cap1_a + impedances.Res1p_Res1m_Cap1_2RC_over_T_plus_2RC * (Res1p_Res1m_Cap1_a + state.Res1p_Res1m_Cap1_z); // Res1p_Res1m_Cap1 state update
    state.Cap2_z = Cap2_a; // Cap2 state update
    state.Cap3_z = Cap3_a; // Cap3 state update
    
    let v_Vin_Res3m : f32 = (Vin_Res3m_a + Vin_Res3m_b) * (0.5 as f32); // Vin_Res3m voltage
    let v_Res1p_Res1m_Cap1 : f32 = (Res1p_Res1m_Cap1_a + Res1p_Res1m_Cap1_b) * (0.5 as f32); // Res1p_Res1m_Cap1 voltage
    return (v_Vin_Res3m, v_Res1p_Res1m_Cap1);
}

