// Auto-generated with wdf_compiler version 1.0.0.8d8c5d4.
// Command: wdf_compiler rc_lowpass.wdf rc_lowpass.h

#pragma once

struct Params {
    static constexpr float R1_value = 1.0e+03;
    static constexpr float C1_value = 1.0e-06;
};

struct Impedances {
    float S1_pr;
};

struct State {
    float C1_z;
};

inline void calc_impedances(Impedances& impedances, float fs, Params params = {}) {
    [[maybe_unused]] static constexpr auto sum = [] (auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [] (auto a, auto b) { return (a * b) / (a + b); };
    
    [[maybe_unused]] const auto T = 1 / fs;
    
    // Computing impedance for: R1;
    [[maybe_unused]] const auto R1_value = params.R1_value;
    [[maybe_unused]] const auto R1_R = R1_value;
    [[maybe_unused]] const auto R1_G = 1 / R1_R;
    // Computing impedance for: C1;
    [[maybe_unused]] const auto C1_value = params.C1_value;
    [[maybe_unused]] const auto C1_G = 2 * C1_value * fs;
    [[maybe_unused]] const auto C1_R = 1 / C1_G;
    // Computing impedance for: S1;
    [[maybe_unused]] const auto S1_R = R1_R + C1_R;
    [[maybe_unused]] const auto S1_G = 1 / S1_R;
    impedances.S1_pr = R1_R * S1_G;

}

inline float process(State& state, const Impedances& impedances, float Vin) {
    const auto C1_b = state.C1_z; // C1 reflected
    const auto R1_b = 0; // R1 reflected
    const auto S1_b = -(R1_b + C1_b); // S1 reflected
    const auto Vin_a = S1_b; // Vin incident
    const auto Vin_b = -Vin_a + 2 * Vin; // Vin reflected
    const auto S1_a = Vin_b; // S1 incident
    const auto R1_a = R1_b - impedances.S1_pr * (S1_a - S1_b); // R1 incident
    const auto C1_a = -S1_a - R1_b + impedances.S1_pr * (S1_a - S1_b); // C1 incident
    state.C1_z = C1_a; // C1 state update
    
    const auto v_C1 = (C1_a + C1_b) * (float) 0.5; // C1 voltage
    return v_C1;
}

