// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler pulse_shaper.wdf pulse_shaper.h

#pragma once

#include "wdf_lib_diode_omega3.hpp"

struct Params {
    static constexpr float r162_value = 4.7e+03;
    static constexpr float c40_value = 1.5e-08;
    static constexpr float c40_alpha = 2.9e-02;
    static constexpr float r163_value = 1.0e+05;
    static constexpr float Vs_res_value = 1.0e-09;
    struct wdf_lib::diode::Diode_Params d53_params;
};

struct Impedances {
    float c40_b_coeff;
    float c40_a_coeff;
    float P1_pr;
    float S1_pr;
    float P2_pr;
    struct wdf_lib::diode::Diode_Vars d53_vars;
};

struct State {
    float c40_z;
    float c40_b1;
};

inline void calc_impedances(Impedances& impedances, float fs, Params params = {}) {
    [[maybe_unused]] static constexpr auto sum = [] (auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [] (auto a, auto b) { return (a * b) / (a + b); };
    
    [[maybe_unused]] const auto T = 1.0 / fs;
    
    // Computing impedance for: r162;
    [[maybe_unused]] const auto r162_value = params.r162_value;
    [[maybe_unused]] const auto r162_R = r162_value;
    [[maybe_unused]] const auto r162_G = 1.0 / r162_R;
    // Computing impedance for: c40;
    impedances.c40_b_coeff = (1 - params.c40_alpha) * 0.5;
    impedances.c40_a_coeff = (1 + params.c40_alpha) * 0.5;
    [[maybe_unused]] const auto c40_G = (1 + params.c40_alpha) * params.c40_value * fs;
    [[maybe_unused]] const auto c40_R = 1.0 / c40_G;
    // Computing impedance for: r163;
    [[maybe_unused]] const auto r163_value = params.r163_value;
    [[maybe_unused]] const auto r163_R = r163_value;
    [[maybe_unused]] const auto r163_G = 1.0 / r163_R;
    // Computing impedance for: Vs;
    [[maybe_unused]] const auto Vs_res_value = params.Vs_res_value;
    [[maybe_unused]] const auto Vs_R = Vs_res_value;
    [[maybe_unused]] const auto Vs_G = 1.0 / Vs_R;
    // Computing impedance for: P1;
    [[maybe_unused]] const auto P1_G = c40_G + r163_G;
    [[maybe_unused]] const auto P1_R = 1.0 / P1_G;
    impedances.P1_pr = c40_G * P1_R;

    // Computing impedance for: S1;
    [[maybe_unused]] const auto S1_R = P1_R + Vs_R;
    [[maybe_unused]] const auto S1_G = 1.0 / S1_R;
    impedances.S1_pr = P1_R * S1_G;

    // Computing impedance for: I1;
    [[maybe_unused]] const auto I1_R = r162_R;
    [[maybe_unused]] const auto I1_G = 1.0 / I1_R;
    // Computing impedance for: P2;
    [[maybe_unused]] const auto P2_G = I1_G + S1_G;
    [[maybe_unused]] const auto P2_R = 1.0 / P2_G;
    impedances.P2_pr = I1_G * P2_R;

    // Computing impedance for: d53;
    wdf_lib::diode::update_vars(&(impedances.d53_vars), &(params.d53_params), P2_R, P2_G);
}

inline float process(State& state, const Impedances& impedances, float Vs) {
    const auto Vs_b = Vs; // Vs reflected
    const auto r163_b = 0; // r163 reflected
    const auto c40_b = impedances.c40_b_coeff * state.c40_b1 + impedances.c40_a_coeff * state.c40_z; // c40 reflected
    state.c40_b1 = c40_b; // c40 state update
    const auto P1_b = r163_b - impedances.P1_pr * (r163_b - c40_b); // P1 reflected
    const auto S1_b = -(P1_b + Vs_b); // S1 reflected
    const auto r162_b = 0; // r162 reflected
    const auto I1_b = -r162_b; // I1 reflected
    const auto P2_b = S1_b - impedances.P2_pr * (S1_b - I1_b); // P2 reflected
    const auto P2_a = wdf_lib::diode::root_compute(&(impedances.d53_vars), P2_b); // P2 incident
    const auto I1_a = P2_b - I1_b + P2_a; // I1 incident
    const auto S1_a = P2_b - S1_b + P2_a; // S1 incident
    const auto r162_a = -I1_a; // r162 incident
    const auto P1_a = P1_b - impedances.S1_pr * (S1_a - S1_b); // P1 incident
    const auto Vs_a = -S1_a - P1_b + impedances.S1_pr * (S1_a - S1_b); // Vs incident
    const auto c40_a = P1_b - c40_b + P1_a; // c40 incident
    const auto r163_a = P1_b - r163_b + P1_a; // r163 incident
    state.c40_z = c40_a; // c40 state update
    
    const auto v_r162 = (r162_a + r162_b) * (float) 0.5; // r162 voltage
    return v_r162;
}

