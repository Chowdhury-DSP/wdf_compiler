// Auto-generated with wdf_compiler version 1.0.0.c475d34.
// Command: wdf_compiler rc_lowpass_2ins.wdf rc_lowpass_2ins.rs -lang rust

#[derive(Default,Copy,Clone)]
pub struct Params {
    pub R1_res_value : f32, // = 1.0e+03
    pub C1_value : f32, // = 1.0e-06
}

#[derive(Default,Copy,Clone)]
pub struct Impedances {
    pub S1_pr : f32,
}

#[derive(Default,Copy,Clone)]
pub struct State {
    pub C1_z : f32,
}

#[allow(unused)]
pub fn calc_impedances(impedances: &mut Impedances, fs: f32, params: Params) {
    fn sum(a: f32, b: f32) -> f32 { a + b }
    fn recip_sum(a: f32, b: f32) -> f32 { (a * b) / (a + b) }
    
    let T = 1.0 / fs;
    
    // Computing impedance for: R1;
    let R1_res_value = params.R1_res_value;
    let R1_R = R1_res_value;
    let R1_G = 1.0 / R1_R;
    // Computing impedance for: C1;
    let C1_value = params.C1_value;
    let C1_G = 2.0 * C1_value * fs;
    let C1_R = 1.0 / C1_G;
    // Computing impedance for: S1;
    let S1_R = R1_R + C1_R;
    let S1_G = 1.0 / S1_R;
    impedances.S1_pr = R1_R * S1_G;

}

#[allow(unused)]
pub fn process(state: &mut State, impedances: &Impedances, Vin: f32, R1: f32) -> f32 {
    let C1_b : f32 = state.C1_z; // C1 reflected
    let R1_b : f32 = R1; // R1 reflected
    let S1_b : f32 = -(R1_b + C1_b); // S1 reflected
    let Vin_a : f32 = S1_b; // Vin incident
    let Vin_b : f32 = -Vin_a + 2.0 * Vin; // Vin reflected
    let S1_a : f32 = Vin_b; // S1 incident
    let R1_a : f32 = R1_b - impedances.S1_pr * (S1_a - S1_b); // R1 incident
    let C1_a : f32 = -S1_a - R1_b + impedances.S1_pr * (S1_a - S1_b); // C1 incident
    state.C1_z = C1_a; // C1 state update
    
    let v_C1 : f32 = (C1_a + C1_b) * (0.5 as f32); // C1 voltage
    return v_C1;
}

