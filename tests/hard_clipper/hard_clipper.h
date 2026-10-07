// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler hard_clipper.wdf hard_clipper.h

#pragma once

#include "custom_hard_clipper.h"

struct Params {
    float C1_value;
    float Vin_res_value = 1.0e-6;
    struct HC_Params HC_params;
};

struct Impedances {
    float P1_pr;
    struct HC_Vars HC_vars;
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
    // Computing impedance for: P1;
    [[maybe_unused]] const auto P1_G = C1_G + Vin_G;
    [[maybe_unused]] const auto P1_R = 1.0 / P1_G;
    impedances.P1_pr = C1_G * P1_R;

    // Computing impedance for: HC;
    update_vars(&(impedances.HC_vars), &(params.HC_params), P1_R, P1_G);
}

inline float process(State& state, const Impedances& impedances, float Vin) {
    const auto Vin_b = Vin; // Vin reflected
    const auto C1_b = state.C1_z; // C1 reflected
    const auto P1_b = Vin_b - impedances.P1_pr * (Vin_b - C1_b); // P1 reflected
    const auto P1_a = root_compute(&(impedances.HC_vars), P1_b); // P1 incident
    const auto C1_a = P1_b - C1_b + P1_a; // C1 incident
    const auto Vin_a = P1_b - Vin_b + P1_a; // Vin incident
    state.C1_z = C1_a; // C1 state update
    
    const auto v_C1 = (C1_a + C1_b) * (float) 0.5; // C1 voltage
    return v_C1;
}

