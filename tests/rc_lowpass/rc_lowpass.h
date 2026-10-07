// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler rc_lowpass.net rc_lowpass.h -netlist

#pragma once

struct Params {
    static constexpr float R1_value = 1.0e+03;
    static constexpr float C1_value = 1.0e-06;
};

struct Impedances {
    float V1_ca;
    float V1_ce;
};

struct State {
    float C1_z;
};

inline void calc_impedances(Impedances& impedances, float fs, Params params = {}) {
    [[maybe_unused]] static constexpr auto sum = [] (auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [] (auto a, auto b) { return (a * b) / (a + b); };
    
    [[maybe_unused]] const auto T = 1.0 / fs;
    
    // Computing impedance for: C1;
    [[maybe_unused]] const auto C1_value = params.C1_value;
    [[maybe_unused]] const auto C1_G = 2.0 * C1_value * fs;
    [[maybe_unused]] const auto C1_R = 1.0 / C1_G;
    
    // Computing impedance for: V1;
    [[maybe_unused]] const auto V1_res_value = params.R1_value;
    impedances.V1_ca = (V1_res_value - C1_R) / (V1_res_value + C1_R);
    impedances.V1_ce = (2.0 * C1_R) / (V1_res_value + C1_R);

}

inline float process(State& state, const Impedances& impedances, float V1) {
    const auto C1_b = state.C1_z; // C1 reflected
    const auto V1_a = C1_b; // V1 incident
    const auto V1_b = impedances.V1_ca * V1_a + impedances.V1_ce * -(V1); // V1 reflected
    const auto C1_a = V1_b; // C1 incident
    state.C1_z = C1_a; // C1 state update
    
    const auto v_C1 = (C1_a + C1_b) * (float) 0.5; // C1 voltage
    return v_C1;
}

