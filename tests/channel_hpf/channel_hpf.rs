// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler channel_hpf.wdf channel_hpf.rs -lang rust

#[derive(Default,Copy,Clone)]
pub struct Params {
    pub Sw1_closed : f32,
    pub R8_value : f32, // = 1.0e+04
    pub Lh_value : f32, // = 3.0e+00
    pub Ll_value : f32, // = 7.0e+00
    pub C17_Vin_cap_value : f32, // = 4.7e-07
    pub C18_value : f32, // = 4.7e-07
    pub R9_value : f32, // = 5.1e+03
}

#[derive(Default,Copy,Clone)]
pub struct Impedances {
    pub Sw1_closed : f32,
    pub S2_pr : f32,
    pub P3_pr : f32,
    pub P2_pr : f32,
    pub S1_pr : f32,
    pub P1_pr : f32,
}

#[derive(Default,Copy,Clone)]
pub struct State {
    pub Lh_z : f32,
    pub Ll_z : f32,
    pub C17_Vin_z : f32,
    pub C17_Vin_v1 : f32,
    pub C18_z : f32,
}

#[allow(unused)]
pub fn calc_impedances(impedances: &mut Impedances, fs: f32, params: Params) {
    fn sum(a: f32, b: f32) -> f32 { a + b }
    fn recip_sum(a: f32, b: f32) -> f32 { (a * b) / (a + b) }
    
    let T = 1.0 / fs;
    
    // Computing impedance for: R8;
    let R8_value = params.R8_value;
    let R8_R = R8_value;
    let R8_G = 1.0 / R8_R;
    // Computing impedance for: Lh;
    let Lh_value = params.Lh_value;
    let Lh_R = 2.0 * Lh_value * fs;
    let Lh_G = 1.0 / Lh_R;
    // Computing impedance for: Ll;
    let Ll_value = params.Ll_value;
    let Ll_R = 2.0 * Ll_value * fs;
    let Ll_G = 1.0 / Ll_R;
    // Computing impedance for: C17_Vin;
    let C17_Vin_cap_value = params.C17_Vin_cap_value;
    let C17_Vin_G = 2.0 * C17_Vin_cap_value * fs;
    let C17_Vin_R = 1.0 / C17_Vin_G;
    // Computing impedance for: C18;
    let C18_value = params.C18_value;
    let C18_G = 2.0 * C18_value * fs;
    let C18_R = 1.0 / C18_G;
    // Computing impedance for: R9;
    let R9_value = params.R9_value;
    let R9_R = R9_value;
    let R9_G = 1.0 / R9_R;
    // Computing impedance for: S2;
    let S2_R = C18_R + R9_R;
    let S2_G = 1.0 / S2_R;
    impedances.S2_pr = C18_R * S2_G;

    // Computing impedance for: P3;
    let P3_G = C17_Vin_G + S2_G;
    let P3_R = 1.0 / P3_G;
    impedances.P3_pr = C17_Vin_G * P3_R;

    // Computing impedance for: P2;
    let P2_G = R8_G + Lh_G;
    let P2_R = 1.0 / P2_G;
    impedances.P2_pr = R8_G * P2_R;

    // Computing impedance for: S1;
    let S1_R = Ll_R + P3_R;
    let S1_G = 1.0 / S1_R;
    impedances.S1_pr = Ll_R * S1_G;

    // Computing impedance for: P1;
    let P1_G = P2_G + S1_G;
    let P1_R = 1.0 / P1_G;
    impedances.P1_pr = P2_G * P1_R;

    
    // Computing state for: Sw1;
    impedances.Sw1_closed = params.Sw1_closed;

}

#[allow(unused)]
pub fn process(state: &mut State, impedances: &Impedances, C17_Vin: f32) -> f32 {
    let R9_b : f32 = 0 as f32; // R9 reflected
    let C18_b : f32 = state.C18_z; // C18 reflected
    let S2_b : f32 = -(C18_b + R9_b); // S2 reflected
    let C17_Vin_b : f32 = state.C17_Vin_z + C17_Vin - state.C17_Vin_v1; // C17_Vin reflected
    state.C17_Vin_v1 = C17_Vin; // C17_Vin state update
    let P3_b : f32 = S2_b - impedances.P3_pr * (S2_b - C17_Vin_b); // P3 reflected
    let Ll_b : f32 = -state.Ll_z; // Ll reflected
    let S1_b : f32 = -(Ll_b + P3_b); // S1 reflected
    let Lh_b : f32 = -state.Lh_z; // Lh reflected
    let R8_b : f32 = 0 as f32; // R8 reflected
    let P2_b : f32 = Lh_b - impedances.P2_pr * (Lh_b - R8_b); // P2 reflected
    let P1_b : f32 = S1_b - impedances.P1_pr * (S1_b - P2_b); // P1 reflected
    let Sw1_a : f32 = P1_b; // Sw1 incident
    let Sw1_b : f32 = if impedances.Sw1_closed != (0 as f32) { -Sw1_a } else { Sw1_a }; // Sw1 reflected
    let P1_a : f32 = Sw1_b; // P1 incident
    let P2_a : f32 = P1_b - P2_b + P1_a; // P2 incident
    let S1_a : f32 = P1_b - S1_b + P1_a; // S1 incident
    let R8_a : f32 = P2_b - R8_b + P2_a; // R8 incident
    let Lh_a : f32 = P2_b - Lh_b + P2_a; // Lh incident
    state.Lh_z = Lh_a; // Lh state update
    let Ll_a : f32 = Ll_b - impedances.S1_pr * (S1_a - S1_b); // Ll incident
    let P3_a : f32 = -S1_a - Ll_b + impedances.S1_pr * (S1_a - S1_b); // P3 incident
    state.Ll_z = Ll_a; // Ll state update
    let C17_Vin_a : f32 = P3_b - C17_Vin_b + P3_a; // C17_Vin incident
    let S2_a : f32 = P3_b - S2_b + P3_a; // S2 incident
    state.C17_Vin_z = C17_Vin_a; // C17_Vin state update
    let C18_a : f32 = C18_b - impedances.S2_pr * (S2_a - S2_b); // C18 incident
    let R9_a : f32 = -S2_a - C18_b + impedances.S2_pr * (S2_a - S2_b); // R9 incident
    state.C18_z = C18_a; // C18 state update
    
    let v_R9 : f32 = (R9_a + R9_b) * (0.5 as f32); // R9 voltage
    return v_R9;
}

