// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler simple_triode.wdf simple_triode.h

#pragma once

#include "custom_dangelo_triode.h"

struct Params {
    static constexpr float Vin_res_value = 2.5e+04;
    static constexpr float Rk_value = 2.0e+02;
    static constexpr float Vplus_v_value = 2.0e+02;
    static constexpr float Vplus_res_value = 4.7e+04;
    static constexpr float Rl_value = 3.3e+05;
    static constexpr float Cp_value = 4.7e-08;
    struct dangelo_triode::Triode_Params Triode_params;
};

struct Impedances {
    static constexpr float Vplus_V = Params::Vplus_v_value;
    float Sp_pr;
    float Pp_pr;
    struct dangelo_triode::Triode_Vars Triode_vars;
};

struct State {
    float Cp_z;
};

inline void calc_impedances(Impedances& impedances, float fs, Params params = {}) {
    [[maybe_unused]] static constexpr auto sum = [] (auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [] (auto a, auto b) { return (a * b) / (a + b); };
    
    [[maybe_unused]] const auto T = 1.0 / fs;
    
    // Computing impedance for: Vin;
    [[maybe_unused]] const auto Vin_res_value = params.Vin_res_value;
    [[maybe_unused]] const auto Vin_R = Vin_res_value;
    [[maybe_unused]] const auto Vin_G = 1.0 / Vin_R;
    // Computing impedance for: Rk;
    [[maybe_unused]] const auto Rk_value = params.Rk_value;
    [[maybe_unused]] const auto Rk_R = Rk_value;
    [[maybe_unused]] const auto Rk_G = 1.0 / Rk_R;
    // Computing impedance for: Vplus;
    [[maybe_unused]] const auto Vplus_res_value = params.Vplus_res_value;
    [[maybe_unused]] const auto Vplus_R = Vplus_res_value;
    [[maybe_unused]] const auto Vplus_G = 1.0 / Vplus_R;
    // Computing impedance for: Rl;
    [[maybe_unused]] const auto Rl_value = params.Rl_value;
    [[maybe_unused]] const auto Rl_R = Rl_value;
    [[maybe_unused]] const auto Rl_G = 1.0 / Rl_R;
    // Computing impedance for: Cp;
    [[maybe_unused]] const auto Cp_value = params.Cp_value;
    [[maybe_unused]] const auto Cp_G = 2.0 * Cp_value * fs;
    [[maybe_unused]] const auto Cp_R = 1.0 / Cp_G;
    // Computing impedance for: Sp;
    [[maybe_unused]] const auto Sp_R = Rl_R + Cp_R;
    [[maybe_unused]] const auto Sp_G = 1.0 / Sp_R;
    impedances.Sp_pr = Rl_R * Sp_G;

    // Computing impedance for: Pp;
    [[maybe_unused]] const auto Pp_G = Vplus_G + Sp_G;
    [[maybe_unused]] const auto Pp_R = 1.0 / Pp_G;
    impedances.Pp_pr = Vplus_G * Pp_R;

    // Computing impedance for: Triode;
    dangelo_triode::update_vars(&(impedances.Triode_vars), &(params.Triode_params), Vin_R, Vin_G, Rk_R, Rk_G, Pp_R, Pp_G);
}

inline float process(State& state, const Impedances& impedances, float Vin) {
    const auto Cp_b = state.Cp_z; // Cp reflected
    const auto Rl_b = 0; // Rl reflected
    const auto Sp_b = -(Rl_b + Cp_b); // Sp reflected
    const auto Vplus_b = impedances.Vplus_V; // Vplus reflected
    const auto Pp_b = Sp_b - impedances.Pp_pr * (Sp_b - Vplus_b); // Pp reflected
    const auto Rk_b = 0; // Rk reflected
    const auto Vin_b = Vin; // Vin reflected
    alignas (16) const float Triode_ins[3] = { Vin_b, Rk_b, Pp_b }; // Triode inputs
    alignas (16) float Triode_outs[3]; // Triode outputs 
    dangelo_triode::root_compute(&impedances.Triode_vars, Triode_ins, Triode_outs); // Triode compute
    const float Vin_a = Triode_outs[0];
    const float Rk_a = Triode_outs[1];
    const float Pp_a = Triode_outs[2];
    const auto Vplus_a = Pp_b - Vplus_b + Pp_a; // Vplus incident
    const auto Sp_a = Pp_b - Sp_b + Pp_a; // Sp incident
    const auto Rl_a = Rl_b - impedances.Sp_pr * (Sp_a - Sp_b); // Rl incident
    const auto Cp_a = -Sp_a - Rl_b + impedances.Sp_pr * (Sp_a - Sp_b); // Cp incident
    state.Cp_z = Cp_a; // Cp state update
    
    const auto v_Rl = (Rl_a + Rl_b) * (float) 0.5; // Rl voltage
    return v_Rl;
}

