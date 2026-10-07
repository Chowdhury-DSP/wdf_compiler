// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler diode_clipper.net diode_clipper.h -netlist

#pragma once

#include <wdf_lib_diode_pair.h>

struct Params {
    static constexpr float C1_value = 1.0e-06;
    static constexpr float Vin_res_value = 1.0e+03;
    struct wdf_lib::diode_pair::Diode_Pair_Params DP_params;
};

struct Impedances {
    float P4_pr;
    struct wdf_lib::diode_pair::Diode_Pair_Vars DP_vars;
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
    // Computing impedance for: Vin;
    [[maybe_unused]] const auto Vin_res_value = params.Vin_res_value;
    [[maybe_unused]] const auto Vin_R = Vin_res_value;
    [[maybe_unused]] const auto Vin_G = 1.0 / Vin_R;
    // Computing impedance for: P4;
    [[maybe_unused]] const auto P4_G = C1_G + Vin_G;
    [[maybe_unused]] const auto P4_R = 1.0 / P4_G;
    impedances.P4_pr = C1_G * P4_R;

    // Computing impedance for: DP;
    wdf_lib::diode_pair::update_vars(&(impedances.DP_vars), &(params.DP_params), P4_R, P4_G);
}

inline float process(State& state, const Impedances& impedances, float Vin) {
    const auto Vin_b = Vin; // Vin reflected
    const auto C1_b = state.C1_z; // C1 reflected
    const auto P4_b = Vin_b - impedances.P4_pr * (Vin_b - C1_b); // P4 reflected
    const auto P4_a = wdf_lib::diode_pair::root_compute(&(impedances.DP_vars), P4_b); // P4 incident
    const auto C1_a = P4_b - C1_b + P4_a; // C1 incident
    const auto Vin_a = P4_b - Vin_b + P4_a; // Vin incident
    state.C1_z = C1_a; // C1 state update
    
    const auto v_C1 = (C1_a + C1_b) * (float) 0.5; // C1 voltage
    return v_C1;
}

