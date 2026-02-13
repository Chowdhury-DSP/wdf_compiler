#[derive(Default,Copy,Clone)]
pub struct L_Params {
    pub value: f32,
}

#[derive(Default,Copy,Clone)]
pub struct L_State {
    pub z: f32,
}

pub fn update_vars(params: L_Params, fs: f32) -> f32 {
    return 2.0 * params.value * fs;
}

pub fn reflected(state: &mut L_State) -> f32
{
    return -state.z;
}

pub fn incident(state: &mut L_State, a: f32)
{
    state.z = a / (f32::sqrt (a * a + 1.0));
}
