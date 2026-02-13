// Auto-generated with wdf_compiler version 1.0.0.630eddd.
// Command: wdf_compiler rc_bandpass.wdf rc_bandpass.rs -lang rust

#[derive(Default,Copy,Clone)]
pub struct Params {
    pub C1_value : f32, // = 1.0e-06
    pub R1_value : f32, // = 1.0e+03
    pub R2_value : f32, // = 1.0e+03
    pub C2_value : f32, // = 1.0e-06
}

#[derive(Default,Copy,Clone)]
pub struct Impedances {
    pub Vin_ca : f32,
    pub Vin_ce : f32,
    pub S2_pr : f32,
    pub P1_pr : f32,
}

#[derive(Default,Copy,Clone)]
pub struct State {
    pub Vin_z : f32,
    pub C2_z : f32,
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
    // Computing impedance for: R2;
    let R2_value = params.R2_value;
    let R2_R = R2_value;
    let R2_G = 1.0 / R2_R;
    // Computing impedance for: C2;
    let C2_value = params.C2_value;
    let C2_G = 2.0 * C2_value * fs;
    let C2_R = 1.0 / C2_G;
    // Computing impedance for: S2;
    let S2_R = R2_R + C2_R;
    let S2_G = 1.0 / S2_R;
    impedances.S2_pr = R2_R * S2_G;

    // Computing impedance for: P1;
    let P1_G = R1_G + S2_G;
    let P1_R = 1.0 / P1_G;
    impedances.P1_pr = R1_G * P1_R;

    
    // Computing impedance for: Vin;
    let Vin_cap_value = params.C1_value;
    impedances.Vin_ca = T / (T + 2.0 * P1_R * Vin_cap_value);
    impedances.Vin_ce = (2.0 * P1_R * Vin_cap_value) / (T + 2.0 * P1_R * Vin_cap_value);

}

#[allow(unused)]
pub fn process(state: &mut State, impedances: &Impedances, Vin: f32) -> f32 {
    let C2_b : f32 = state.C2_z; // C2 reflected
    let R2_b : f32 = 0 as f32; // R2 reflected
    let S2_b : f32 = -(R2_b + C2_b); // S2 reflected
    let R1_b : f32 = 0 as f32; // R1 reflected
    let P1_b : f32 = S2_b - impedances.P1_pr * (S2_b - R1_b); // P1 reflected
    let Vin_a : f32 = P1_b; // Vin incident
    let Vin_a0a1 : f32 = 2.0 * (Vin + state.Vin_z) + Vin_a; // Vin reflected
    let Vin_b : f32 = impedances.Vin_ca * Vin_a - impedances.Vin_ce * Vin_a0a1; // Vin reflected
    state.Vin_z += -Vin_b - Vin_a0a1; // Vin_z state update
    let P1_a : f32 = Vin_b; // P1 incident
    let R1_a : f32 = P1_b - R1_b + P1_a; // R1 incident
    let S2_a : f32 = P1_b - S2_b + P1_a; // S2 incident
    let R2_a : f32 = R2_b - impedances.S2_pr * (S2_a - S2_b); // R2 incident
    let C2_a : f32 = -S2_a - R2_b + impedances.S2_pr * (S2_a - S2_b); // C2 incident
    state.C2_z = C2_a; // C2 state update
    
    let v_C2 : f32 = (C2_a + C2_b) * (0.5 as f32); // C2 voltage
    return v_C2;
}

