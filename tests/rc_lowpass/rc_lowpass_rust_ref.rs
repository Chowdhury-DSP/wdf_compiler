pub struct Params {
    pub R1_value: f32, // = 1.0e+03
    pub C1_value: f32, // = 1.0e-06
}

#[derive(Default)]
pub struct Impedances {
    pub Vin_ca: f32,
    pub Vin_ce: f32,
}

#[derive(Default)]
pub struct State {
    pub C1_z: f32,
}

pub fn calc_impedances(impedances: &mut Impedances, fs: f32, params: Params) {
    // Equivalent to: const float T = 1 / fs;
    let _t: f32 = 1.0 / fs;

    // Computing impedance for: C1
    let C1_value = params.C1_value;
    let C1_g = 2.0 * C1_value * fs;
    let C1_r = 1.0 / C1_g;

    // Computing impedance for: Vin
    let vin_res_value = params.R1_value;

    impedances.Vin_ca = (vin_res_value - C1_r) / (vin_res_value + C1_r);
    impedances.Vin_ce = (2.0 * C1_r) / (vin_res_value + C1_r);
}

pub fn process(state: &mut State, impedances: &Impedances, Vin: f32) -> f32 {
    let C1_b = state.C1_z; // C1 reflected
    let Vin_a = C1_b; // Vin incident
    let Vin_b = impedances.Vin_ca * Vin_a + impedances.Vin_ce * -Vin; // Vin reflected
    let C1_a = Vin_b; // C1 incident
    state.C1_z = C1_a; // C1 state update

    let v_C1 = (C1_a + C1_b) * 0.5_f32; // C1 voltage
    v_C1
}

// #[allow(unused_variables)]
