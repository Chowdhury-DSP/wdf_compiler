// Auto-generated with wdf_compiler version 1.0.0.630eddd.
// Command: wdf_compiler rl_lowpass.wdf rl_lowpass.rs -lang rust

#[derive(Default,Copy,Clone)]
pub struct Params {
    pub R1_value : f32, // = 1.0e+03
    pub L1_value : f32, // = 1.0e-03
}

#[derive(Default,Copy,Clone)]
pub struct Impedances {
    pub P1_R : f32,
    pub R1_G : f32,
    pub P1_pr : f32,
}

#[derive(Default,Copy,Clone)]
pub struct State {
    pub L1_z : f32,
}

#[allow(unused)]
pub fn calc_impedances(impedances: &mut Impedances, fs: f32, params: Params) {
    fn sum(a: f32, b: f32) -> f32 { a + b }
    fn recip_sum(a: f32, b: f32) -> f32 { (a * b) / (a + b) }
    
    let T = 1.0 / fs;
    
    // Computing impedance for: R1;
    let R1_value = params.R1_value;
    let R1_R = R1_value;
    let R1_G = 1.0 / R1_R;
    impedances.R1_G = R1_G;
    // Computing impedance for: L1;
    let L1_value = params.L1_value;
    let L1_R = 2.0 * L1_value * fs;
    let L1_G = 1.0 / L1_R;
    // Computing impedance for: P1;
    let P1_G = R1_G + L1_G;
    let P1_R = 1.0 / P1_G;
    impedances.P1_R = P1_R;
    impedances.P1_pr = R1_G * P1_R;

}

#[allow(unused)]
pub fn process(state: &mut State, impedances: &Impedances, Iin: f32) -> f32 {
    let L1_b : f32 = -state.L1_z; // L1 reflected
    let R1_b : f32 = 0 as f32; // R1 reflected
    let P1_b : f32 = L1_b - impedances.P1_pr * (L1_b - R1_b); // P1 reflected
    let Iin_a : f32 = P1_b; // Iin incident
    let Iin_b : f32 = 2.0 * impedances.P1_R * Iin + Iin_a; // Iin reflected (@TODO: 2.0 * R?)
    let P1_a : f32 = Iin_b; // P1 incident
    let R1_a : f32 = P1_b - R1_b + P1_a; // R1 incident
    let L1_a : f32 = P1_b - L1_b + P1_a; // L1 incident
    state.L1_z = L1_a; // L1 state update
    
    let i_R1 : f32 = (R1_a - R1_b) * ((0.5 as f32) * impedances.R1_G); // R1 current
    return i_R1;
}

