// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler rl_lowpass.wdf rl_lowpass.h

#pragma once

namespace test::rl_lowpass
{

struct Params {
    static constexpr float R1_value = 1.0e+03;
    static constexpr float L1_value = 1.0e-03;
};

struct Impedances {
    float P1_R;
    float R1_G;
    float P1_pr;
};

struct State {
    float L1_z;
};

inline void calc_impedances(Impedances& impedances, float fs, Params params = {}) {
    [[maybe_unused]] static constexpr auto sum = [] (auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [] (auto a, auto b) { return (a * b) / (a + b); };
    
    [[maybe_unused]] const auto T = 1.0 / fs;
    
    // Computing impedance for: R1;
    [[maybe_unused]] const auto R1_value = params.R1_value;
    [[maybe_unused]] const auto R1_R = R1_value;
    [[maybe_unused]] const auto R1_G = 1.0 / R1_R;
    impedances.R1_G = R1_G;
    // Computing impedance for: L1;
    [[maybe_unused]] const auto L1_value = params.L1_value;
    [[maybe_unused]] const auto L1_R = 2.0 * L1_value * fs;
    [[maybe_unused]] const auto L1_G = 1.0 / L1_R;
    // Computing impedance for: P1;
    [[maybe_unused]] const auto P1_G = R1_G + L1_G;
    [[maybe_unused]] const auto P1_R = 1.0 / P1_G;
    impedances.P1_R = P1_R;
    impedances.P1_pr = R1_G * P1_R;

}

inline float process(State& state, const Impedances& impedances, float Iin) {
    const auto L1_b = -state.L1_z; // L1 reflected
    const auto R1_b = 0; // R1 reflected
    const auto P1_b = L1_b - impedances.P1_pr * (L1_b - R1_b); // P1 reflected
    const auto Iin_a = P1_b; // Iin incident
    const auto Iin_b = 2.0 * impedances.P1_R * Iin + Iin_a; // Iin reflected (@TODO: 2.0 * R?)
    const auto P1_a = Iin_b; // P1 incident
    const auto R1_a = P1_b - R1_b + P1_a; // R1 incident
    const auto L1_a = P1_b - L1_b + P1_a; // L1 incident
    state.L1_z = L1_a; // L1 state update
    
    const auto i_R1 = (R1_a - R1_b) * ((float) 0.5 * impedances.R1_G); // R1 current
    return i_R1;
}

} // namespace test::rl_lowpass

