// Auto-generated with wdf_compiler version 1.0.0.9dd0ac9.
// Command: wdf_compiler analog_eq.wdf analog_eq.rs -lang rust

use crate::custom_analog_eq_rtype;
use crate::custom_nonlinear_inductor;

#[derive(Default,Copy,Clone)]
pub struct Params {
    pub Ra_value : f32, // = 6.2e+02
    pub P_treble_boost_plus_value : f32,
    pub P_treble_boost_minus_value : f32,
    pub R_treble_boost_bw_value : f32,
    pub C_treble_boost_value : f32,
    pub L_treble_boost_params: custom_nonlinear_inductor::L_Params,
    pub Rlc_value : f32, // = 1.0e+03
    pub P_low_cut_value : f32,
    pub C_low_cut_value : f32,
    pub P_treble_cut_plus_value : f32,
    pub P_treble_cut_minus_value : f32,
    pub Rc_value : f32, // = 7.5e+01
    pub C_treble_cut_value : f32,
    pub P_low_boost_value : f32,
    pub C_low_boost_value : f32,
    pub Rb_value : f32, // = 1.0e+04
    pub Rload_value : f32, // = 1.0e+06
    pub R_params: custom_analog_eq_rtype::R_Params,
}

#[derive(Default,Copy,Clone)]
pub struct Impedances {
    pub Pb_2RC_over_2RC_plus_T : f32,
    pub Streble_cut_2RC_over_T_plus_2RC : f32,
    pub Pbass_boost_2RC_over_2RC_plus_T : f32,
    pub Streble_boost_1_pr : f32,
    pub Streble_boost_pr : f32,
    pub Sb_pr : f32,
    pub Ptreble_boost_pr : f32,
    pub Ptreble_cut_pr : f32,
    pub R_vars: custom_analog_eq_rtype::R_Vars,
    pub Svin_pr : f32,
    pub SR_pr : f32,
    pub Pvin_pr : f32,
}

#[derive(Default,Copy,Clone)]
pub struct State {
    pub C_treble_boost_z : f32,
    pub L_treble_boost_state: custom_nonlinear_inductor::L_State,
    pub Pb_z : f32,
    pub Streble_cut_z : f32,
    pub Pbass_boost_z : f32,
    pub R_state: custom_analog_eq_rtype::R_State,
}

#[allow(unused)]
pub fn calc_impedances(impedances: &mut Impedances, fs: f32, params: Params) {
    fn sum(a: f32, b: f32) -> f32 { a + b }
    fn recip_sum(a: f32, b: f32) -> f32 { (a * b) / (a + b) }
    
    let T = 1.0 / fs;
    
    // Computing impedance for: Ra;
    let Ra_value = params.Ra_value;
    let Ra_R = Ra_value;
    let Ra_G = 1.0 / Ra_R;
    // Computing impedance for: P_treble_boost_plus;
    let P_treble_boost_plus_value = params.P_treble_boost_plus_value;
    let P_treble_boost_plus_R = P_treble_boost_plus_value;
    let P_treble_boost_plus_G = 1.0 / P_treble_boost_plus_R;
    // Computing impedance for: P_treble_boost_minus;
    let P_treble_boost_minus_value = params.P_treble_boost_minus_value;
    let P_treble_boost_minus_R = P_treble_boost_minus_value;
    let P_treble_boost_minus_G = 1.0 / P_treble_boost_minus_R;
    // Computing impedance for: R_treble_boost_bw;
    let R_treble_boost_bw_value = params.R_treble_boost_bw_value;
    let R_treble_boost_bw_R = R_treble_boost_bw_value;
    let R_treble_boost_bw_G = 1.0 / R_treble_boost_bw_R;
    // Computing impedance for: C_treble_boost;
    let C_treble_boost_value = params.C_treble_boost_value;
    let C_treble_boost_G = 2.0 * C_treble_boost_value * fs;
    let C_treble_boost_R = 1.0 / C_treble_boost_G;
    // Computing impedance for: L_treble_boost;
    let L_treble_boost_R_calc : f32 = custom_nonlinear_inductor::update_vars(params.L_treble_boost_params, fs);
    let L_treble_boost_R = L_treble_boost_R_calc;
    let L_treble_boost_G = 1.0 / L_treble_boost_R;
    // Computing impedance for: Rlc;
    let Rlc_value = params.Rlc_value;
    let Rlc_R = Rlc_value;
    let Rlc_G = 1.0 / Rlc_R;
    // Computing impedance for: Pb;
    let Pb_res_value = params.P_low_cut_value;
    let Pb_cap_value = params.C_low_cut_value;
    let Pb_2RC = 2.0 * Pb_cap_value * Pb_res_value;
    let Pb_R = Pb_res_value * T / (Pb_2RC + T);
    let Pb_G = 1.0 / Pb_R;
    impedances.Pb_2RC_over_2RC_plus_T = Pb_2RC / (Pb_2RC + T);

    // Computing impedance for: P_treble_cut_plus;
    let P_treble_cut_plus_value = params.P_treble_cut_plus_value;
    let P_treble_cut_plus_R = P_treble_cut_plus_value;
    let P_treble_cut_plus_G = 1.0 / P_treble_cut_plus_R;
    // Computing impedance for: P_treble_cut_minus;
    let P_treble_cut_minus_value = params.P_treble_cut_minus_value;
    let P_treble_cut_minus_R = P_treble_cut_minus_value;
    let P_treble_cut_minus_G = 1.0 / P_treble_cut_minus_R;
    // Computing impedance for: Streble_cut;
    let Streble_cut_res_value = params.Rc_value;
    let Streble_cut_cap_value = params.C_treble_cut_value;
    let Streble_cut_R = (T / (2.0 * Streble_cut_cap_value)) + Streble_cut_res_value;
    let Streble_cut_G = 1.0 / Streble_cut_R;
    impedances.Streble_cut_2RC_over_T_plus_2RC = (2.0 * Streble_cut_cap_value * Streble_cut_res_value) / (2.0 * Streble_cut_cap_value * Streble_cut_res_value + T);

    // Computing impedance for: Pbass_boost;
    let Pbass_boost_res_value = params.P_low_boost_value;
    let Pbass_boost_cap_value = params.C_low_boost_value;
    let Pbass_boost_2RC = 2.0 * Pbass_boost_cap_value * Pbass_boost_res_value;
    let Pbass_boost_R = Pbass_boost_res_value * T / (Pbass_boost_2RC + T);
    let Pbass_boost_G = 1.0 / Pbass_boost_R;
    impedances.Pbass_boost_2RC_over_2RC_plus_T = Pbass_boost_2RC / (Pbass_boost_2RC + T);

    // Computing impedance for: Rb;
    let Rb_value = params.Rb_value;
    let Rb_R = Rb_value;
    let Rb_G = 1.0 / Rb_R;
    // Computing impedance for: Rload;
    let Rload_value = params.Rload_value;
    let Rload_R = Rload_value;
    let Rload_G = 1.0 / Rload_R;
    // Computing impedance for: Streble_boost_1;
    let Streble_boost_1_R = C_treble_boost_R + L_treble_boost_R;
    let Streble_boost_1_G = 1.0 / Streble_boost_1_R;
    impedances.Streble_boost_1_pr = C_treble_boost_R * Streble_boost_1_G;

    // Computing impedance for: Streble_boost;
    let Streble_boost_R = R_treble_boost_bw_R + Streble_boost_1_R;
    let Streble_boost_G = 1.0 / Streble_boost_R;
    impedances.Streble_boost_pr = R_treble_boost_bw_R * Streble_boost_G;

    // Computing impedance for: Sb;
    let Sb_R = Rlc_R + Pb_R;
    let Sb_G = 1.0 / Sb_R;
    impedances.Sb_pr = Rlc_R * Sb_G;

    // Computing impedance for: Ptreble_boost;
    let Ptreble_boost_G = P_treble_boost_minus_G + Streble_boost_G;
    let Ptreble_boost_R = 1.0 / Ptreble_boost_G;
    impedances.Ptreble_boost_pr = P_treble_boost_minus_G * Ptreble_boost_R;

    // Computing impedance for: Ptreble_cut;
    let Ptreble_cut_G = P_treble_cut_minus_G + Streble_cut_G;
    let Ptreble_cut_R = 1.0 / Ptreble_cut_G;
    impedances.Ptreble_cut_pr = P_treble_cut_minus_G * Ptreble_cut_R;

    // Computing impedance for: R;
    let R_R_calc = custom_analog_eq_rtype::update_vars(&mut impedances.R_vars, params.R_params, Sb_R, Sb_G, P_treble_cut_plus_R, P_treble_cut_plus_G, Ptreble_cut_R, Ptreble_cut_G, Pbass_boost_R, Pbass_boost_G, Rb_R, Rb_G, Rload_R, Rload_G);
    let R_R = R_R_calc;
    let R_G = 1.0 / R_R;
    // Computing impedance for: Svin;
    let Svin_R = P_treble_boost_plus_R + Ptreble_boost_R;
    let Svin_G = 1.0 / Svin_R;
    impedances.Svin_pr = P_treble_boost_plus_R * Svin_G;

    // Computing impedance for: SR;
    let SR_R = Svin_R + R_R;
    let SR_G = 1.0 / SR_R;
    impedances.SR_pr = Svin_R * SR_G;

    // Computing impedance for: Pvin;
    let Pvin_G = Ra_G + SR_G;
    let Pvin_R = 1.0 / Pvin_G;
    impedances.Pvin_pr = Ra_G * Pvin_R;

}

#[allow(unused)]
pub fn process(state: &mut State, impedances: &Impedances, Vin: f32) -> f32 {
    let Rload_b : f32 = 0 as f32; // Rload reflected
    let Rb_b : f32 = 0 as f32; // Rb reflected
    let Pbass_boost_b : f32 = impedances.Pbass_boost_2RC_over_2RC_plus_T * state.Pbass_boost_z; // Pbass_boost reflected
    let Streble_cut_b : f32 = -state.Streble_cut_z; // Streble_cut reflected
    let P_treble_cut_minus_b : f32 = 0 as f32; // P_treble_cut_minus reflected
    let Ptreble_cut_b : f32 = Streble_cut_b - impedances.Ptreble_cut_pr * (Streble_cut_b - P_treble_cut_minus_b); // Ptreble_cut reflected
    let P_treble_cut_plus_b : f32 = 0 as f32; // P_treble_cut_plus reflected
    let Pb_b : f32 = impedances.Pb_2RC_over_2RC_plus_T * state.Pb_z; // Pb reflected
    let Rlc_b : f32 = 0 as f32; // Rlc reflected
    let Sb_b : f32 = -(Rlc_b + Pb_b); // Sb reflected
    let R_ins : [f32; 6] = [Sb_b, P_treble_cut_plus_b, Ptreble_cut_b, Pbass_boost_b, Rb_b, Rload_b]; // R inputs
    let R_b = custom_analog_eq_rtype::reflected(&impedances.R_vars, &mut state.R_state, &R_ins); // R reflected
    let L_treble_boost_b =  custom_nonlinear_inductor::reflected(&mut state.L_treble_boost_state); // L_treble_boost reflected
    let C_treble_boost_b : f32 = state.C_treble_boost_z; // C_treble_boost reflected
    let Streble_boost_1_b : f32 = -(C_treble_boost_b + L_treble_boost_b); // Streble_boost_1 reflected
    let R_treble_boost_bw_b : f32 = 0 as f32; // R_treble_boost_bw reflected
    let Streble_boost_b : f32 = -(R_treble_boost_bw_b + Streble_boost_1_b); // Streble_boost reflected
    let P_treble_boost_minus_b : f32 = 0 as f32; // P_treble_boost_minus reflected
    let Ptreble_boost_b : f32 = Streble_boost_b - impedances.Ptreble_boost_pr * (Streble_boost_b - P_treble_boost_minus_b); // Ptreble_boost reflected
    let P_treble_boost_plus_b : f32 = 0 as f32; // P_treble_boost_plus reflected
    let Svin_b : f32 = -(P_treble_boost_plus_b + Ptreble_boost_b); // Svin reflected
    let SR_b : f32 = -(Svin_b + R_b); // SR reflected
    let Ra_b : f32 = 0 as f32; // Ra reflected
    let Pvin_b : f32 = SR_b - impedances.Pvin_pr * (SR_b - Ra_b); // Pvin reflected
    let Vin_a : f32 = Pvin_b; // Vin incident
    let Vin_b : f32 = -Vin_a + 2.0 * Vin; // Vin reflected
    let Pvin_a : f32 = Vin_b; // Pvin incident
    let Ra_a : f32 = Pvin_b - Ra_b + Pvin_a; // Ra incident
    let SR_a : f32 = Pvin_b - SR_b + Pvin_a; // SR incident
    let Svin_a : f32 = Svin_b - impedances.SR_pr * (SR_a - SR_b); // Svin incident
    let R_a : f32 = -SR_a - Svin_b + impedances.SR_pr * (SR_a - SR_b); // R incident
    let P_treble_boost_plus_a : f32 = P_treble_boost_plus_b - impedances.Svin_pr * (Svin_a - Svin_b); // P_treble_boost_plus incident
    let Ptreble_boost_a : f32 = -Svin_a - P_treble_boost_plus_b + impedances.Svin_pr * (Svin_a - Svin_b); // Ptreble_boost incident
    let P_treble_boost_minus_a : f32 = Ptreble_boost_b - P_treble_boost_minus_b + Ptreble_boost_a; // P_treble_boost_minus incident
    let Streble_boost_a : f32 = Ptreble_boost_b - Streble_boost_b + Ptreble_boost_a; // Streble_boost incident
    let R_treble_boost_bw_a : f32 = R_treble_boost_bw_b - impedances.Streble_boost_pr * (Streble_boost_a - Streble_boost_b); // R_treble_boost_bw incident
    let Streble_boost_1_a : f32 = -Streble_boost_a - R_treble_boost_bw_b + impedances.Streble_boost_pr * (Streble_boost_a - Streble_boost_b); // Streble_boost_1 incident
    let C_treble_boost_a : f32 = C_treble_boost_b - impedances.Streble_boost_1_pr * (Streble_boost_1_a - Streble_boost_1_b); // C_treble_boost incident
    let L_treble_boost_a : f32 = -Streble_boost_1_a - C_treble_boost_b + impedances.Streble_boost_1_pr * (Streble_boost_1_a - Streble_boost_1_b); // L_treble_boost incident
    state.C_treble_boost_z = C_treble_boost_a; // C_treble_boost state update
    custom_nonlinear_inductor::incident(&mut state.L_treble_boost_state, L_treble_boost_a); // L_treble_boost incident
    let mut R_outs : [f32; 6] = [0 as f32; 6]; // 6 outputs 
    custom_analog_eq_rtype::incident(&impedances.R_vars, &mut state.R_state, R_a, &R_ins, &mut R_outs); // R compute
    let Sb_a = R_outs[0];
    let P_treble_cut_plus_a = R_outs[1];
    let Ptreble_cut_a = R_outs[2];
    let Pbass_boost_a = R_outs[3];
    let Rb_a = R_outs[4];
    let Rload_a = R_outs[5];
    let Rlc_a : f32 = Rlc_b - impedances.Sb_pr * (Sb_a - Sb_b); // Rlc incident
    let Pb_a : f32 = -Sb_a - Rlc_b + impedances.Sb_pr * (Sb_a - Sb_b); // Pb incident
    state.Pb_z = Pb_b + Pb_a - state.Pb_z; // Pb state update
    let P_treble_cut_minus_a : f32 = Ptreble_cut_b - P_treble_cut_minus_b + Ptreble_cut_a; // P_treble_cut_minus incident
    let Streble_cut_a : f32 = Ptreble_cut_b - Streble_cut_b + Ptreble_cut_a; // Streble_cut incident
    state.Streble_cut_z = -Streble_cut_a + impedances.Streble_cut_2RC_over_T_plus_2RC * (Streble_cut_a + state.Streble_cut_z); // Streble_cut state update
    state.Pbass_boost_z = Pbass_boost_b + Pbass_boost_a - state.Pbass_boost_z; // Pbass_boost state update
    
    let v_Rload : f32 = (Rload_a + Rload_b) * (0.5 as f32); // Rload voltage
    return v_Rload;
}

