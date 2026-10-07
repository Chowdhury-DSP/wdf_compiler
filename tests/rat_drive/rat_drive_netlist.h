// Auto-generated with wdf_compiler version 1.1.0.c175981.
// Command: wdf_compiler rat_drive.net rat_drive.h -netlist

#pragma once

#include <wdf_lib_diode.h>
#include "R26_rtype.h"

struct Params {
    static constexpr float R6_value = 1.0e+03;
    static constexpr float C7_value = 4.7e-06;
    static constexpr float R3_value = 1.0e+03;
    float S16_v_value;
    float Vplus_res_value = 1.0e-6;
    static constexpr float R2_value = 1.0e+06;
    static constexpr float C1_value = 2.2e-08;
    static constexpr float C2_value = 1.0e-09;
    static constexpr float R4_value = 4.7e+01;
    static constexpr float C5_value = 2.2e-06;
    static constexpr float R5_value = 5.6e+02;
    static constexpr float C6_value = 4.7e-06;
    static constexpr float Rd_value = 5.0e+04;
    static constexpr float C4_value = 1.0e-10;
    struct rtype_R26::R_Params R26_params;
    struct wdf_lib::diode::Diode_Params DP_params;
};

struct Impedances {
    float S24_2RC_over_T_plus_2RC;
    float S16_V;
    float S21_2RC_over_T_plus_2RC;
    float S22_2RC_over_T_plus_2RC;
    float P15_2RC_over_2RC_plus_T;
    float P18_pr;
    float S19_pr;
    float P20_pr;
    float P23_pr;
    struct rtype_R26::R_Vars R26_vars;
    float S25_pr;
    struct wdf_lib::diode::Diode_Vars DP_vars;
};

struct State {
    float S24_z;
    float S17_z;
    float S17_v1;
    float C2_z;
    float S21_z;
    float S22_z;
    float P15_z;
};

inline void calc_impedances(Impedances& impedances, float fs, Params params = {}) {
    [[maybe_unused]] static constexpr auto sum = [] (auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [] (auto a, auto b) { return (a * b) / (a + b); };
    
    [[maybe_unused]] const auto T = 1.0 / fs;
    
    // Computing impedance for: S24;
    [[maybe_unused]] const auto S24_res_value = params.R6_value;
    [[maybe_unused]] const auto S24_cap_value = params.C7_value;
    [[maybe_unused]] const auto S24_R = (T / (2.0 * S24_cap_value)) + S24_res_value;
    [[maybe_unused]] const auto S24_G = 1.0 / S24_R;
    impedances.S24_2RC_over_T_plus_2RC = (2.0 * S24_cap_value * S24_res_value) / (2.0 * S24_cap_value * S24_res_value + T);

    // Computing impedance for: R3;
    [[maybe_unused]] const auto R3_value = params.R3_value;
    [[maybe_unused]] const auto R3_R = R3_value;
    [[maybe_unused]] const auto R3_G = 1.0 / R3_R;
    // Computing voltage for: S16;
    impedances.S16_V = params.S16_v_value;

    // Computing impedance for: S16;
    [[maybe_unused]] const auto S16_res_value = sum(params.Vplus_res_value, params.R2_value);
    [[maybe_unused]] const auto S16_R = S16_res_value;
    [[maybe_unused]] const auto S16_G = 1.0 / S16_R;
    // Computing impedance for: S17;
    [[maybe_unused]] const auto S17_cap_value = params.C1_value;
    [[maybe_unused]] const auto S17_G = 2.0 * S17_cap_value * fs;
    [[maybe_unused]] const auto S17_R = 1.0 / S17_G;
    // Computing impedance for: C2;
    [[maybe_unused]] const auto C2_value = params.C2_value;
    [[maybe_unused]] const auto C2_G = 2.0 * C2_value * fs;
    [[maybe_unused]] const auto C2_R = 1.0 / C2_G;
    // Computing impedance for: S21;
    [[maybe_unused]] const auto S21_res_value = params.R4_value;
    [[maybe_unused]] const auto S21_cap_value = params.C5_value;
    [[maybe_unused]] const auto S21_R = (T / (2.0 * S21_cap_value)) + S21_res_value;
    [[maybe_unused]] const auto S21_G = 1.0 / S21_R;
    impedances.S21_2RC_over_T_plus_2RC = (2.0 * S21_cap_value * S21_res_value) / (2.0 * S21_cap_value * S21_res_value + T);

    // Computing impedance for: S22;
    [[maybe_unused]] const auto S22_res_value = params.R5_value;
    [[maybe_unused]] const auto S22_cap_value = params.C6_value;
    [[maybe_unused]] const auto S22_R = (T / (2.0 * S22_cap_value)) + S22_res_value;
    [[maybe_unused]] const auto S22_G = 1.0 / S22_R;
    impedances.S22_2RC_over_T_plus_2RC = (2.0 * S22_cap_value * S22_res_value) / (2.0 * S22_cap_value * S22_res_value + T);

    // Computing impedance for: P15;
    [[maybe_unused]] const auto P15_res_value = params.Rd_value;
    [[maybe_unused]] const auto P15_cap_value = params.C4_value;
    [[maybe_unused]] const auto P15_2RC = 2.0 * P15_cap_value * P15_res_value;
    [[maybe_unused]] const auto P15_R = P15_res_value * T / (P15_2RC + T);
    [[maybe_unused]] const auto P15_G = 1.0 / P15_R;
    impedances.P15_2RC_over_2RC_plus_T = P15_2RC / (P15_2RC + T);

    // Computing impedance for: P18;
    [[maybe_unused]] const auto P18_G = S16_G + S17_G;
    [[maybe_unused]] const auto P18_R = 1.0 / P18_G;
    impedances.P18_pr = S16_G * P18_R;

    // Computing impedance for: S19;
    [[maybe_unused]] const auto S19_R = R3_R + P18_R;
    [[maybe_unused]] const auto S19_G = 1.0 / S19_R;
    impedances.S19_pr = R3_R * S19_G;

    // Computing impedance for: P20;
    [[maybe_unused]] const auto P20_G = S19_G + C2_G;
    [[maybe_unused]] const auto P20_R = 1.0 / P20_G;
    impedances.P20_pr = S19_G * P20_R;

    // Computing impedance for: P23;
    [[maybe_unused]] const auto P23_G = S21_G + S22_G;
    [[maybe_unused]] const auto P23_R = 1.0 / P23_G;
    impedances.P23_pr = S21_G * P23_R;

    // Computing impedance for: R26;
    [[maybe_unused]] const auto R26_R_calc = rtype_R26::update_vars(&(impedances.R26_vars), &(params.R26_params), P20_R, P20_G, P23_R, P23_G, P15_R, P15_G);
    [[maybe_unused]] const auto R26_R = R26_R_calc;
    [[maybe_unused]] const auto R26_G = 1.0 / R26_R;
    // Computing impedance for: S25;
    [[maybe_unused]] const auto S25_R = S24_R + R26_R;
    [[maybe_unused]] const auto S25_G = 1.0 / S25_R;
    impedances.S25_pr = S24_R * S25_G;

    // Computing impedance for: DP;
    wdf_lib::diode::update_vars(&(impedances.DP_vars), &(params.DP_params), S25_R, S25_G);
}

inline float process(State& state, const Impedances& impedances, float S17) {
    const auto P15_b = impedances.P15_2RC_over_2RC_plus_T * state.P15_z; // P15 reflected
    const auto S22_b = -state.S22_z; // S22 reflected
    const auto S21_b = -state.S21_z; // S21 reflected
    const auto P23_b = S22_b - impedances.P23_pr * (S22_b - S21_b); // P23 reflected
    const auto C2_b = state.C2_z; // C2 reflected
    const auto S17_b = state.S17_z + S17 - state.S17_v1; // S17 reflected
    state.S17_v1 = S17; // S17 state update
    const auto S16_b = -(impedances.S16_V); // S16 reflected
    const auto P18_b = S17_b - impedances.P18_pr * (S17_b - S16_b); // P18 reflected
    const auto R3_b = 0; // R3 reflected
    const auto S19_b = -(R3_b + P18_b); // S19 reflected
    const auto P20_b = C2_b - impedances.P20_pr * (C2_b - S19_b); // P20 reflected
    alignas (16) const float R26_ins[3] = { P20_b, P23_b, P15_b }; // R26 inputs
    const float R26_b = rtype_R26::reflected(&(impedances.R26_vars), R26_ins); // R26 reflected
    const auto S24_b = -state.S24_z; // S24 reflected
    const auto S25_b = -(S24_b + R26_b); // S25 reflected
    const auto S25_a = wdf_lib::diode::root_compute(&(impedances.DP_vars), S25_b); // S25 incident
    const auto S24_a = S24_b - impedances.S25_pr * (S25_a - S25_b); // S24 incident
    const auto R26_a = -S25_a - S24_b + impedances.S25_pr * (S25_a - S25_b); // R26 incident
    state.S24_z = -S24_a + impedances.S24_2RC_over_T_plus_2RC * (S24_a + state.S24_z); // S24 state update
    alignas (16) float R26_outs[3]; // 3 outputs 
    rtype_R26::incident(&(impedances.R26_vars), R26_a, R26_ins, R26_outs); // R26 compute
    const float P20_a = R26_outs[0];
    const float P23_a = R26_outs[1];
    const float P15_a = R26_outs[2];
    const auto S19_a = P20_b - S19_b + P20_a; // S19 incident
    const auto C2_a = P20_b - C2_b + P20_a; // C2 incident
    const auto R3_a = R3_b - impedances.S19_pr * (S19_a - S19_b); // R3 incident
    const auto P18_a = -S19_a - R3_b + impedances.S19_pr * (S19_a - S19_b); // P18 incident
    const auto S16_a = P18_b - S16_b + P18_a; // S16 incident
    const auto S17_a = P18_b - S17_b + P18_a; // S17 incident
    state.S17_z = S17_a; // S17 state update
    state.C2_z = C2_a; // C2 state update
    const auto S21_a = P23_b - S21_b + P23_a; // S21 incident
    const auto S22_a = P23_b - S22_b + P23_a; // S22 incident
    state.S21_z = -S21_a + impedances.S21_2RC_over_T_plus_2RC * (S21_a + state.S21_z); // S21 state update
    state.S22_z = -S22_a + impedances.S22_2RC_over_T_plus_2RC * (S22_a + state.S22_z); // S22 state update
    state.P15_z = P15_b + P15_a - state.P15_z; // P15 state update
    
    const auto v_S25 = (S25_a + S25_b) * (float) 0.5; // S25 voltage
    return v_S25;
}

