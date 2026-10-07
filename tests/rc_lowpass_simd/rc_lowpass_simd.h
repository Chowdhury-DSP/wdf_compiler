// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler rc_lowpass_simd.wdf rc_lowpass_simd.h -dtype WDF_Float

#pragma once

struct Params {
    const WDF_Float R1_value = 1.0e+03;
    const WDF_Float C1_value = 1.0e-06;
};

struct Impedances {
    WDF_Float Vin_ca;
    WDF_Float Vin_ce;
};

struct State {
    WDF_Float C1_z;
};

inline void calc_impedances(Impedances& impedances, WDF_Float fs, Params params = {}) {
    [[maybe_unused]] static constexpr auto sum = [] (auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [] (auto a, auto b) { return (a * b) / (a + b); };
    
    [[maybe_unused]] const auto T = 1.0 / fs;
    
    // Computing impedance for: C1;
    [[maybe_unused]] const auto C1_value = params.C1_value;
    [[maybe_unused]] const auto C1_G = 2.0 * C1_value * fs;
    [[maybe_unused]] const auto C1_R = 1.0 / C1_G;
    
    // Computing impedance for: Vin;
    [[maybe_unused]] const auto Vin_res_value = params.R1_value;
    impedances.Vin_ca = (Vin_res_value - C1_R) / (Vin_res_value + C1_R);
    impedances.Vin_ce = (2.0 * C1_R) / (Vin_res_value + C1_R);

}

inline WDF_Float process(State& state, const Impedances& impedances, WDF_Float Vin) {
    const auto C1_b = state.C1_z; // C1 reflected
    const auto Vin_a = C1_b; // Vin incident
    const auto Vin_b = impedances.Vin_ca * Vin_a + impedances.Vin_ce * -(Vin); // Vin reflected
    const auto C1_a = Vin_b; // C1 incident
    state.C1_z = C1_a; // C1 state update
    
    const auto v_C1 = (C1_a + C1_b) * (WDF_Float) 0.5; // C1 voltage
    return v_C1;
}

