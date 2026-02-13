// Auto-generated with wdf_compiler version 1.0.0.64f18cd.
// Command: wdf_compiler baxandall_eq.wdf baxandall_eq.rs -lang rust

use crate::custom_baxandall_rtype;

#[derive(Default,Copy,Clone)]
pub struct Params {
    pub Ca_value : f32, // = 1.0e-06
    pub S4_res_value : f32,
    pub S4_cap_value : f32, // = 6.4e-09
    pub Rl_value : f32, // = 1.0e+06
    pub S5_res_value : f32,
    pub S5_cap_value : f32, // = 6.4e-08
    pub Resc_value : f32, // = 1.0e+04
    pub Resb_value : f32, // = 1.0e+03
    pub P3_res_value : f32,
    pub P3_cap_value : f32, // = 2.2e-07
    pub Resa_value : f32, // = 1.0e+04
    pub P2_res_value : f32,
    pub P2_cap_value : f32, // = 2.2e-08
    pub R_params: custom_baxandall_rtype::R_Params,
}

#[derive(Default,Copy,Clone)]
pub struct Impedances {
    pub Vin_ca : f32,
    pub Vin_ce : f32,
    pub S4_2RC_over_T_plus_2RC : f32,
    pub S5_2RC_over_T_plus_2RC : f32,
    pub P3_2RC_over_2RC_plus_T : f32,
    pub P2_2RC_over_2RC_plus_T : f32,
    pub S3_pr : f32,
    pub S2_pr : f32,
    pub P1_pr : f32,
    pub R_vars: custom_baxandall_rtype::R_Vars,
}

#[derive(Default,Copy,Clone)]
pub struct State {
    pub Vin_z : f32,
    pub S4_z : f32,
    pub S5_z : f32,
    pub P3_z : f32,
    pub P2_z : f32,
    pub R_state: custom_baxandall_rtype::R_State,
}

#[allow(unused)]
pub fn calc_impedances(impedances: &mut Impedances, fs: f32, params: Params) {
    fn sum(a: f32, b: f32) -> f32 { a + b }
    fn recip_sum(a: f32, b: f32) -> f32 { (a * b) / (a + b) }
    
    let T = 1.0 / fs;
    
    // Computing impedance for: S4;
    let S4_res_value = params.S4_res_value;
    let S4_cap_value = params.S4_cap_value;
    let S4_R = (T / (2.0 * S4_cap_value)) + S4_res_value;
    let S4_G = 1.0 / S4_R;
    impedances.S4_2RC_over_T_plus_2RC = (2.0 * S4_cap_value * S4_res_value) / (2.0 * S4_cap_value * S4_res_value + T);

    // Computing impedance for: Rl;
    let Rl_value = params.Rl_value;
    let Rl_R = Rl_value;
    let Rl_G = 1.0 / Rl_R;
    // Computing impedance for: S5;
    let S5_res_value = params.S5_res_value;
    let S5_cap_value = params.S5_cap_value;
    let S5_R = (T / (2.0 * S5_cap_value)) + S5_res_value;
    let S5_G = 1.0 / S5_R;
    impedances.S5_2RC_over_T_plus_2RC = (2.0 * S5_cap_value * S5_res_value) / (2.0 * S5_cap_value * S5_res_value + T);

    // Computing impedance for: Resc;
    let Resc_value = params.Resc_value;
    let Resc_R = Resc_value;
    let Resc_G = 1.0 / Resc_R;
    // Computing impedance for: Resb;
    let Resb_value = params.Resb_value;
    let Resb_R = Resb_value;
    let Resb_G = 1.0 / Resb_R;
    // Computing impedance for: P3;
    let P3_res_value = params.P3_res_value;
    let P3_cap_value = params.P3_cap_value;
    let P3_2RC = 2.0 * P3_cap_value * P3_res_value;
    let P3_R = P3_res_value * T / (P3_2RC + T);
    let P3_G = 1.0 / P3_R;
    impedances.P3_2RC_over_2RC_plus_T = P3_2RC / (P3_2RC + T);

    // Computing impedance for: Resa;
    let Resa_value = params.Resa_value;
    let Resa_R = Resa_value;
    let Resa_G = 1.0 / Resa_R;
    // Computing impedance for: P2;
    let P2_res_value = params.P2_res_value;
    let P2_cap_value = params.P2_cap_value;
    let P2_2RC = 2.0 * P2_cap_value * P2_res_value;
    let P2_R = P2_res_value * T / (P2_2RC + T);
    let P2_G = 1.0 / P2_R;
    impedances.P2_2RC_over_2RC_plus_T = P2_2RC / (P2_2RC + T);

    // Computing impedance for: S3;
    let S3_R = Resb_R + P3_R;
    let S3_G = 1.0 / S3_R;
    impedances.S3_pr = Resb_R * S3_G;

    // Computing impedance for: S2;
    let S2_R = Resa_R + P2_R;
    let S2_G = 1.0 / S2_R;
    impedances.S2_pr = Resa_R * S2_G;

    // Computing impedance for: P1;
    let P1_G = Rl_G + S5_G;
    let P1_R = 1.0 / P1_G;
    impedances.P1_pr = Rl_G * P1_R;

    // Computing impedance for: R;
    let R_R_calc = custom_baxandall_rtype::update_vars(&mut impedances.R_vars, params.R_params, S4_R, S4_G, P1_R, P1_G, Resc_R, Resc_G, S3_R, S3_G, S2_R, S2_G);
    let R_R = R_R_calc;
    let R_G = 1.0 / R_R;
    
    // Computing impedance for: Vin;
    let Vin_cap_value = params.Ca_value;
    impedances.Vin_ca = T / (T + 2.0 * R_R * Vin_cap_value);
    impedances.Vin_ce = (2.0 * R_R * Vin_cap_value) / (T + 2.0 * R_R * Vin_cap_value);

}

#[allow(unused)]
pub fn process(state: &mut State, impedances: &Impedances, Vin: f32) -> f32 {
    let P2_b : f32 = impedances.P2_2RC_over_2RC_plus_T * state.P2_z; // P2 reflected
    let Resa_b : f32 = 0 as f32; // Resa reflected
    let S2_b : f32 = -(Resa_b + P2_b); // S2 reflected
    let P3_b : f32 = impedances.P3_2RC_over_2RC_plus_T * state.P3_z; // P3 reflected
    let Resb_b : f32 = 0 as f32; // Resb reflected
    let S3_b : f32 = -(Resb_b + P3_b); // S3 reflected
    let Resc_b : f32 = 0 as f32; // Resc reflected
    let S5_b : f32 = -state.S5_z; // S5 reflected
    let Rl_b : f32 = 0 as f32; // Rl reflected
    let P1_b : f32 = S5_b - impedances.P1_pr * (S5_b - Rl_b); // P1 reflected
    let S4_b : f32 = -state.S4_z; // S4 reflected
    let R_ins : [f32; 5] = [S4_b, P1_b, Resc_b, S3_b, S2_b]; // R inputs
    let R_b = custom_baxandall_rtype::reflected(&impedances.R_vars, &mut state.R_state, &R_ins); // R reflected
    let Vin_a : f32 = R_b; // Vin incident
    let Vin_a0a1 : f32 = 2.0 * (Vin + state.Vin_z) + Vin_a; // Vin reflected
    let Vin_b : f32 = impedances.Vin_ca * Vin_a - impedances.Vin_ce * Vin_a0a1; // Vin reflected
    state.Vin_z += -Vin_b - Vin_a0a1; // Vin_z state update
    let R_a : f32 = Vin_b; // R incident
    let mut R_outs : [f32; 5] = [0 as f32; 5]; // 5 outputs 
    custom_baxandall_rtype::incident(&impedances.R_vars, &mut state.R_state, R_a, &R_ins, &mut R_outs); // R compute
    let S4_a = R_outs[0];
    let P1_a = R_outs[1];
    let Resc_a = R_outs[2];
    let S3_a = R_outs[3];
    let S2_a = R_outs[4];
    state.S4_z = -S4_a + impedances.S4_2RC_over_T_plus_2RC * (S4_a + state.S4_z); // S4 state update
    let Rl_a : f32 = P1_b - Rl_b + P1_a; // Rl incident
    let S5_a : f32 = P1_b - S5_b + P1_a; // S5 incident
    state.S5_z = -S5_a + impedances.S5_2RC_over_T_plus_2RC * (S5_a + state.S5_z); // S5 state update
    let Resb_a : f32 = Resb_b - impedances.S3_pr * (S3_a - S3_b); // Resb incident
    let P3_a : f32 = -S3_a - Resb_b + impedances.S3_pr * (S3_a - S3_b); // P3 incident
    state.P3_z = P3_b + P3_a - state.P3_z; // P3 state update
    let Resa_a : f32 = Resa_b - impedances.S2_pr * (S2_a - S2_b); // Resa incident
    let P2_a : f32 = -S2_a - Resa_b + impedances.S2_pr * (S2_a - S2_b); // P2 incident
    state.P2_z = P2_b + P2_a - state.P2_z; // P2 state update
    
    let v_Rl : f32 = (Rl_a + Rl_b) * (0.5 as f32); // Rl voltage
    return v_Rl;
}

