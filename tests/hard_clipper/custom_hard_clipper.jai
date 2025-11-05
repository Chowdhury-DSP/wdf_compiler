HC_Params :: struct {
    A : float32 = 1.0;
}

HC_Vars :: struct {
    A : float32 = 1.0;
}

update_vars :: (vars: *HC_Vars,
                params: HC_Params,
                child_R: float32,
                child_G: float32)
{
    vars.A = params.A;
}

root_compute :: (vars: *HC_Vars, a: float32) -> float32
{
    b: float32;
    if abs (a) < vars.A * 0.5 { b = a; }
    else { b = -a + (ifx a >= 0.0 then vars.A else -vars.A); }
    return b;
}
