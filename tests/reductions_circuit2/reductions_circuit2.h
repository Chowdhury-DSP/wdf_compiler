// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler reductions_circuit2.wdf reductions_circuit2.h

#pragma once

struct Params {
    static constexpr float Vb_v_value = 1.5e+00;
    static constexpr float Rl1_value = 1.0e+03;
    static constexpr float L1_value = 2.0e-01;
    static constexpr float Rl2_value = 1.0e+04;
    static constexpr float Rp_value = 4.0e+03;
    static constexpr float Vcc_v_value = 1.2e+00;
    float Vcc_cap_value;
    static constexpr float C1_value = 1.0e-08;
    static constexpr float Cp2_value = 7.0e-09;
    static constexpr float Vin_res_value = 5.0e+03;
    static constexpr float R1_value = 1.0e+04;
    float R2_value;
    static constexpr float Cp_value = 4.0e-06;
    static constexpr float Rp2_value = 7.0e+03;
    static constexpr float Rl_v_value = -1.1e+00;
    static constexpr float Rl_res_value = 1.0e+05;
    static constexpr float Rl_cap_value = 1.0e-05;
};

struct Impedances {
    static constexpr float Vb_V = Params::Vb_v_value;
    float Sr_Ri;
    static constexpr float S4_V = Params::Vcc_v_value;
    float S8_T_over_2RC;
    static constexpr float Rl_V = Params::Rl_v_value;
    float Rl_T_over_2RC;
    float S7_pr;
    float S6_pr;
    float S3_pr;
    float Sl_pr;
    float Pr_pr;
};

struct State {
    float Sr_z;
    float S4_z;
    float S4_v1;
    float Cp2_z;
    float S8_z;
    float Rl_z;
};

inline void calc_impedances(Impedances& impedances, float fs, Params params = {}) {
    [[maybe_unused]] static constexpr auto sum = [] (auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [] (auto a, auto b) { return (a * b) / (a + b); };
    
    [[maybe_unused]] const auto T = 1.0 / fs;
    
    // Computing impedance for: Sr;
    [[maybe_unused]] const auto Sr_res_value = sum(params.Rl1_value, params.Rl2_value);
    [[maybe_unused]] const auto Sr_ind_value = params.L1_value;
    [[maybe_unused]] const auto Sr_R = 2.0 * Sr_ind_value * fs + Sr_res_value;
    [[maybe_unused]] const auto Sr_G = 1.0 / Sr_R;
    impedances.Sr_Ri = Sr_res_value * Sr_G;

    // Computing impedance for: Rp;
    [[maybe_unused]] const auto Rp_value = params.Rp_value;
    [[maybe_unused]] const auto Rp_R = Rp_value;
    [[maybe_unused]] const auto Rp_G = 1.0 / Rp_R;
    // Computing impedance for: S4;
    [[maybe_unused]] const auto S4_cap_value = recip_sum(params.Vcc_cap_value, params.C1_value);
    [[maybe_unused]] const auto S4_G = 2.0 * S4_cap_value * fs;
    [[maybe_unused]] const auto S4_R = 1.0 / S4_G;
    // Computing impedance for: Cp2;
    [[maybe_unused]] const auto Cp2_value = params.Cp2_value;
    [[maybe_unused]] const auto Cp2_G = 2.0 * Cp2_value * fs;
    [[maybe_unused]] const auto Cp2_R = 1.0 / Cp2_G;
    // Computing impedance for: S8;
    [[maybe_unused]] const auto S8_res_value = sum(sum(sum(params.Vin_res_value, params.R1_value), params.R2_value), params.Rp2_value);
    [[maybe_unused]] const auto S8_cap_value = params.Cp_value;
    [[maybe_unused]] const auto S8_R = (T / (2.0 * S8_cap_value)) + S8_res_value;
    [[maybe_unused]] const auto S8_G = 1.0 / S8_R;
    impedances.S8_T_over_2RC = T / (2.0 * S8_cap_value * S8_res_value);

    // Computing impedance for: Rl;
    [[maybe_unused]] const auto Rl_res_value = params.Rl_res_value;
    [[maybe_unused]] const auto Rl_cap_value = params.Rl_cap_value;
    [[maybe_unused]] const auto Rl_R = (T / (2.0 * Rl_cap_value)) + Rl_res_value;
    [[maybe_unused]] const auto Rl_G = 1.0 / Rl_R;
    impedances.Rl_T_over_2RC = T / (2.0 * Rl_cap_value * Rl_res_value);

    // Computing impedance for: S7;
    [[maybe_unused]] const auto S7_R = Cp2_R + S8_R;
    [[maybe_unused]] const auto S7_G = 1.0 / S7_R;
    impedances.S7_pr = Cp2_R * S7_G;

    // Computing impedance for: S6;
    [[maybe_unused]] const auto S6_R = Rp_R + S4_R;
    [[maybe_unused]] const auto S6_G = 1.0 / S6_R;
    impedances.S6_pr = Rp_R * S6_G;

    // Computing impedance for: S3;
    [[maybe_unused]] const auto S3_R = S6_R + S7_R;
    [[maybe_unused]] const auto S3_G = 1.0 / S3_R;
    impedances.S3_pr = S6_R * S3_G;

    // Computing impedance for: Sl;
    [[maybe_unused]] const auto Sl_R = S3_R + Rl_R;
    [[maybe_unused]] const auto Sl_G = 1.0 / Sl_R;
    impedances.Sl_pr = S3_R * Sl_G;

    // Computing impedance for: Pr;
    [[maybe_unused]] const auto Pr_R = Sr_R + Sl_R;
    [[maybe_unused]] const auto Pr_G = 1.0 / Pr_R;
    impedances.Pr_pr = Sr_R * Pr_G;

}

inline float process(State& state, const Impedances& impedances, float S8) {
    const auto Rl_b = -(state.Rl_z + impedances.Rl_V); // Rl reflected
    const auto S8_b = -(state.S8_z + -(S8)); // S8 reflected
    const auto Cp2_b = state.Cp2_z; // Cp2 reflected
    const auto S7_b = -(Cp2_b + S8_b); // S7 reflected
    const auto S4_b = state.S4_z + -(impedances.S4_V) - state.S4_v1; // S4 reflected
    state.S4_v1 = -(impedances.S4_V); // S4 state update
    const auto Rp_b = 0; // Rp reflected
    const auto S6_b = -(Rp_b + S4_b); // S6 reflected
    const auto S3_b = -(S6_b + S7_b); // S3 reflected
    const auto Sl_b = -(S3_b + Rl_b); // Sl reflected
    const auto Sr_b = state.Sr_z; // Sr reflected
    const auto Pr_b = -(Sr_b + Sl_b); // Pr reflected
    const auto Vb_a = Pr_b; // Vb incident
    const auto Vb_b = -Vb_a + 2.0 * impedances.Vb_V; // Vb reflected
    const auto Pr_a = Vb_b; // Pr incident
    const auto Sr_a = Sr_b - impedances.Pr_pr * (Pr_a - Pr_b); // Sr incident
    const auto Sl_a = -Pr_a - Sr_b + impedances.Pr_pr * (Pr_a - Pr_b); // Sl incident
    state.Sr_z = -Sr_a + impedances.Sr_Ri * (Sr_a - state.Sr_z); // Sr state update
    const auto S3_a = S3_b - impedances.Sl_pr * (Sl_a - Sl_b); // S3 incident
    const auto Rl_a = -Sl_a - S3_b + impedances.Sl_pr * (Sl_a - Sl_b); // Rl incident
    const auto S6_a = S6_b - impedances.S3_pr * (S3_a - S3_b); // S6 incident
    const auto S7_a = -S3_a - S6_b + impedances.S3_pr * (S3_a - S3_b); // S7 incident
    const auto Rp_a = Rp_b - impedances.S6_pr * (S6_a - S6_b); // Rp incident
    const auto S4_a = -S6_a - Rp_b + impedances.S6_pr * (S6_a - S6_b); // S4 incident
    state.S4_z = S4_a; // S4 state update
    const auto Cp2_a = Cp2_b - impedances.S7_pr * (S7_a - S7_b); // Cp2 incident
    const auto S8_a = -S7_a - Cp2_b + impedances.S7_pr * (S7_a - S7_b); // S8 incident
    state.Cp2_z = Cp2_a; // Cp2 state update
    state.S8_z -= impedances.S8_T_over_2RC * (S8_a - S8_b); // S8 state update
    state.Rl_z -= impedances.Rl_T_over_2RC * (Rl_a - Rl_b); // Rl state update
    
    const auto v_Rl = (Rl_a + Rl_b) * (float) 0.5; // Rl voltage
    return v_Rl;
}

