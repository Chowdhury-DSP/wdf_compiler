L_Params :: struct {
    value: float32;
}

L_State :: struct {
    z: float32;
}

update_vars :: (params: L_Params, fs: float32) -> float32 {
    return 2 * params.value * fs;
}

reflected :: (state: *L_State) -> float32
{
    return -state.z;
}

incident :: (state: *L_State, a: float32)
{
    state.z = a / (Math.sqrt (a * a + 1.0));
}

Math :: #import "Math";
