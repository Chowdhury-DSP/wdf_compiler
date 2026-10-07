// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler bassman_tone_stack.net bassman_tone_stack.h -netlist

#pragma once

#include <tuple>
#include "R14_rtype.h"

struct Params {
    float V1_res_value = 1.0e-6;
    float R3m_value;
    static constexpr float R4_value = 5.6e+04;
    static constexpr float C2_value = 2.0e-08;
    static constexpr float C3_value = 2.0e-08;
    float R1m_value;
    static constexpr float C1_value = 2.5e-10;
    float R1p_value;
    float R2_value;
    float R3p_value;
    struct rtype_R14::R_Params R14_params;
};

struct Impedances {
    float S11_pr;
    float S12_pr;
    float S10_pr;
    struct rtype_R14::R_Vars R14_vars;
};

struct State {
    float C2_z;
    float C3_z;
    float C1_z;
};

inline void calc_impedances(Impedances& impedances, float fs, Params params = {}) {
    [[maybe_unused]] static constexpr auto sum = [] (auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [] (auto a, auto b) { return (a * b) / (a + b); };
    
    [[maybe_unused]] const auto T = 1.0 / fs;
    
    // Computing impedance for: V1;
    [[maybe_unused]] const auto V1_res_value = params.V1_res_value;
    [[maybe_unused]] const auto V1_R = V1_res_value;
    [[maybe_unused]] const auto V1_G = 1.0 / V1_R;
    // Computing impedance for: R3m;
    [[maybe_unused]] const auto R3m_value = params.R3m_value;
    [[maybe_unused]] const auto R3m_R = R3m_value;
    [[maybe_unused]] const auto R3m_G = 1.0 / R3m_R;
    // Computing impedance for: R4;
    [[maybe_unused]] const auto R4_value = params.R4_value;
    [[maybe_unused]] const auto R4_R = R4_value;
    [[maybe_unused]] const auto R4_G = 1.0 / R4_R;
    // Computing impedance for: C2;
    [[maybe_unused]] const auto C2_value = params.C2_value;
    [[maybe_unused]] const auto C2_G = 2.0 * C2_value * fs;
    [[maybe_unused]] const auto C2_R = 1.0 / C2_G;
    // Computing impedance for: C3;
    [[maybe_unused]] const auto C3_value = params.C3_value;
    [[maybe_unused]] const auto C3_G = 2.0 * C3_value * fs;
    [[maybe_unused]] const auto C3_R = 1.0 / C3_G;
    // Computing impedance for: R1m;
    [[maybe_unused]] const auto R1m_value = params.R1m_value;
    [[maybe_unused]] const auto R1m_R = R1m_value;
    [[maybe_unused]] const auto R1m_G = 1.0 / R1m_R;
    // Computing impedance for: C1;
    [[maybe_unused]] const auto C1_value = params.C1_value;
    [[maybe_unused]] const auto C1_G = 2.0 * C1_value * fs;
    [[maybe_unused]] const auto C1_R = 1.0 / C1_G;
    // Computing impedance for: R1p;
    [[maybe_unused]] const auto R1p_value = params.R1p_value;
    [[maybe_unused]] const auto R1p_R = R1p_value;
    [[maybe_unused]] const auto R1p_G = 1.0 / R1p_R;
    // Computing impedance for: S13;
    [[maybe_unused]] const auto S13_value = sum(params.R2_value, params.R3p_value);
    [[maybe_unused]] const auto S13_R = S13_value;
    [[maybe_unused]] const auto S13_G = 1.0 / S13_R;
    // Computing impedance for: S11;
    [[maybe_unused]] const auto S11_R = C1_R + R1p_R;
    [[maybe_unused]] const auto S11_G = 1.0 / S11_R;
    impedances.S11_pr = C1_R * S11_G;

    // Computing impedance for: S12;
    [[maybe_unused]] const auto S12_R = R1m_R + S11_R;
    [[maybe_unused]] const auto S12_G = 1.0 / S12_R;
    impedances.S12_pr = R1m_R * S12_G;

    // Computing impedance for: S10;
    [[maybe_unused]] const auto S10_R = V1_R + R3m_R;
    [[maybe_unused]] const auto S10_G = 1.0 / S10_R;
    impedances.S10_pr = V1_R * S10_G;

    // Computing impedance for: R14;
    rtype_R14::update_vars(&(impedances.R14_vars), &(params.R14_params), S10_R, S10_G, R4_R, R4_G, C2_R, C2_G, C3_R, C3_G, S12_R, S12_G, S13_R, S13_G);
}

inline std::tuple<float, float, float, float> process(State& state, const Impedances& impedances, float V1) {
    const auto S13_b = 0; // S13 reflected
    const auto R1p_b = 0; // R1p reflected
    const auto C1_b = state.C1_z; // C1 reflected
    const auto S11_b = -(C1_b + R1p_b); // S11 reflected
    const auto R1m_b = 0; // R1m reflected
    const auto S12_b = -(R1m_b + S11_b); // S12 reflected
    const auto C3_b = state.C3_z; // C3 reflected
    const auto C2_b = state.C2_z; // C2 reflected
    const auto R4_b = 0; // R4 reflected
    const auto R3m_b = 0; // R3m reflected
    const auto V1_b = V1; // V1 reflected
    const auto S10_b = -(V1_b + R3m_b); // S10 reflected
    alignas (16) const float R14_ins[6] = { S10_b, R4_b, C2_b, C3_b, S12_b, S13_b }; // R14 inputs
    alignas (16) float R14_outs[6]; // R14 outputs 
    rtype_R14::compute(&impedances.R14_vars, R14_ins, R14_outs); // R14 compute
    const float S10_a = R14_outs[0];
    const float R4_a = R14_outs[1];
    const float C2_a = R14_outs[2];
    const float C3_a = R14_outs[3];
    const float S12_a = R14_outs[4];
    const float S13_a = R14_outs[5];
    const auto V1_a = V1_b - impedances.S10_pr * (S10_a - S10_b); // V1 incident
    const auto R3m_a = -S10_a - V1_b + impedances.S10_pr * (S10_a - S10_b); // R3m incident
    state.C2_z = C2_a; // C2 state update
    state.C3_z = C3_a; // C3 state update
    const auto R1m_a = R1m_b - impedances.S12_pr * (S12_a - S12_b); // R1m incident
    const auto S11_a = -S12_a - R1m_b + impedances.S12_pr * (S12_a - S12_b); // S11 incident
    const auto C1_a = C1_b - impedances.S11_pr * (S11_a - S11_b); // C1 incident
    const auto R1p_a = -S11_a - C1_b + impedances.S11_pr * (S11_a - S11_b); // R1p incident
    state.C1_z = C1_a; // C1 state update
    
    const auto v_C1 = (C1_a + C1_b) * (float) 0.5; // C1 voltage
    const auto v_R1p = (R1p_a + R1p_b) * (float) 0.5; // R1p voltage
    const auto v_R1m = (R1m_a + R1m_b) * (float) 0.5; // R1m voltage
    const auto v_R3m = (R3m_a + R3m_b) * (float) 0.5; // R3m voltage
    return std::make_tuple(v_C1, v_R1p, v_R1m, v_R3m);
}

