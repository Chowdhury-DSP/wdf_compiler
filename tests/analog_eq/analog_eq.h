// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler analog_eq.wdf analog_eq.h

#pragma once

#include "custom_analog_eq_rtype.h"
#include "custom_nonlinear_inductor.h"

namespace analog_eq
{

struct Params {
    static constexpr float Ra_value = 6.2e+02;
    float P_treble_boost_plus_value;
    float P_treble_boost_minus_value;
    float R_treble_boost_bw_value;
    float C_treble_boost_value;
    struct nl_inductor::L_Params L_treble_boost_params;
    static constexpr float Rlc_value = 1.0e+03;
    float P_low_cut_value;
    float C_low_cut_value;
    float P_treble_cut_plus_value;
    float P_treble_cut_minus_value;
    static constexpr float Rc_value = 7.5e+01;
    float C_treble_cut_value;
    float P_low_boost_value;
    float C_low_boost_value;
    static constexpr float Rb_value = 1.0e+04;
    static constexpr float Rload_value = 1.0e+06;
    struct analog_eq_r::R_Params R_params;
};

struct Impedances {
    float Pb_2RC_over_2RC_plus_T;
    float Streble_cut_2RC_over_T_plus_2RC;
    float Pbass_boost_2RC_over_2RC_plus_T;
    float Streble_boost_1_pr;
    float Streble_boost_pr;
    float Sb_pr;
    float Ptreble_boost_pr;
    float Ptreble_cut_pr;
    struct analog_eq_r::R_Vars R_vars;
    float Svin_pr;
    float SR_pr;
    float Pvin_pr;
};

struct State {
    float C_treble_boost_z;
    struct nl_inductor::L_State L_treble_boost_state;
    float Pb_z;
    float Streble_cut_z;
    float Pbass_boost_z;
};

inline void calc_impedances(Impedances& impedances, float fs, Params params = {}) {
    [[maybe_unused]] static constexpr auto sum = [] (auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [] (auto a, auto b) { return (a * b) / (a + b); };
    
    [[maybe_unused]] const auto T = 1.0 / fs;
    
    // Computing impedance for: Ra;
    [[maybe_unused]] const auto Ra_value = params.Ra_value;
    [[maybe_unused]] const auto Ra_R = Ra_value;
    [[maybe_unused]] const auto Ra_G = 1.0 / Ra_R;
    // Computing impedance for: P_treble_boost_plus;
    [[maybe_unused]] const auto P_treble_boost_plus_value = params.P_treble_boost_plus_value;
    [[maybe_unused]] const auto P_treble_boost_plus_R = P_treble_boost_plus_value;
    [[maybe_unused]] const auto P_treble_boost_plus_G = 1.0 / P_treble_boost_plus_R;
    // Computing impedance for: P_treble_boost_minus;
    [[maybe_unused]] const auto P_treble_boost_minus_value = params.P_treble_boost_minus_value;
    [[maybe_unused]] const auto P_treble_boost_minus_R = P_treble_boost_minus_value;
    [[maybe_unused]] const auto P_treble_boost_minus_G = 1.0 / P_treble_boost_minus_R;
    // Computing impedance for: R_treble_boost_bw;
    [[maybe_unused]] const auto R_treble_boost_bw_value = params.R_treble_boost_bw_value;
    [[maybe_unused]] const auto R_treble_boost_bw_R = R_treble_boost_bw_value;
    [[maybe_unused]] const auto R_treble_boost_bw_G = 1.0 / R_treble_boost_bw_R;
    // Computing impedance for: C_treble_boost;
    [[maybe_unused]] const auto C_treble_boost_value = params.C_treble_boost_value;
    [[maybe_unused]] const auto C_treble_boost_G = 2.0 * C_treble_boost_value * fs;
    [[maybe_unused]] const auto C_treble_boost_R = 1.0 / C_treble_boost_G;
    // Computing impedance for: L_treble_boost;
    [[maybe_unused]] const auto L_treble_boost_R_calc = nl_inductor::update_vars(&(params.L_treble_boost_params), fs);
    [[maybe_unused]] const auto L_treble_boost_R = L_treble_boost_R_calc;
    [[maybe_unused]] const auto L_treble_boost_G = 1.0 / L_treble_boost_R;
    // Computing impedance for: Rlc;
    [[maybe_unused]] const auto Rlc_value = params.Rlc_value;
    [[maybe_unused]] const auto Rlc_R = Rlc_value;
    [[maybe_unused]] const auto Rlc_G = 1.0 / Rlc_R;
    // Computing impedance for: Pb;
    [[maybe_unused]] const auto Pb_res_value = params.P_low_cut_value;
    [[maybe_unused]] const auto Pb_cap_value = params.C_low_cut_value;
    [[maybe_unused]] const auto Pb_2RC = 2.0 * Pb_cap_value * Pb_res_value;
    [[maybe_unused]] const auto Pb_R = Pb_res_value * T / (Pb_2RC + T);
    [[maybe_unused]] const auto Pb_G = 1.0 / Pb_R;
    impedances.Pb_2RC_over_2RC_plus_T = Pb_2RC / (Pb_2RC + T);

    // Computing impedance for: P_treble_cut_plus;
    [[maybe_unused]] const auto P_treble_cut_plus_value = params.P_treble_cut_plus_value;
    [[maybe_unused]] const auto P_treble_cut_plus_R = P_treble_cut_plus_value;
    [[maybe_unused]] const auto P_treble_cut_plus_G = 1.0 / P_treble_cut_plus_R;
    // Computing impedance for: P_treble_cut_minus;
    [[maybe_unused]] const auto P_treble_cut_minus_value = params.P_treble_cut_minus_value;
    [[maybe_unused]] const auto P_treble_cut_minus_R = P_treble_cut_minus_value;
    [[maybe_unused]] const auto P_treble_cut_minus_G = 1.0 / P_treble_cut_minus_R;
    // Computing impedance for: Streble_cut;
    [[maybe_unused]] const auto Streble_cut_res_value = params.Rc_value;
    [[maybe_unused]] const auto Streble_cut_cap_value = params.C_treble_cut_value;
    [[maybe_unused]] const auto Streble_cut_R = (T / (2.0 * Streble_cut_cap_value)) + Streble_cut_res_value;
    [[maybe_unused]] const auto Streble_cut_G = 1.0 / Streble_cut_R;
    impedances.Streble_cut_2RC_over_T_plus_2RC = (2.0 * Streble_cut_cap_value * Streble_cut_res_value) / (2.0 * Streble_cut_cap_value * Streble_cut_res_value + T);

    // Computing impedance for: Pbass_boost;
    [[maybe_unused]] const auto Pbass_boost_res_value = params.P_low_boost_value;
    [[maybe_unused]] const auto Pbass_boost_cap_value = params.C_low_boost_value;
    [[maybe_unused]] const auto Pbass_boost_2RC = 2.0 * Pbass_boost_cap_value * Pbass_boost_res_value;
    [[maybe_unused]] const auto Pbass_boost_R = Pbass_boost_res_value * T / (Pbass_boost_2RC + T);
    [[maybe_unused]] const auto Pbass_boost_G = 1.0 / Pbass_boost_R;
    impedances.Pbass_boost_2RC_over_2RC_plus_T = Pbass_boost_2RC / (Pbass_boost_2RC + T);

    // Computing impedance for: Rb;
    [[maybe_unused]] const auto Rb_value = params.Rb_value;
    [[maybe_unused]] const auto Rb_R = Rb_value;
    [[maybe_unused]] const auto Rb_G = 1.0 / Rb_R;
    // Computing impedance for: Rload;
    [[maybe_unused]] const auto Rload_value = params.Rload_value;
    [[maybe_unused]] const auto Rload_R = Rload_value;
    [[maybe_unused]] const auto Rload_G = 1.0 / Rload_R;
    // Computing impedance for: Streble_boost_1;
    [[maybe_unused]] const auto Streble_boost_1_R = C_treble_boost_R + L_treble_boost_R;
    [[maybe_unused]] const auto Streble_boost_1_G = 1.0 / Streble_boost_1_R;
    impedances.Streble_boost_1_pr = C_treble_boost_R * Streble_boost_1_G;

    // Computing impedance for: Streble_boost;
    [[maybe_unused]] const auto Streble_boost_R = R_treble_boost_bw_R + Streble_boost_1_R;
    [[maybe_unused]] const auto Streble_boost_G = 1.0 / Streble_boost_R;
    impedances.Streble_boost_pr = R_treble_boost_bw_R * Streble_boost_G;

    // Computing impedance for: Sb;
    [[maybe_unused]] const auto Sb_R = Rlc_R + Pb_R;
    [[maybe_unused]] const auto Sb_G = 1.0 / Sb_R;
    impedances.Sb_pr = Rlc_R * Sb_G;

    // Computing impedance for: Ptreble_boost;
    [[maybe_unused]] const auto Ptreble_boost_G = P_treble_boost_minus_G + Streble_boost_G;
    [[maybe_unused]] const auto Ptreble_boost_R = 1.0 / Ptreble_boost_G;
    impedances.Ptreble_boost_pr = P_treble_boost_minus_G * Ptreble_boost_R;

    // Computing impedance for: Ptreble_cut;
    [[maybe_unused]] const auto Ptreble_cut_G = P_treble_cut_minus_G + Streble_cut_G;
    [[maybe_unused]] const auto Ptreble_cut_R = 1.0 / Ptreble_cut_G;
    impedances.Ptreble_cut_pr = P_treble_cut_minus_G * Ptreble_cut_R;

    // Computing impedance for: R;
    [[maybe_unused]] const auto R_R_calc = analog_eq_r::update_vars(&(impedances.R_vars), &(params.R_params), Sb_R, Sb_G, P_treble_cut_plus_R, P_treble_cut_plus_G, Ptreble_cut_R, Ptreble_cut_G, Pbass_boost_R, Pbass_boost_G, Rb_R, Rb_G, Rload_R, Rload_G);
    [[maybe_unused]] const auto R_R = R_R_calc;
    [[maybe_unused]] const auto R_G = 1.0 / R_R;
    // Computing impedance for: Svin;
    [[maybe_unused]] const auto Svin_R = P_treble_boost_plus_R + Ptreble_boost_R;
    [[maybe_unused]] const auto Svin_G = 1.0 / Svin_R;
    impedances.Svin_pr = P_treble_boost_plus_R * Svin_G;

    // Computing impedance for: SR;
    [[maybe_unused]] const auto SR_R = Svin_R + R_R;
    [[maybe_unused]] const auto SR_G = 1.0 / SR_R;
    impedances.SR_pr = Svin_R * SR_G;

    // Computing impedance for: Pvin;
    [[maybe_unused]] const auto Pvin_G = Ra_G + SR_G;
    [[maybe_unused]] const auto Pvin_R = 1.0 / Pvin_G;
    impedances.Pvin_pr = Ra_G * Pvin_R;

}

inline float process(State& state, const Impedances& impedances, float Vin) {
    const auto Rload_b = 0; // Rload reflected
    const auto Rb_b = 0; // Rb reflected
    const auto Pbass_boost_b = impedances.Pbass_boost_2RC_over_2RC_plus_T * state.Pbass_boost_z; // Pbass_boost reflected
    const auto Streble_cut_b = -state.Streble_cut_z; // Streble_cut reflected
    const auto P_treble_cut_minus_b = 0; // P_treble_cut_minus reflected
    const auto Ptreble_cut_b = Streble_cut_b - impedances.Ptreble_cut_pr * (Streble_cut_b - P_treble_cut_minus_b); // Ptreble_cut reflected
    const auto P_treble_cut_plus_b = 0; // P_treble_cut_plus reflected
    const auto Pb_b = impedances.Pb_2RC_over_2RC_plus_T * state.Pb_z; // Pb reflected
    const auto Rlc_b = 0; // Rlc reflected
    const auto Sb_b = -(Rlc_b + Pb_b); // Sb reflected
    alignas (16) const float R_ins[6] = { Sb_b, P_treble_cut_plus_b, Ptreble_cut_b, Pbass_boost_b, Rb_b, Rload_b }; // R inputs
    const float R_b = analog_eq_r::reflected(&(impedances.R_vars), R_ins); // R reflected
    const float L_treble_boost_b =  nl_inductor::reflected(&(state.L_treble_boost_state)); // L_treble_boost reflected
    const auto C_treble_boost_b = state.C_treble_boost_z; // C_treble_boost reflected
    const auto Streble_boost_1_b = -(C_treble_boost_b + L_treble_boost_b); // Streble_boost_1 reflected
    const auto R_treble_boost_bw_b = 0; // R_treble_boost_bw reflected
    const auto Streble_boost_b = -(R_treble_boost_bw_b + Streble_boost_1_b); // Streble_boost reflected
    const auto P_treble_boost_minus_b = 0; // P_treble_boost_minus reflected
    const auto Ptreble_boost_b = Streble_boost_b - impedances.Ptreble_boost_pr * (Streble_boost_b - P_treble_boost_minus_b); // Ptreble_boost reflected
    const auto P_treble_boost_plus_b = 0; // P_treble_boost_plus reflected
    const auto Svin_b = -(P_treble_boost_plus_b + Ptreble_boost_b); // Svin reflected
    const auto SR_b = -(Svin_b + R_b); // SR reflected
    const auto Ra_b = 0; // Ra reflected
    const auto Pvin_b = SR_b - impedances.Pvin_pr * (SR_b - Ra_b); // Pvin reflected
    const auto Vin_a = Pvin_b; // Vin incident
    const auto Vin_b = -Vin_a + 2.0 * Vin; // Vin reflected
    const auto Pvin_a = Vin_b; // Pvin incident
    const auto Ra_a = Pvin_b - Ra_b + Pvin_a; // Ra incident
    const auto SR_a = Pvin_b - SR_b + Pvin_a; // SR incident
    const auto Svin_a = Svin_b - impedances.SR_pr * (SR_a - SR_b); // Svin incident
    const auto R_a = -SR_a - Svin_b + impedances.SR_pr * (SR_a - SR_b); // R incident
    const auto P_treble_boost_plus_a = P_treble_boost_plus_b - impedances.Svin_pr * (Svin_a - Svin_b); // P_treble_boost_plus incident
    const auto Ptreble_boost_a = -Svin_a - P_treble_boost_plus_b + impedances.Svin_pr * (Svin_a - Svin_b); // Ptreble_boost incident
    const auto P_treble_boost_minus_a = Ptreble_boost_b - P_treble_boost_minus_b + Ptreble_boost_a; // P_treble_boost_minus incident
    const auto Streble_boost_a = Ptreble_boost_b - Streble_boost_b + Ptreble_boost_a; // Streble_boost incident
    const auto R_treble_boost_bw_a = R_treble_boost_bw_b - impedances.Streble_boost_pr * (Streble_boost_a - Streble_boost_b); // R_treble_boost_bw incident
    const auto Streble_boost_1_a = -Streble_boost_a - R_treble_boost_bw_b + impedances.Streble_boost_pr * (Streble_boost_a - Streble_boost_b); // Streble_boost_1 incident
    const auto C_treble_boost_a = C_treble_boost_b - impedances.Streble_boost_1_pr * (Streble_boost_1_a - Streble_boost_1_b); // C_treble_boost incident
    const auto L_treble_boost_a = -Streble_boost_1_a - C_treble_boost_b + impedances.Streble_boost_1_pr * (Streble_boost_1_a - Streble_boost_1_b); // L_treble_boost incident
    state.C_treble_boost_z = C_treble_boost_a; // C_treble_boost state update
    nl_inductor::incident(&(state.L_treble_boost_state), L_treble_boost_a); // L_treble_boost incident
    alignas (16) float R_outs[6]; // 6 outputs 
    analog_eq_r::incident(&(impedances.R_vars), R_a, R_ins, R_outs); // R compute
    const float Sb_a = R_outs[0];
    const float P_treble_cut_plus_a = R_outs[1];
    const float Ptreble_cut_a = R_outs[2];
    const float Pbass_boost_a = R_outs[3];
    const float Rb_a = R_outs[4];
    const float Rload_a = R_outs[5];
    const auto Rlc_a = Rlc_b - impedances.Sb_pr * (Sb_a - Sb_b); // Rlc incident
    const auto Pb_a = -Sb_a - Rlc_b + impedances.Sb_pr * (Sb_a - Sb_b); // Pb incident
    state.Pb_z = Pb_b + Pb_a - state.Pb_z; // Pb state update
    const auto P_treble_cut_minus_a = Ptreble_cut_b - P_treble_cut_minus_b + Ptreble_cut_a; // P_treble_cut_minus incident
    const auto Streble_cut_a = Ptreble_cut_b - Streble_cut_b + Ptreble_cut_a; // Streble_cut incident
    state.Streble_cut_z = -Streble_cut_a + impedances.Streble_cut_2RC_over_T_plus_2RC * (Streble_cut_a + state.Streble_cut_z); // Streble_cut state update
    state.Pbass_boost_z = Pbass_boost_b + Pbass_boost_a - state.Pbass_boost_z; // Pbass_boost state update
    
    const auto v_Rload = (Rload_a + Rload_b) * (float) 0.5; // Rload voltage
    return v_Rload;
}

} // namespace analog_eq

