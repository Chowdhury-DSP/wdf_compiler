// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler rc_bandpass_var.wdf rc_bandpass_var.h

#pragma once

struct Params {
    float C1_value;
    float R1_value;
    float R2_value;
    float C2_value;
};

struct Impedances {
    float C1_R;
    float R1_G;
    float S2_G;
    float S2_pr;
    float P1_pr;
    float S1_pr;
};

struct State {
    float C1_z;
    float C2_z;
};

inline void calc_impedances(Impedances& impedances, float fs, Params params = {}) {
    [[maybe_unused]] static constexpr auto sum = [] (auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [] (auto a, auto b) { return (a * b) / (a + b); };
    
    [[maybe_unused]] const auto T = 1.0 / fs;
    
    // Computing impedance for: C1;
    [[maybe_unused]] const auto C1_value = params.C1_value;
    [[maybe_unused]] const auto C1_G = 2.0 * C1_value * fs;
    [[maybe_unused]] const auto C1_R = 1.0 / C1_G;
    impedances.C1_R = C1_R;
    // Computing impedance for: R1;
    [[maybe_unused]] const auto R1_value = params.R1_value;
    [[maybe_unused]] const auto R1_R = R1_value;
    [[maybe_unused]] const auto R1_G = 1.0 / R1_R;
    impedances.R1_G = R1_G;
    // Computing impedance for: R2;
    [[maybe_unused]] const auto R2_value = params.R2_value;
    [[maybe_unused]] const auto R2_R = R2_value;
    [[maybe_unused]] const auto R2_G = 1.0 / R2_R;
    // Computing impedance for: C2;
    [[maybe_unused]] const auto C2_value = params.C2_value;
    [[maybe_unused]] const auto C2_G = 2.0 * C2_value * fs;
    [[maybe_unused]] const auto C2_R = 1.0 / C2_G;
    // Computing impedance for: S2;
    [[maybe_unused]] const auto S2_R = R2_R + C2_R;
    [[maybe_unused]] const auto S2_G = 1.0 / S2_R;
    impedances.S2_G = S2_G;
    impedances.S2_pr = R2_R * S2_G;

    // Computing impedance for: P1;
    [[maybe_unused]] const auto P1_G = R1_G + S2_G;
    [[maybe_unused]] const auto P1_R = 1.0 / P1_G;
    impedances.P1_pr = R1_G * P1_R;

    // Computing impedance for: S1;
    [[maybe_unused]] const auto S1_R = C1_R + P1_R;
    [[maybe_unused]] const auto S1_G = 1.0 / S1_R;
    impedances.S1_pr = C1_R * S1_G;

    // Computing impedance for: I1;
    [[maybe_unused]] const auto I1_R = S1_R;
    [[maybe_unused]] const auto I1_G = 1.0 / I1_R;
}

inline void calc_impedances_lpf1(Impedances& impedances, float fs, Params params) {
    [[maybe_unused]] static constexpr auto sum = [] (auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [] (auto a, auto b) { return (a * b) / (a + b); };
    
    [[maybe_unused]] const auto T = 1.0 / fs;
    
    // Computing impedance for: C1;
    [[maybe_unused]] const auto C1_value = params.C1_value;
    [[maybe_unused]] const auto C1_G = 2.0 * C1_value * fs;
    [[maybe_unused]] const auto C1_R = 1.0 / C1_G;
    // Computing impedance for: R1;
    [[maybe_unused]] const auto R1_value = params.R1_value;
    [[maybe_unused]] const auto R1_R = R1_value;
    [[maybe_unused]] const auto R1_G = 1.0 / R1_R;
    [[maybe_unused]] const auto S2_G = impedances.S2_G;
    // Computing impedance for: P1;
    [[maybe_unused]] const auto P1_G = R1_G + S2_G;
    [[maybe_unused]] const auto P1_R = 1.0 / P1_G;
    impedances.P1_pr = R1_G * P1_R;

    // Computing impedance for: S1;
    [[maybe_unused]] const auto S1_R = C1_R + P1_R;
    [[maybe_unused]] const auto S1_G = 1.0 / S1_R;
    impedances.S1_pr = C1_R * S1_G;

    // Computing impedance for: I1;
    [[maybe_unused]] const auto I1_R = S1_R;
    [[maybe_unused]] const auto I1_G = 1.0 / I1_R;
}

inline void calc_impedances_lpf2(Impedances& impedances, float fs, Params params) {
    [[maybe_unused]] static constexpr auto sum = [] (auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [] (auto a, auto b) { return (a * b) / (a + b); };
    
    [[maybe_unused]] const auto T = 1.0 / fs;
    
    // Computing impedance for: R2;
    [[maybe_unused]] const auto R2_value = params.R2_value;
    [[maybe_unused]] const auto R2_R = R2_value;
    [[maybe_unused]] const auto R2_G = 1.0 / R2_R;
    // Computing impedance for: C2;
    [[maybe_unused]] const auto C2_value = params.C2_value;
    [[maybe_unused]] const auto C2_G = 2.0 * C2_value * fs;
    [[maybe_unused]] const auto C2_R = 1.0 / C2_G;
    // Computing impedance for: S2;
    [[maybe_unused]] const auto S2_R = R2_R + C2_R;
    [[maybe_unused]] const auto S2_G = 1.0 / S2_R;
    impedances.S2_G = S2_G;
    impedances.S2_pr = R2_R * S2_G;

    [[maybe_unused]] const auto R1_G = impedances.R1_G;
    // Computing impedance for: P1;
    [[maybe_unused]] const auto P1_G = R1_G + S2_G;
    [[maybe_unused]] const auto P1_R = 1.0 / P1_G;
    impedances.P1_pr = R1_G * P1_R;

    [[maybe_unused]] const auto C1_R = impedances.C1_R;
    // Computing impedance for: S1;
    [[maybe_unused]] const auto S1_R = C1_R + P1_R;
    [[maybe_unused]] const auto S1_G = 1.0 / S1_R;
    impedances.S1_pr = C1_R * S1_G;

    // Computing impedance for: I1;
    [[maybe_unused]] const auto I1_R = S1_R;
    [[maybe_unused]] const auto I1_G = 1.0 / I1_R;
}

inline float process(State& state, const Impedances& impedances, float Vin) {
    const auto C2_b = state.C2_z; // C2 reflected
    const auto R2_b = 0; // R2 reflected
    const auto S2_b = -(R2_b + C2_b); // S2 reflected
    const auto R1_b = 0; // R1 reflected
    const auto P1_b = S2_b - impedances.P1_pr * (S2_b - R1_b); // P1 reflected
    const auto C1_b = state.C1_z; // C1 reflected
    const auto S1_b = -(C1_b + P1_b); // S1 reflected
    const auto I1_b = -S1_b; // I1 reflected
    const auto Vin_a = I1_b; // Vin incident
    const auto Vin_b = -Vin_a + 2.0 * Vin; // Vin reflected
    const auto I1_a = Vin_b; // I1 incident
    const auto S1_a = -I1_a; // S1 incident
    const auto C1_a = C1_b - impedances.S1_pr * (S1_a - S1_b); // C1 incident
    const auto P1_a = -S1_a - C1_b + impedances.S1_pr * (S1_a - S1_b); // P1 incident
    state.C1_z = C1_a; // C1 state update
    const auto R1_a = P1_b - R1_b + P1_a; // R1 incident
    const auto S2_a = P1_b - S2_b + P1_a; // S2 incident
    const auto R2_a = R2_b - impedances.S2_pr * (S2_a - S2_b); // R2 incident
    const auto C2_a = -S2_a - R2_b + impedances.S2_pr * (S2_a - S2_b); // C2 incident
    state.C2_z = C2_a; // C2 state update
    
    const auto v_C2 = (C2_a + C2_b) * (float) 0.5; // C2 voltage
    return v_C2;
}

