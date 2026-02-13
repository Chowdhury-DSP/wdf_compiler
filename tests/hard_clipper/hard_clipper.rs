// Auto-generated with wdf_compiler version 1.0.0.630eddd.
// Command: wdf_compiler hard_clipper.wdf hard_clipper.rs -lang rust

use crate::custom_hard_clipper;

#[derive(Default,Copy,Clone)]
pub struct Params {
    pub C1_value : f32,
    pub Vin_res_value : f32,
    pub HC_params: custom_hard_clipper::HC_Params,
}

#[derive(Default,Copy,Clone)]
pub struct Impedances {
    pub P1_pr : f32,
    pub HC_vars: custom_hard_clipper::HC_Vars,
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
    let Vin_res_value = params.Vin_res_value;
    let Vin_R = Vin_res_value;
    let Vin_G = 1.0 / Vin_R;
    // Computing impedance for: P1;
    let P1_G = C1_G + Vin_G;
    let P1_R = 1.0 / P1_G;
    impedances.P1_pr = C1_G * P1_R;

    // Computing impedance for: HC;
    custom_hard_clipper::update_vars(&mut impedances.HC_vars, params.HC_params, P1_R, P1_G);
}

#[allow(unused)]
pub fn process(state: &mut State, impedances: &Impedances, Vin: f32) -> f32 {
    let Vin_b : f32 = Vin; // Vin reflected
    let C1_b : f32 = state.C1_z; // C1 reflected
    let P1_b : f32 = Vin_b - impedances.P1_pr * (Vin_b - C1_b); // P1 reflected
    let P1_a = custom_hard_clipper::root_compute(&(impedances.HC_vars), P1_b); // P1 incident
    let C1_a : f32 = P1_b - C1_b + P1_a; // C1 incident
    let Vin_a : f32 = P1_b - Vin_b + P1_a; // Vin incident
    state.C1_z = C1_a; // C1 state update
    
    let v_C1 : f32 = (C1_a + C1_b) * (0.5 as f32); // C1 voltage
    return v_C1;
}

