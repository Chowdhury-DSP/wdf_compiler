// Auto-generated with wdf_compiler version 1.0.0.c475d34.
// Command: wdf_compiler rl_lowpass_var.wdf rl_lowpass_var.rs -lang rust

#[derive(Default,Copy,Clone)]
pub struct Params {
    pub Iplus_i_value : f32,
    pub Iin_res_value : f32, // = 1.0e+03
    pub Ivar_i_value : f32,
    pub Ivar_res_value : f32, // = 2.0e+03
    pub R1_value : f32,
    pub L1_value : f32, // = 1.0e-03
}

#[derive(Default,Copy,Clone)]
pub struct Impedances {
    pub Iplus_I : f32,
    pub P1_R : f32,
    pub Iin_R : f32,
    pub Ivar_R : f32,
    pub Ivar_I : f32,
    pub R1_G : f32,
    pub P3_pr : f32,
    pub P2_pr : f32,
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
    
    // Computing impedance for: Iin;
    let Iin_R = params.Iin_res_value;
    impedances.Iin_R = Iin_R;
    let Iin_G = 1.0 / Iin_R;
    // Computing current for: Ivar;
    impedances.Ivar_I = params.Ivar_i_value;

    // Computing impedance for: Ivar;
    let Ivar_R = params.Ivar_res_value;
    impedances.Ivar_R = Ivar_R;
    let Ivar_G = 1.0 / Ivar_R;
    // Computing impedance for: R1;
    let R1_value = params.R1_value;
    let R1_R = R1_value;
    let R1_G = 1.0 / R1_R;
    impedances.R1_G = R1_G;
    // Computing impedance for: L1;
    let L1_value = params.L1_value;
    let L1_R = 2.0 * L1_value * fs;
    let L1_G = 1.0 / L1_R;
    // Computing impedance for: P3;
    let P3_G = R1_G + L1_G;
    let P3_R = 1.0 / P3_G;
    impedances.P3_pr = R1_G * P3_R;

    // Computing impedance for: P2;
    let P2_G = Ivar_G + P3_G;
    let P2_R = 1.0 / P2_G;
    impedances.P2_pr = Ivar_G * P2_R;

    // Computing impedance for: P1;
    let P1_G = Iin_G + P2_G;
    let P1_R = 1.0 / P1_G;
    impedances.P1_R = P1_R;
    impedances.P1_pr = Iin_G * P1_R;

    
    // Computing current for: Iplus;
    impedances.Iplus_I = params.Iplus_i_value;

}

#[allow(unused)]
pub fn process(state: &mut State, impedances: &Impedances, Iin: f32) -> f32 {
    let L1_b : f32 = -state.L1_z; // L1 reflected
    let R1_b : f32 = 0 as f32; // R1 reflected
    let P3_b : f32 = L1_b - impedances.P3_pr * (L1_b - R1_b); // P3 reflected
    let Ivar_b : f32 = impedances.Ivar_R * impedances.Ivar_I; // Ivar reflected
    let P2_b : f32 = P3_b - impedances.P2_pr * (P3_b - Ivar_b); // P2 reflected
    let Iin_b : f32 = impedances.Iin_R * Iin; // Iin reflected
    let P1_b : f32 = P2_b - impedances.P1_pr * (P2_b - Iin_b); // P1 reflected
    let Iplus_a : f32 = P1_b; // Iplus incident
    let Iplus_b : f32 = 2.0 * impedances.P1_R * impedances.Iplus_I + Iplus_a; // Iplus reflected (@TODO: 2.0 * R?)
    let P1_a : f32 = Iplus_b; // P1 incident
    let Iin_a : f32 = P1_b - Iin_b + P1_a; // Iin incident
    let P2_a : f32 = P1_b - P2_b + P1_a; // P2 incident
    let Ivar_a : f32 = P2_b - Ivar_b + P2_a; // Ivar incident
    let P3_a : f32 = P2_b - P3_b + P2_a; // P3 incident
    let R1_a : f32 = P3_b - R1_b + P3_a; // R1 incident
    let L1_a : f32 = P3_b - L1_b + P3_a; // L1 incident
    state.L1_z = L1_a; // L1 state update
    
    let i_R1 : f32 = (R1_a - R1_b) * ((0.5 as f32) * impedances.R1_G); // R1 current
    return i_R1;
}

