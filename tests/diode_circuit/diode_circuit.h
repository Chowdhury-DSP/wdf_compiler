// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler diode_circuit.net diode_circuit.h -netlist

#pragma once

#include <wdf_lib_diode.h>

struct Params {
    static constexpr float C1_value = 1.0e-09;
    static constexpr float V1_res_value = 1.0e+02;
    struct wdf_lib::diode::Diode_Params DP_params;
};

struct Impedances {
    float P4_pr;
    struct wdf_lib::diode::Diode_Vars DP_vars;
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
    [[maybe_unused]] const auto V1_res_value = params.V1_res_value;
    [[maybe_unused]] const auto V1_R = V1_res_value;
    [[maybe_unused]] const auto V1_G = 1.0 / V1_R;
    // Computing impedance for: P4;
    [[maybe_unused]] const auto P4_G = C1_G + V1_G;
    [[maybe_unused]] const auto P4_R = 1.0 / P4_G;
    impedances.P4_pr = C1_G * P4_R;

    // Computing impedance for: DP;
    wdf_lib::diode::update_vars(&(impedances.DP_vars), &(params.DP_params), P4_R, P4_G);
}

inline float process(State& state, const Impedances& impedances, float V1) {
    const auto V1_b = V1; // V1 reflected
    const auto C1_b = state.C1_z; // C1 reflected
    const auto P4_b = V1_b - impedances.P4_pr * (V1_b - C1_b); // P4 reflected
    const auto P4_a = wdf_lib::diode::root_compute(&(impedances.DP_vars), P4_b); // P4 incident
    const auto C1_a = P4_b - C1_b + P4_a; // C1 incident
    const auto V1_a = P4_b - V1_b + P4_a; // V1 incident
    state.C1_z = C1_a; // C1 state update
    
    const auto v_C1 = (C1_a + C1_b) * (float) 0.5; // C1 voltage
    return v_C1;
}

