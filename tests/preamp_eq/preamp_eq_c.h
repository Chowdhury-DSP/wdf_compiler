// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler preamp_eq.wdf preamp_eq_c.h -lang c

#pragma once

struct Params {
    float lfc_cap_value; // = 4.7e-09
    float r_series_value; // = 1.0e+04
    float hfl_res_value; // = 8.0e+04
    float hfl_cap_value; // = 2.7e-09
    float hfc_cap_value; // = 3.3e-06
    float r_shunt_value; // = 4.674e+03
    float lfl_res_value; // = 1.5e+04
    float lfl_cap_value; // = 1.5e-07
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

void calc_impedances(struct Impedances* impedances, float fs, struct Params params) {
    #define wdf_compiler_sum(a, b) ((a) + (b))
    #define wdf_compiler_recip_sum(a, b) (((a) * (b)) / ((a) + (b)))
    
    const float T = 1.0 / fs;
    
    // Computing impedance for: r_series;
    const float r_series_value = params.r_series_value;
    const float r_series_R = r_series_value;
    const float r_series_G = 1.0 / r_series_R;
    // Computing impedance for: hfl_res_cap;
    const float hfl_res_cap_res_value = params.hfl_res_value;
    const float hfl_res_cap_cap_value = params.hfl_cap_value;
    const float hfl_res_cap_R = (T / (2.0 * hfl_res_cap_cap_value)) + hfl_res_cap_res_value;
    const float hfl_res_cap_G = 1.0 / hfl_res_cap_R;
    impedances->hfl_res_cap_2RC_over_T_plus_2RC = (2.0 * hfl_res_cap_cap_value * hfl_res_cap_res_value) / (2.0 * hfl_res_cap_cap_value * hfl_res_cap_res_value + T);

    // Computing impedance for: hfc_cap;
    const float hfc_cap_value = params.hfc_cap_value;
    const float hfc_cap_G = 2.0 * hfc_cap_value * fs;
    const float hfc_cap_R = 1.0 / hfc_cap_G;
    // Computing impedance for: r_shunt;
    const float r_shunt_value = params.r_shunt_value;
    const float r_shunt_R = r_shunt_value;
    const float r_shunt_G = 1.0 / r_shunt_R;
    // Computing impedance for: lfl_res_cap;
    const float lfl_res_cap_res_value = params.lfl_res_value;
    const float lfl_res_cap_cap_value = params.lfl_cap_value;
    const float lfl_res_cap_2RC = 2.0 * lfl_res_cap_cap_value * lfl_res_cap_res_value;
    const float lfl_res_cap_R = lfl_res_cap_res_value * T / (lfl_res_cap_2RC + T);
    const float lfl_res_cap_G = 1.0 / lfl_res_cap_R;
    impedances->lfl_res_cap_2RC_over_2RC_plus_T = lfl_res_cap_2RC / (lfl_res_cap_2RC + T);

    // Computing impedance for: s_s;
    const float s_s_R = r_shunt_R + lfl_res_cap_R;
    const float s_s_G = 1.0 / s_s_R;
    impedances->s_s_pr = r_shunt_R * s_s_G;

    // Computing impedance for: p_s;
    const float p_s_G = hfc_cap_G + s_s_G;
    const float p_s_R = 1.0 / p_s_G;
    impedances->p_s_pr = hfc_cap_G * p_s_R;

    // Computing impedance for: p_p;
    const float p_p_G = r_series_G + hfl_res_cap_G;
    const float p_p_R = 1.0 / p_p_G;
    impedances->p_p_pr = r_series_G * p_p_R;

    // Computing impedance for: s_o;
    const float s_o_R = p_p_R + p_s_R;
    const float s_o_G = 1.0 / s_o_R;
    impedances->s_o_pr = p_p_R * s_o_G;

    
    // Computing impedance for: v_in;
    const float v_in_cap_value = params.lfc_cap_value;
    impedances->v_in_ca = T / (T + 2.0 * s_o_R * v_in_cap_value);
    impedances->v_in_ce = (2.0 * s_o_R * v_in_cap_value) / (T + 2.0 * s_o_R * v_in_cap_value);

}

float process(struct State* state, const struct Impedances* impedances, float v_in) {
    const float lfl_res_cap_b = impedances->lfl_res_cap_2RC_over_2RC_plus_T * state->lfl_res_cap_z; // lfl_res_cap reflected
    const float r_shunt_b = 0; // r_shunt reflected
    const float s_s_b = -(r_shunt_b + lfl_res_cap_b); // s_s reflected
    const float hfc_cap_b = state->hfc_cap_z; // hfc_cap reflected
    const float p_s_b = s_s_b - impedances->p_s_pr * (s_s_b - hfc_cap_b); // p_s reflected
    const float hfl_res_cap_b = -state->hfl_res_cap_z; // hfl_res_cap reflected
    const float r_series_b = 0; // r_series reflected
    const float p_p_b = hfl_res_cap_b - impedances->p_p_pr * (hfl_res_cap_b - r_series_b); // p_p reflected
    const float s_o_b = -(p_p_b + p_s_b); // s_o reflected
    const float v_in_a = s_o_b; // v_in incident
    const float v_in_a0a1 = 2.0 * (v_in + state->v_in_z) + v_in_a; // v_in reflected
    const float v_in_b = impedances->v_in_ca * v_in_a - impedances->v_in_ce * v_in_a0a1; // v_in reflected
    state->v_in_z += -v_in_b - v_in_a0a1; // v_in_z state update
    const float s_o_a = v_in_b; // s_o incident
    const float p_p_a = p_p_b - impedances->s_o_pr * (s_o_a - s_o_b); // p_p incident
    const float p_s_a = -s_o_a - p_p_b + impedances->s_o_pr * (s_o_a - s_o_b); // p_s incident
    const float r_series_a = p_p_b - r_series_b + p_p_a; // r_series incident
    const float hfl_res_cap_a = p_p_b - hfl_res_cap_b + p_p_a; // hfl_res_cap incident
    state->hfl_res_cap_z = -hfl_res_cap_a + impedances->hfl_res_cap_2RC_over_T_plus_2RC * (hfl_res_cap_a + state->hfl_res_cap_z); // hfl_res_cap state update
    const float hfc_cap_a = p_s_b - hfc_cap_b + p_s_a; // hfc_cap incident
    const float s_s_a = p_s_b - s_s_b + p_s_a; // s_s incident
    state->hfc_cap_z = hfc_cap_a; // hfc_cap state update
    const float r_shunt_a = r_shunt_b - impedances->s_s_pr * (s_s_a - s_s_b); // r_shunt incident
    const float lfl_res_cap_a = -s_s_a - r_shunt_b + impedances->s_s_pr * (s_s_a - s_s_b); // lfl_res_cap incident
    state->lfl_res_cap_z = lfl_res_cap_b + lfl_res_cap_a - state->lfl_res_cap_z; // lfl_res_cap state update
    
    const float v_hfc_cap = (hfc_cap_a + hfc_cap_b) * (float) 0.5; // hfc_cap voltage
    return v_hfc_cap;
}

