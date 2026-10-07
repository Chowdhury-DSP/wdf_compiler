// Auto-generated with wdf_compiler version 1.1.0.49a5cfa.
// Command: wdf_compiler rat_drive.wdf rat_drive_wdf_test.h

#pragma once

#include "custom_rat_drive_rtype.h"
#include <wdf_lib_diode_pair.h>

struct Params {
    static constexpr float R6_value = 1.0e+03;
    static constexpr float C7_value = 4.7e-06;
    static constexpr float C2_value = 1.0e-09;
    static constexpr float R3_value = 1.0e+03;
    static constexpr float Vplus_R2_v_value = 4.5e+00;
    static constexpr float Vplus_R2_res_value = 1.0e+06;
    static constexpr float Vin_C1_cap_value = 2.2e-08;
    static constexpr float R4_value = 4.7e+01;
    static constexpr float C5_value = 2.2e-06;
    static constexpr float R5_value = 5.6e+02;
    static constexpr float C6_value = 4.7e-06;
    float Rd_value;
    static constexpr float C4_value = 1.0e-10;
    struct rat_drive_rtype::R_Params R_params;
    struct wdf_lib::diode_pair::Diode_Pair_Params DP_params;
};

struct Impedances {
    float S_R6_C7_2RC_over_T_plus_2RC;
    static constexpr float Vplus_R2_V = Params::Vplus_R2_v_value;
    float S_R4_C5_2RC_over_T_plus_2RC;
    float S_R5_C6_2RC_over_T_plus_2RC;
    float P_Rd_C4_2RC_over_2RC_plus_T;
    float P1_pr;
    float S2_pr;
    float Pb_pr;
    float Pa_pr;
    struct rat_drive_rtype::R_Vars R_vars;
    float Sd_pr;
    struct wdf_lib::diode_pair::Diode_Pair_Vars DP_vars;
};

struct State {
    float S_R6_C7_z;
    float C2_z;
    float Vin_C1_z;
    float Vin_C1_v1;
    float S_R4_C5_z;
    float S_R5_C6_z;
    float P_Rd_C4_z;
};

inline void calc_impedances(Impedances& impedances, float fs, Params params = {}) {
    [[maybe_unused]] static constexpr auto sum = [] (auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [] (auto a, auto b) { return (a * b) / (a + b); };
    
    [[maybe_unused]] const auto T = 1.0 / fs;
    
    // Computing impedance for: S_R6_C7;
    [[maybe_unused]] const auto S_R6_C7_res_value = params.R6_value;
    [[maybe_unused]] const auto S_R6_C7_cap_value = params.C7_value;
    [[maybe_unused]] const auto S_R6_C7_R = (T / (2.0 * S_R6_C7_cap_value)) + S_R6_C7_res_value;
    [[maybe_unused]] const auto S_R6_C7_G = 1.0 / S_R6_C7_R;
    impedances.S_R6_C7_2RC_over_T_plus_2RC = (2.0 * S_R6_C7_cap_value * S_R6_C7_res_value) / (2.0 * S_R6_C7_cap_value * S_R6_C7_res_value + T);

    // Computing impedance for: C2;
    [[maybe_unused]] const auto C2_value = params.C2_value;
    [[maybe_unused]] const auto C2_G = 2.0 * C2_value * fs;
    [[maybe_unused]] const auto C2_R = 1.0 / C2_G;
    // Computing impedance for: R3;
    [[maybe_unused]] const auto R3_value = params.R3_value;
    [[maybe_unused]] const auto R3_R = R3_value;
    [[maybe_unused]] const auto R3_G = 1.0 / R3_R;
    // Computing impedance for: Vplus_R2;
    [[maybe_unused]] const auto Vplus_R2_res_value = params.Vplus_R2_res_value;
    [[maybe_unused]] const auto Vplus_R2_R = Vplus_R2_res_value;
    [[maybe_unused]] const auto Vplus_R2_G = 1.0 / Vplus_R2_R;
    // Computing impedance for: Vin_C1;
    [[maybe_unused]] const auto Vin_C1_cap_value = params.Vin_C1_cap_value;
    [[maybe_unused]] const auto Vin_C1_G = 2.0 * Vin_C1_cap_value * fs;
    [[maybe_unused]] const auto Vin_C1_R = 1.0 / Vin_C1_G;
    // Computing impedance for: S_R4_C5;
    [[maybe_unused]] const auto S_R4_C5_res_value = params.R4_value;
    [[maybe_unused]] const auto S_R4_C5_cap_value = params.C5_value;
    [[maybe_unused]] const auto S_R4_C5_R = (T / (2.0 * S_R4_C5_cap_value)) + S_R4_C5_res_value;
    [[maybe_unused]] const auto S_R4_C5_G = 1.0 / S_R4_C5_R;
    impedances.S_R4_C5_2RC_over_T_plus_2RC = (2.0 * S_R4_C5_cap_value * S_R4_C5_res_value) / (2.0 * S_R4_C5_cap_value * S_R4_C5_res_value + T);

    // Computing impedance for: S_R5_C6;
    [[maybe_unused]] const auto S_R5_C6_res_value = params.R5_value;
    [[maybe_unused]] const auto S_R5_C6_cap_value = params.C6_value;
    [[maybe_unused]] const auto S_R5_C6_R = (T / (2.0 * S_R5_C6_cap_value)) + S_R5_C6_res_value;
    [[maybe_unused]] const auto S_R5_C6_G = 1.0 / S_R5_C6_R;
    impedances.S_R5_C6_2RC_over_T_plus_2RC = (2.0 * S_R5_C6_cap_value * S_R5_C6_res_value) / (2.0 * S_R5_C6_cap_value * S_R5_C6_res_value + T);

    // Computing impedance for: P_Rd_C4;
    [[maybe_unused]] const auto P_Rd_C4_res_value = params.Rd_value;
    [[maybe_unused]] const auto P_Rd_C4_cap_value = params.C4_value;
    [[maybe_unused]] const auto P_Rd_C4_2RC = 2.0 * P_Rd_C4_cap_value * P_Rd_C4_res_value;
    [[maybe_unused]] const auto P_Rd_C4_R = P_Rd_C4_res_value * T / (P_Rd_C4_2RC + T);
    [[maybe_unused]] const auto P_Rd_C4_G = 1.0 / P_Rd_C4_R;
    impedances.P_Rd_C4_2RC_over_2RC_plus_T = P_Rd_C4_2RC / (P_Rd_C4_2RC + T);

    // Computing impedance for: P1;
    [[maybe_unused]] const auto P1_G = Vplus_R2_G + Vin_C1_G;
    [[maybe_unused]] const auto P1_R = 1.0 / P1_G;
    impedances.P1_pr = Vplus_R2_G * P1_R;

    // Computing impedance for: S2;
    [[maybe_unused]] const auto S2_R = R3_R + P1_R;
    [[maybe_unused]] const auto S2_G = 1.0 / S2_R;
    impedances.S2_pr = R3_R * S2_G;

    // Computing impedance for: Pb;
    [[maybe_unused]] const auto Pb_G = S_R4_C5_G + S_R5_C6_G;
    [[maybe_unused]] const auto Pb_R = 1.0 / Pb_G;
    impedances.Pb_pr = S_R4_C5_G * Pb_R;

    // Computing impedance for: Pa;
    [[maybe_unused]] const auto Pa_G = C2_G + S2_G;
    [[maybe_unused]] const auto Pa_R = 1.0 / Pa_G;
    impedances.Pa_pr = C2_G * Pa_R;

    // Computing impedance for: R;
    [[maybe_unused]] const auto R_R_calc = rat_drive_rtype::update_vars(&(impedances.R_vars), &(params.R_params), Pa_R, Pa_G, Pb_R, Pb_G, P_Rd_C4_R, P_Rd_C4_G);
    [[maybe_unused]] const auto R_R = R_R_calc;
    [[maybe_unused]] const auto R_G = 1.0 / R_R;
    // Computing impedance for: Sd;
    [[maybe_unused]] const auto Sd_R = S_R6_C7_R + R_R;
    [[maybe_unused]] const auto Sd_G = 1.0 / Sd_R;
    impedances.Sd_pr = S_R6_C7_R * Sd_G;

    // Computing impedance for: DP;
    wdf_lib::diode_pair::update_vars(&(impedances.DP_vars), &(params.DP_params), Sd_R, Sd_G);
}

inline float process(State& state, const Impedances& impedances, float Vin_C1) {
    const auto P_Rd_C4_b = impedances.P_Rd_C4_2RC_over_2RC_plus_T * state.P_Rd_C4_z; // P_Rd_C4 reflected
    const auto S_R5_C6_b = -state.S_R5_C6_z; // S_R5_C6 reflected
    const auto S_R4_C5_b = -state.S_R4_C5_z; // S_R4_C5 reflected
    const auto Pb_b = S_R5_C6_b - impedances.Pb_pr * (S_R5_C6_b - S_R4_C5_b); // Pb reflected
    const auto Vin_C1_b = state.Vin_C1_z + Vin_C1 - state.Vin_C1_v1; // Vin_C1 reflected
    state.Vin_C1_v1 = Vin_C1; // Vin_C1 state update
    const auto Vplus_R2_b = impedances.Vplus_R2_V; // Vplus_R2 reflected
    const auto P1_b = Vin_C1_b - impedances.P1_pr * (Vin_C1_b - Vplus_R2_b); // P1 reflected
    const auto R3_b = 0; // R3 reflected
    const auto S2_b = -(R3_b + P1_b); // S2 reflected
    const auto C2_b = state.C2_z; // C2 reflected
    const auto Pa_b = S2_b - impedances.Pa_pr * (S2_b - C2_b); // Pa reflected
    alignas (16) const float R_ins[3] = { Pa_b, Pb_b, P_Rd_C4_b }; // R inputs
    const float R_b = rat_drive_rtype::reflected(&(impedances.R_vars), R_ins); // R reflected
    const auto S_R6_C7_b = -state.S_R6_C7_z; // S_R6_C7 reflected
    const auto Sd_b = -(S_R6_C7_b + R_b); // Sd reflected
    const auto Sd_a = wdf_lib::diode_pair::root_compute(&(impedances.DP_vars), Sd_b); // Sd incident
    const auto S_R6_C7_a = S_R6_C7_b - impedances.Sd_pr * (Sd_a - Sd_b); // S_R6_C7 incident
    const auto R_a = -Sd_a - S_R6_C7_b + impedances.Sd_pr * (Sd_a - Sd_b); // R incident
    state.S_R6_C7_z = -S_R6_C7_a + impedances.S_R6_C7_2RC_over_T_plus_2RC * (S_R6_C7_a + state.S_R6_C7_z); // S_R6_C7 state update
    alignas (16) float R_outs[3]; // 3 outputs 
    rat_drive_rtype::incident(&(impedances.R_vars), R_a, R_ins, R_outs); // R compute
    const float Pa_a = R_outs[0];
    const float Pb_a = R_outs[1];
    const float P_Rd_C4_a = R_outs[2];
    const auto C2_a = Pa_b - C2_b + Pa_a; // C2 incident
    const auto S2_a = Pa_b - S2_b + Pa_a; // S2 incident
    state.C2_z = C2_a; // C2 state update
    const auto R3_a = R3_b - impedances.S2_pr * (S2_a - S2_b); // R3 incident
    const auto P1_a = -S2_a - R3_b + impedances.S2_pr * (S2_a - S2_b); // P1 incident
    const auto Vplus_R2_a = P1_b - Vplus_R2_b + P1_a; // Vplus_R2 incident
    const auto Vin_C1_a = P1_b - Vin_C1_b + P1_a; // Vin_C1 incident
    state.Vin_C1_z = Vin_C1_a; // Vin_C1 state update
    const auto S_R4_C5_a = Pb_b - S_R4_C5_b + Pb_a; // S_R4_C5 incident
    const auto S_R5_C6_a = Pb_b - S_R5_C6_b + Pb_a; // S_R5_C6 incident
    state.S_R4_C5_z = -S_R4_C5_a + impedances.S_R4_C5_2RC_over_T_plus_2RC * (S_R4_C5_a + state.S_R4_C5_z); // S_R4_C5 state update
    state.S_R5_C6_z = -S_R5_C6_a + impedances.S_R5_C6_2RC_over_T_plus_2RC * (S_R5_C6_a + state.S_R5_C6_z); // S_R5_C6 state update
    state.P_Rd_C4_z = P_Rd_C4_b + P_Rd_C4_a - state.P_Rd_C4_z; // P_Rd_C4 state update
    
    const auto v_Sd = (Sd_a + Sd_b) * (float) 0.5; // Sd voltage
    return v_Sd;
}

