// Translated from wdf_lib_omega.h
// which was originally written by Stefano D'Angelo.
// Please see the copyright notice at the top of that file.

omega4 :: (x: float32) -> float32 {
    y := omega3 (x);
    return y - (y - exp_approx (x - y)) / (y + 1);
}

omega3 :: (x: float32) -> float32 {
    x1 :: cast(float32) -3.341459552768620;
    x2 :: cast(float32) 8.0;
    a :: cast(float32) -1.314293149877800e-3;
    b :: cast(float32) 4.775931364975583e-2;
    c :: cast(float32) 3.631952663804445e-1;
    d :: cast(float32) 6.313183464296682e-1;

    if (x < x1) { return 0; }
    if (x < x2) { x_sq := x * x; return (a * x_sq + c) * x + (b * x_sq + d); }
    return x - log_approx(x);
}

#scope_file

log_approx :: (x: float32) -> float32 {
    Convert :: struct {
        f: float32;
        #overlay (f) i: s32;
    }
    v: Convert;
    v.f = x;
    ex := v.i & 0x7f800000;
    e := (ex >> 23) - 127;
    v.i = (v.i - ex) | 0x3f800000;

    return cast(float32) 0.693147180559945 * (cast(float32) e + log2_approx(v.f));
}

log2_approx :: (x: float32) -> float32 {
    alpha :: cast(float32) 0.1640425613334452;
    beta :: cast(float32) -1.098865286222744;
    gamma :: cast(float32) 3.148297929334117;
    zeta :: cast(float32) -2.213475204444817;

    x_sq := x * x;
    return (alpha * x_sq + gamma) * x + (beta * x_sq + zeta);
}

exp_approx :: (x: float32) -> float32
{
    x = Basic.max(-126.0, cast(float32)1.442695040888963 * x);

    Convert :: struct {
        f: float32;
        #overlay (f) i: s32;
    }
    v: Convert;
    xi := cast(s32) x;
    l := ifx x < 0 then xi - 1 else xi;
    f := x - cast(float32) l;
    v.i = (l + 127) << 23;

    return v.f * pow2_approx (f);
}

pow2_approx :: (x: float32) -> float32
{
    alpha :: cast(float32) 0.07944154167983575;
    beta :: cast(float32) 0.2274112777602189;
    gamma :: cast(float32) 0.6931471805599453;
    zeta :: cast(float32) 1.0;

    x_sq := x * x;
    return (alpha * x_sq + gamma) * x + (beta * x_sq + zeta);
}

Basic :: #import "Basic";
Math :: #import "Math";
