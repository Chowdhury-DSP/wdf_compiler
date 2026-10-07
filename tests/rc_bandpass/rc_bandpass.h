// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler rc_bandpass.net rc_bandpass.h -netlist

#pragma once

struct Params {
    static constexpr float C1_value = 1.0e-06;
    static constexpr float R1_value = 1.0e+03;
    static constexpr float C2_value = 1.0e-06;
    static constexpr float R2_value = 1.0e+03;
};

struct Impedances {
    float Vin_ca;
    float Vin_ce;
    float Vin_cr;
    float S7_pr;
};

struct State {
    float Vin_z;
    float C2_z;
};

inline void calc_impedances(Impedances& impedances, float fs, Params params = {}) {
    [[maybe_unused]] static constexpr auto sum = [] (auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [] (auto a, auto b) { return (a * b) / (a + b); };
    
    [[maybe_unused]] const auto T = 1.0 / fs;
    
    // Computing impedance for: C2;
    [[maybe_unused]] const auto C2_value = params.C2_value;
    [[maybe_unused]] const auto C2_G = 2.0 * C2_value * fs;
    [[maybe_unused]] const auto C2_R = 1.0 / C2_G;
    // Computing impedance for: R2;
    [[maybe_unused]] const auto R2_value = params.R2_value;
    [[maybe_unused]] const auto R2_R = R2_value;
    [[maybe_unused]] const auto R2_G = 1.0 / R2_R;
    // Computing impedance for: S7;
    [[maybe_unused]] const auto S7_R = C2_R + R2_R;
    [[maybe_unused]] const auto S7_G = 1.0 / S7_R;
    impedances.S7_pr = C2_R * S7_G;

    
    // Computing impedance for: Vin;
    [[maybe_unused]] const auto Vin_res_value = params.R1_value;
    [[maybe_unused]] const auto Vin_cap_value = params.C1_value;
    [[maybe_unused]] const auto Vin_Gp = (1.0 / Vin_res_value) + S7_G;
    [[maybe_unused]] const auto Vin_Rp = 1.0 / Vin_Gp;
    impedances.Vin_ca = T / (T + 2.0 * Vin_Rp * Vin_cap_value);
    impedances.Vin_ce = (2.0 * Vin_Rp * Vin_cap_value) / (T + 2.0 * Vin_Rp * Vin_cap_value);
    impedances.Vin_cr = Vin_Rp / Vin_res_value;

}

inline float process(State& state, const Impedances& impedances, float Vin) {
    const auto R2_b = 0; // R2 reflected
    const auto C2_b = state.C2_z; // C2 reflected
    const auto S7_b = -(C2_b + R2_b); // S7 reflected
    const auto Vin_a = S7_b; // Vin incident
    const auto Vin_a_cvs = impedances.Vin_cr * Vin_a; // Vin_CVs incident
    const auto Vin_a0a1 = 2.0 * (Vin + state.Vin_z) + Vin_a_cvs; // Vin_CVs reflected
    const auto Vin_b_cvs = impedances.Vin_ca * Vin_a_cvs - impedances.Vin_ce * Vin_a0a1; // Vin_CVs reflected
    const auto Vin_b = Vin_b_cvs - Vin_a_cvs; // Vin reflected
    state.Vin_z += -Vin_b_cvs - Vin_a0a1; // Vin_z state update
    const auto S7_a = Vin_b; // S7 incident
    const auto C2_a = C2_b - impedances.S7_pr * (S7_a - S7_b); // C2 incident
    const auto R2_a = -S7_a - C2_b + impedances.S7_pr * (S7_a - S7_b); // R2 incident
    state.C2_z = C2_a; // C2 state update
    
    const auto v_C2 = (C2_a + C2_b) * (float) 0.5; // C2 voltage
    return v_C2;
}

