// Auto-generated with wdf_compiler version 1.0.0.c475d34.
// Command: wdf_compiler reductions_circuit.wdf reductions_circuit.rs -lang rust

#[derive(Default,Copy,Clone)]
pub struct Params {
    pub Rv1_value : f32, // = 1.0e+04
    pub Rv2_value : f32, // = 5.0e+03
    pub Rl_value : f32, // = 1.0e+05
    pub R7_value : f32, // = 1.0e+04
    pub C7_value : f32, // = 1.0e-05
    pub R8_value : f32, // = 1.0e+05
    pub C8_value : f32, // = 9.999999e-08
    pub R1_value : f32, // = 1.0e+03
    pub R2_value : f32,
    pub R3_value : f32, // = 4.0e+03
    pub R4_value : f32,
    pub R5_value : f32, // = 3.0e+03
    pub C1_value : f32, // = 1.0e-06
    pub C2_value : f32,
    pub C3_value : f32, // = 4.0e-09
    pub C4_value : f32,
    pub C5_value : f32, // = 3.0e-06
    pub R6_value : f32, // = 1.0e+04
    pub C6_value : f32, // = 4.0e-06
}

#[derive(Default,Copy,Clone)]
pub struct Impedances {
    pub Vin_ca : f32,
    pub Vin_ce : f32,
    pub P7_2RC_over_2RC_plus_T : f32,
    pub S7_2RC_over_T_plus_2RC : f32,
    pub S8_pr : f32,
    pub Sl_pr : f32,
}

#[derive(Default,Copy,Clone)]
pub struct State {
    pub P7_z : f32,
    pub S7_z : f32,
}

#[allow(unused)]
pub fn calc_impedances(impedances: &mut Impedances, fs: f32, params: Params) {
    fn sum(a: f32, b: f32) -> f32 { a + b }
    fn recip_sum(a: f32, b: f32) -> f32 { (a * b) / (a + b) }
    
    let T = 1.0 / fs;
    
    // Computing impedance for: Rl;
    let Rl_value = params.Rl_value;
    let Rl_R = Rl_value;
    let Rl_G = 1.0 / Rl_R;
    // Computing impedance for: P7;
    let P7_res_value = recip_sum(params.R7_value, params.R8_value);
    let P7_cap_value = sum(params.C7_value, params.C8_value);
    let P7_2RC = 2.0 * P7_cap_value * P7_res_value;
    let P7_R = P7_res_value * T / (P7_2RC + T);
    let P7_G = 1.0 / P7_R;
    impedances.P7_2RC_over_2RC_plus_T = P7_2RC / (P7_2RC + T);

    // Computing impedance for: S7;
    let S7_res_value = sum(recip_sum(sum(sum(params.R1_value, params.R2_value), params.R3_value), recip_sum(params.R4_value, params.R5_value)), params.R6_value);
    let S7_cap_value = recip_sum(sum(recip_sum(recip_sum(params.C1_value, params.C2_value), params.C3_value), sum(params.C4_value, params.C5_value)), params.C6_value);
    let S7_R = (T / (2.0 * S7_cap_value)) + S7_res_value;
    let S7_G = 1.0 / S7_R;
    impedances.S7_2RC_over_T_plus_2RC = (2.0 * S7_cap_value * S7_res_value) / (2.0 * S7_cap_value * S7_res_value + T);

    // Computing impedance for: S8;
    let S8_R = P7_R + S7_R;
    let S8_G = 1.0 / S8_R;
    impedances.S8_pr = P7_R * S8_G;

    // Computing impedance for: Sl;
    let Sl_R = Rl_R + S8_R;
    let Sl_G = 1.0 / Sl_R;
    impedances.Sl_pr = Rl_R * Sl_G;

    
    // Computing impedance for: Vin;
    let Vin_res_value = sum(params.Rv1_value, params.Rv2_value);
    impedances.Vin_ca = (Vin_res_value - Sl_R) / (Vin_res_value + Sl_R);
    impedances.Vin_ce = (2.0 * Sl_R) / (Vin_res_value + Sl_R);

}

#[allow(unused)]
pub fn process(state: &mut State, impedances: &Impedances, Vin: f32) -> f32 {
    let S7_b : f32 = -state.S7_z; // S7 reflected
    let P7_b : f32 = impedances.P7_2RC_over_2RC_plus_T * state.P7_z; // P7 reflected
    let S8_b : f32 = -(P7_b + S7_b); // S8 reflected
    let Rl_b : f32 = 0 as f32; // Rl reflected
    let Sl_b : f32 = -(Rl_b + S8_b); // Sl reflected
    let Vin_a : f32 = Sl_b; // Vin incident
    let Vin_b : f32 = impedances.Vin_ca * Vin_a + impedances.Vin_ce * Vin; // Vin reflected
    let Sl_a : f32 = Vin_b; // Sl incident
    let Rl_a : f32 = Rl_b - impedances.Sl_pr * (Sl_a - Sl_b); // Rl incident
    let S8_a : f32 = -Sl_a - Rl_b + impedances.Sl_pr * (Sl_a - Sl_b); // S8 incident
    let P7_a : f32 = P7_b - impedances.S8_pr * (S8_a - S8_b); // P7 incident
    let S7_a : f32 = -S8_a - P7_b + impedances.S8_pr * (S8_a - S8_b); // S7 incident
    state.P7_z = P7_b + P7_a - state.P7_z; // P7 state update
    state.S7_z = -S7_a + impedances.S7_2RC_over_T_plus_2RC * (S7_a + state.S7_z); // S7 state update
    
    let v_Rl : f32 = (Rl_a + Rl_b) * (0.5 as f32); // Rl voltage
    return v_Rl;
}

