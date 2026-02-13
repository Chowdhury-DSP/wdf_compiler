#[derive(Default,Copy,Clone)]
pub struct HC_Params {
    pub A : f32, // = 1.0
}

#[derive(Default,Copy,Clone)]
pub struct HC_Vars {
    pub A : f32 // = 1.0
}

#[allow(unused)]
pub fn update_vars(vars: &mut HC_Vars, params: HC_Params, child_R: f32, child_G: f32) {
    vars.A = params.A;
}

#[allow(unused)]
pub fn root_compute(vars: &HC_Vars, a: f32) -> f32 {
    let b : f32;
    if f32::abs (a) < vars.A * 0.5 {
        b = a;
    } else {
        b = -a + (if a >= 0.0 { vars.A } else { -vars.A });
    }
    return b;
}
