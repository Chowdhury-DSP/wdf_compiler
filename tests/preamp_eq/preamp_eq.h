// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler preamp_eq.wdf preamp_eq.h

#pragma once

struct Params {
    static constexpr float lfc_cap_value = 4.7e-09;
    static constexpr float r_series_value = 1.0e+04;
    static constexpr float hfl_res_value = 8.0e+04;
    static constexpr float hfl_cap_value = 2.7e-09;
    static constexpr float hfc_cap_value = 3.3e-06;
    static constexpr float r_shunt_value = 4.674e+03;
    static constexpr float lfl_res_value = 1.5e+04;
    static constexpr float lfl_cap_value = 1.5e-07;
};

struct Impedances {
    float v_in_ca;
    float v_in_ce;
    float hfl_res_cap_2RC_over_T_plus_2RC;
    float lfl_res_cap_2RC_over_2RC_plus_T;
    float s_s_pr;
    float p_s_pr;
    float p_p_pr;
    float s_o_pr;
};

struct State {
    float v_in_z;
    float hfl_res_cap_z;
    float hfc_cap_z;
    float lfl_res_cap_z;
};

inline void calc_impedances(Impedances& impedances, float fs, Params params = {}) {
    [[maybe_unused]] static constexpr auto sum = [] (auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [] (auto a, auto b) { return (a * b) / (a + b); };
    
    [[maybe_unused]] const auto T = 1.0 / fs;
    
    // Computing impedance for: r_series;
    [[maybe_unused]] const auto r_series_value = params.r_series_value;
    [[maybe_unused]] const auto r_series_R = r_series_value;
    [[maybe_unused]] const auto r_series_G = 1.0 / r_series_R;
    // Computing impedance for: hfl_res_cap;
    [[maybe_unused]] const auto hfl_res_cap_res_value = params.hfl_res_value;
    [[maybe_unused]] const auto hfl_res_cap_cap_value = params.hfl_cap_value;
    [[maybe_unused]] const auto hfl_res_cap_R = (T / (2.0 * hfl_res_cap_cap_value)) + hfl_res_cap_res_value;
    [[maybe_unused]] const auto hfl_res_cap_G = 1.0 / hfl_res_cap_R;
    impedances.hfl_res_cap_2RC_over_T_plus_2RC = (2.0 * hfl_res_cap_cap_value * hfl_res_cap_res_value) / (2.0 * hfl_res_cap_cap_value * hfl_res_cap_res_value + T);

    // Computing impedance for: hfc_cap;
    [[maybe_unused]] const auto hfc_cap_value = params.hfc_cap_value;
    [[maybe_unused]] const auto hfc_cap_G = 2.0 * hfc_cap_value * fs;
    [[maybe_unused]] const auto hfc_cap_R = 1.0 / hfc_cap_G;
    // Computing impedance for: r_shunt;
    [[maybe_unused]] const auto r_shunt_value = params.r_shunt_value;
    [[maybe_unused]] const auto r_shunt_R = r_shunt_value;
    [[maybe_unused]] const auto r_shunt_G = 1.0 / r_shunt_R;
    // Computing impedance for: lfl_res_cap;
    [[maybe_unused]] const auto lfl_res_cap_res_value = params.lfl_res_value;
    [[maybe_unused]] const auto lfl_res_cap_cap_value = params.lfl_cap_value;
    [[maybe_unused]] const auto lfl_res_cap_2RC = 2.0 * lfl_res_cap_cap_value * lfl_res_cap_res_value;
    [[maybe_unused]] const auto lfl_res_cap_R = lfl_res_cap_res_value * T / (lfl_res_cap_2RC + T);
    [[maybe_unused]] const auto lfl_res_cap_G = 1.0 / lfl_res_cap_R;
    impedances.lfl_res_cap_2RC_over_2RC_plus_T = lfl_res_cap_2RC / (lfl_res_cap_2RC + T);

    // Computing impedance for: s_s;
    [[maybe_unused]] const auto s_s_R = r_shunt_R + lfl_res_cap_R;
    [[maybe_unused]] const auto s_s_G = 1.0 / s_s_R;
    impedances.s_s_pr = r_shunt_R * s_s_G;

    // Computing impedance for: p_s;
    [[maybe_unused]] const auto p_s_G = hfc_cap_G + s_s_G;
    [[maybe_unused]] const auto p_s_R = 1.0 / p_s_G;
    impedances.p_s_pr = hfc_cap_G * p_s_R;

    // Computing impedance for: p_p;
    [[maybe_unused]] const auto p_p_G = r_series_G + hfl_res_cap_G;
    [[maybe_unused]] const auto p_p_R = 1.0 / p_p_G;
    impedances.p_p_pr = r_series_G * p_p_R;

    // Computing impedance for: s_o;
    [[maybe_unused]] const auto s_o_R = p_p_R + p_s_R;
    [[maybe_unused]] const auto s_o_G = 1.0 / s_o_R;
    impedances.s_o_pr = p_p_R * s_o_G;

    
    // Computing impedance for: v_in;
    [[maybe_unused]] const auto v_in_cap_value = params.lfc_cap_value;
    impedances.v_in_ca = T / (T + 2.0 * s_o_R * v_in_cap_value);
    impedances.v_in_ce = (2.0 * s_o_R * v_in_cap_value) / (T + 2.0 * s_o_R * v_in_cap_value);

}

inline float process(State& state, const Impedances& impedances, float v_in) {
    const auto lfl_res_cap_b = impedances.lfl_res_cap_2RC_over_2RC_plus_T * state.lfl_res_cap_z; // lfl_res_cap reflected
    const auto r_shunt_b = 0; // r_shunt reflected
    const auto s_s_b = -(r_shunt_b + lfl_res_cap_b); // s_s reflected
    const auto hfc_cap_b = state.hfc_cap_z; // hfc_cap reflected
    const auto p_s_b = s_s_b - impedances.p_s_pr * (s_s_b - hfc_cap_b); // p_s reflected
    const auto hfl_res_cap_b = -state.hfl_res_cap_z; // hfl_res_cap reflected
    const auto r_series_b = 0; // r_series reflected
    const auto p_p_b = hfl_res_cap_b - impedances.p_p_pr * (hfl_res_cap_b - r_series_b); // p_p reflected
    const auto s_o_b = -(p_p_b + p_s_b); // s_o reflected
    const auto v_in_a = s_o_b; // v_in incident
    const auto v_in_a0a1 = 2.0 * (v_in + state.v_in_z) + v_in_a; // v_in reflected
    const auto v_in_b = impedances.v_in_ca * v_in_a - impedances.v_in_ce * v_in_a0a1; // v_in reflected
    state.v_in_z += -v_in_b - v_in_a0a1; // v_in_z state update
    const auto s_o_a = v_in_b; // s_o incident
    const auto p_p_a = p_p_b - impedances.s_o_pr * (s_o_a - s_o_b); // p_p incident
    const auto p_s_a = -s_o_a - p_p_b + impedances.s_o_pr * (s_o_a - s_o_b); // p_s incident
    const auto r_series_a = p_p_b - r_series_b + p_p_a; // r_series incident
    const auto hfl_res_cap_a = p_p_b - hfl_res_cap_b + p_p_a; // hfl_res_cap incident
    state.hfl_res_cap_z = -hfl_res_cap_a + impedances.hfl_res_cap_2RC_over_T_plus_2RC * (hfl_res_cap_a + state.hfl_res_cap_z); // hfl_res_cap state update
    const auto hfc_cap_a = p_s_b - hfc_cap_b + p_s_a; // hfc_cap incident
    const auto s_s_a = p_s_b - s_s_b + p_s_a; // s_s incident
    state.hfc_cap_z = hfc_cap_a; // hfc_cap state update
    const auto r_shunt_a = r_shunt_b - impedances.s_s_pr * (s_s_a - s_s_b); // r_shunt incident
    const auto lfl_res_cap_a = -s_s_a - r_shunt_b + impedances.s_s_pr * (s_s_a - s_s_b); // lfl_res_cap incident
    state.lfl_res_cap_z = lfl_res_cap_b + lfl_res_cap_a - state.lfl_res_cap_z; // lfl_res_cap state update
    
    const auto v_hfc_cap = (hfc_cap_a + hfc_cap_b) * (float) 0.5; // hfc_cap voltage
    return v_hfc_cap;
}

