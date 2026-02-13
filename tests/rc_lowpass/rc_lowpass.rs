// Auto-generated with wdf_compiler version 1.0.0.630eddd.
// Command: wdf_compiler rc_lowpass.wdf rc_lowpass.rs -lang rust

#[derive(Default,Copy,Clone)]
pub struct Params {
    pub R1_value : f32, // = 1.0e+03
    pub C1_value : f32, // = 1.0e-06
}

#[derive(Default,Copy,Clone)]
pub struct Impedances {
    pub Vin_ca : f32,
    pub Vin_ce : f32,
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
    
    // Computing impedance for: C1;
    let C1_value = params.C1_value;
    let C1_G = 2.0 * C1_value * fs;
    let C1_R = 1.0 / C1_G;
    
    // Computing impedance for: Vin;
    let Vin_res_value = params.R1_value;
    impedances.Vin_ca = (Vin_res_value - C1_R) / (Vin_res_value + C1_R);
    impedances.Vin_ce = (2.0 * C1_R) / (Vin_res_value + C1_R);

}

#[allow(unused)]
pub fn process(state: &mut State, impedances: &Impedances, Vin: f32) -> f32 {
    let C1_b : f32 = state.C1_z; // C1 reflected
    let Vin_a : f32 = C1_b; // Vin incident
    let Vin_b : f32 = impedances.Vin_ca * Vin_a + impedances.Vin_ce * -(Vin); // Vin reflected
    let C1_a : f32 = Vin_b; // C1 incident
    state.C1_z = C1_a; // C1 state update
    
    let v_C1 : f32 = (C1_a + C1_b) * (0.5 as f32); // C1 voltage
    return v_C1;
}

