// Auto-generated with wdf_compiler version 1.0.0.630eddd.
// Command: wdf_compiler rc_lowpass_var.wdf rc_lowpass_var.rs -lang rust

#[derive(Default,Copy,Clone)]
pub struct Params {
    pub Vplus_v_value : f32, // = 1.0e+01
    pub Vin_res_value : f32, // = 1.0e+03
    pub Vminus_v_value : f32, // = -5.0e+00
    pub Vminus_res_value : f32,
    pub R1_v_value : f32,
    pub R1_res_value : f32,
    pub C1_value : f32,
}

#[derive(Default,Copy,Clone)]
pub struct Impedances {
    pub Vplus_V : f32, // = Params.Vplus_v_value
    pub Vminus_V : f32, // = Params.Vminus_v_value
    pub R1_V : f32,
    pub S3_pr : f32,
    pub S2_pr : f32,
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
    
    // Computing impedance for: Vin;
    let Vin_res_value = params.Vin_res_value;
    let Vin_R = Vin_res_value;
    let Vin_G = 1.0 / Vin_R;
    // Computing voltage for: Vminus;
    impedances.Vminus_V = params.Vminus_v_value;

    // Computing impedance for: Vminus;
    let Vminus_res_value = params.Vminus_res_value;
    let Vminus_R = Vminus_res_value;
    let Vminus_G = 1.0 / Vminus_R;
    // Computing voltage for: R1;
    impedances.R1_V = params.R1_v_value;

    // Computing impedance for: R1;
    let R1_res_value = params.R1_res_value;
    let R1_R = R1_res_value;
    let R1_G = 1.0 / R1_R;
    // Computing impedance for: C1;
    let C1_value = params.C1_value;
    let C1_G = 2.0 * C1_value * fs;
    let C1_R = 1.0 / C1_G;
    // Computing impedance for: S3;
    let S3_R = R1_R + C1_R;
    let S3_G = 1.0 / S3_R;
    impedances.S3_pr = R1_R * S3_G;

    // Computing impedance for: S2;
    let S2_R = Vminus_R + S3_R;
    let S2_G = 1.0 / S2_R;
    impedances.S2_pr = Vminus_R * S2_G;

    // Computing impedance for: S1;
    let S1_R = Vin_R + S2_R;
    let S1_G = 1.0 / S1_R;
    impedances.S1_pr = Vin_R * S1_G;

    
    // Computing voltage for: Vplus;
    impedances.Vplus_V = params.Vplus_v_value;

}

#[allow(unused)]
pub fn process(state: &mut State, impedances: &Impedances, Vin: f32) -> f32 {
    let C1_b : f32 = state.C1_z; // C1 reflected
    let R1_b : f32 = impedances.R1_V; // R1 reflected
    let S3_b : f32 = -(R1_b + C1_b); // S3 reflected
    let Vminus_b : f32 = impedances.Vminus_V; // Vminus reflected
    let S2_b : f32 = -(Vminus_b + S3_b); // S2 reflected
    let Vin_b : f32 = Vin; // Vin reflected
    let S1_b : f32 = -(Vin_b + S2_b); // S1 reflected
    let Vplus_a : f32 = S1_b; // Vplus incident
    let Vplus_b : f32 = -Vplus_a + 2.0 * impedances.Vplus_V; // Vplus reflected
    let S1_a : f32 = Vplus_b; // S1 incident
    let Vin_a : f32 = Vin_b - impedances.S1_pr * (S1_a - S1_b); // Vin incident
    let S2_a : f32 = -S1_a - Vin_b + impedances.S1_pr * (S1_a - S1_b); // S2 incident
    let Vminus_a : f32 = Vminus_b - impedances.S2_pr * (S2_a - S2_b); // Vminus incident
    let S3_a : f32 = -S2_a - Vminus_b + impedances.S2_pr * (S2_a - S2_b); // S3 incident
    let R1_a : f32 = R1_b - impedances.S3_pr * (S3_a - S3_b); // R1 incident
    let C1_a : f32 = -S3_a - R1_b + impedances.S3_pr * (S3_a - S3_b); // C1 incident
    state.C1_z = C1_a; // C1 state update
    
    let v_C1 : f32 = (C1_a + C1_b) * (0.5 as f32); // C1 voltage
    return v_C1;
}

