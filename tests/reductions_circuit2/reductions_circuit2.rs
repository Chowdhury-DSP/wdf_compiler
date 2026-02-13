// Auto-generated with wdf_compiler version 1.0.0.630eddd.
// Command: wdf_compiler reductions_circuit2.wdf reductions_circuit2.rs -lang rust

#[derive(Default,Copy,Clone)]
pub struct Params {
    pub Vb_v_value : f32, // = 1.5e+00
    pub Rp_value : f32, // = 4.0e+03
    pub S4_v_value : f32, // = 1.2e+00
    pub Vcc_cap_value : f32,
    pub C1_value : f32, // = 1.0e-08
    pub Cp2_value : f32, // = 7.0e-09
    pub Vin_res_value : f32, // = 5.0e+03
    pub R1_value : f32, // = 1.0e+04
    pub R2_value : f32,
    pub Cp_value : f32, // = 4.0e-06
    pub Rp2_value : f32, // = 7.0e+03
    pub Rl_v_value : f32, // = -1.1e+00
    pub Rl_res_value : f32, // = 1.0e+05
    pub Rl_cap_value : f32, // = 1.0e-05
}

#[derive(Default,Copy,Clone)]
pub struct Impedances {
    pub Vb_V : f32, // = Params.Vb_v_value
    pub S4_V : f32, // = Params.S4_v_value
    pub S8_T_over_2RC : f32,
    pub Rl_V : f32, // = Params.Rl_v_value
    pub Rl_T_over_2RC : f32,
    pub S7_pr : f32,
    pub S6_pr : f32,
    pub S3_pr : f32,
    pub Sl_pr : f32,
}

#[derive(Default,Copy,Clone)]
pub struct State {
    pub S4_z : f32,
    pub S4_v1 : f32,
    pub Cp2_z : f32,
    pub S8_z : f32,
    pub Rl_z : f32,
}

#[allow(unused)]
pub fn calc_impedances(impedances: &mut Impedances, fs: f32, params: Params) {
    fn sum(a: f32, b: f32) -> f32 { a + b }
    fn recip_sum(a: f32, b: f32) -> f32 { (a * b) / (a + b) }
    
    let T = 1.0 / fs;
    
    // Computing impedance for: Rp;
    let Rp_value = params.Rp_value;
    let Rp_R = Rp_value;
    let Rp_G = 1.0 / Rp_R;
    // Computing voltage for: S4;
    impedances.S4_V = params.S4_v_value;

    // Computing impedance for: S4;
    let S4_cap_value = recip_sum(params.Vcc_cap_value, params.C1_value);
    let S4_G = 2.0 * S4_cap_value * fs;
    let S4_R = 1.0 / S4_G;
    // Computing impedance for: Cp2;
    let Cp2_value = params.Cp2_value;
    let Cp2_G = 2.0 * Cp2_value * fs;
    let Cp2_R = 1.0 / Cp2_G;
    // Computing impedance for: S8;
    let S8_res_value = sum(sum(sum(params.Vin_res_value, params.R1_value), params.R2_value), params.Rp2_value);
    let S8_cap_value = params.Cp_value;
    let S8_R = (T / (2.0 * S8_cap_value)) + S8_res_value;
    let S8_G = 1.0 / S8_R;
    impedances.S8_T_over_2RC = T / (2.0 * S8_cap_value * S8_res_value);

    // Computing voltage for: Rl;
    impedances.Rl_V = params.Rl_v_value;

    // Computing impedance for: Rl;
    let Rl_res_value = params.Rl_res_value;
    let Rl_cap_value = params.Rl_cap_value;
    let Rl_R = (T / (2.0 * Rl_cap_value)) + Rl_res_value;
    let Rl_G = 1.0 / Rl_R;
    impedances.Rl_T_over_2RC = T / (2.0 * Rl_cap_value * Rl_res_value);

    // Computing impedance for: S7;
    let S7_R = Cp2_R + S8_R;
    let S7_G = 1.0 / S7_R;
    impedances.S7_pr = Cp2_R * S7_G;

    // Computing impedance for: S6;
    let S6_R = Rp_R + S4_R;
    let S6_G = 1.0 / S6_R;
    impedances.S6_pr = Rp_R * S6_G;

    // Computing impedance for: S3;
    let S3_R = S6_R + S7_R;
    let S3_G = 1.0 / S3_R;
    impedances.S3_pr = S6_R * S3_G;

    // Computing impedance for: Sl;
    let Sl_R = S3_R + Rl_R;
    let Sl_G = 1.0 / Sl_R;
    impedances.Sl_pr = S3_R * Sl_G;

    
    // Computing voltage for: Vb;
    impedances.Vb_V = params.Vb_v_value;

}

#[allow(unused)]
pub fn process(state: &mut State, impedances: &Impedances, S8: f32) -> f32 {
    let Rl_b : f32 = -(state.Rl_z + impedances.Rl_V); // Rl reflected
    let S8_b : f32 = -(state.S8_z + -(S8)); // S8 reflected
    let Cp2_b : f32 = state.Cp2_z; // Cp2 reflected
    let S7_b : f32 = -(Cp2_b + S8_b); // S7 reflected
    let S4_b : f32 = state.S4_z + -(impedances.S4_V) - state.S4_v1; // S4 reflected
    state.S4_v1 = -(impedances.S4_V); // S4 state update
    let Rp_b : f32 = 0 as f32; // Rp reflected
    let S6_b : f32 = -(Rp_b + S4_b); // S6 reflected
    let S3_b : f32 = -(S6_b + S7_b); // S3 reflected
    let Sl_b : f32 = -(S3_b + Rl_b); // Sl reflected
    let Vb_a : f32 = Sl_b; // Vb incident
    let Vb_b : f32 = -Vb_a + 2.0 * impedances.Vb_V; // Vb reflected
    let Sl_a : f32 = Vb_b; // Sl incident
    let S3_a : f32 = S3_b - impedances.Sl_pr * (Sl_a - Sl_b); // S3 incident
    let Rl_a : f32 = -Sl_a - S3_b + impedances.Sl_pr * (Sl_a - Sl_b); // Rl incident
    let S6_a : f32 = S6_b - impedances.S3_pr * (S3_a - S3_b); // S6 incident
    let S7_a : f32 = -S3_a - S6_b + impedances.S3_pr * (S3_a - S3_b); // S7 incident
    let Rp_a : f32 = Rp_b - impedances.S6_pr * (S6_a - S6_b); // Rp incident
    let S4_a : f32 = -S6_a - Rp_b + impedances.S6_pr * (S6_a - S6_b); // S4 incident
    state.S4_z = S4_a; // S4 state update
    let Cp2_a : f32 = Cp2_b - impedances.S7_pr * (S7_a - S7_b); // Cp2 incident
    let S8_a : f32 = -S7_a - Cp2_b + impedances.S7_pr * (S7_a - S7_b); // S8 incident
    state.Cp2_z = Cp2_a; // Cp2 state update
    state.S8_z -= impedances.S8_T_over_2RC * (S8_a - S8_b); // S8 state update
    state.Rl_z -= impedances.Rl_T_over_2RC * (Rl_a - Rl_b); // Rl state update
    
    let v_Rl : f32 = (Rl_a + Rl_b) * (0.5 as f32); // Rl voltage
    return v_Rl;
}

