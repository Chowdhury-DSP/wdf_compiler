// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler baxandall_eq.wdf baxandall_eq_c.h -lang c

#pragma once

#include "custom_baxandall_rtype_c.h"

struct Params {
    float Ca_value; // = 1.0e-06
    float S4_res_value;
    float S4_cap_value; // = 6.4e-09
    float Rl_value; // = 1.0e+06
    float S5_res_value;
    float S5_cap_value; // = 6.4e-08
    float Resc_value; // = 1.0e+04
    float Resb_value; // = 1.0e+03
    float P3_res_value;
    float P3_cap_value; // = 2.2e-07
    float Resa_value; // = 1.0e+04
    float P2_res_value;
    float P2_cap_value; // = 2.2e-08
    struct R_Params R_params;
};

struct Impedances {
    float Vin_ca;
    float Vin_ce;
    float S4_2RC_over_T_plus_2RC;
    float S5_2RC_over_T_plus_2RC;
    float P3_2RC_over_2RC_plus_T;
    float P2_2RC_over_2RC_plus_T;
    float S3_pr;
    float S2_pr;
    float P1_pr;
    struct R_Vars R_vars;
};

struct State {
    float Vin_z;
    float S4_z;
    float S5_z;
    float P3_z;
    float P2_z;
};

void calc_impedances(struct Impedances* impedances, float fs, struct Params params) {
    #define wdf_compiler_sum(a, b) ((a) + (b))
    #define wdf_compiler_recip_sum(a, b) (((a) * (b)) / ((a) + (b)))
    
    const float T = 1.0 / fs;
    
    // Computing impedance for: S4;
    const float S4_res_value = params.S4_res_value;
    const float S4_cap_value = params.S4_cap_value;
    const float S4_R = (T / (2.0 * S4_cap_value)) + S4_res_value;
    const float S4_G = 1.0 / S4_R;
    impedances->S4_2RC_over_T_plus_2RC = (2.0 * S4_cap_value * S4_res_value) / (2.0 * S4_cap_value * S4_res_value + T);

    // Computing impedance for: Rl;
    const float Rl_value = params.Rl_value;
    const float Rl_R = Rl_value;
    const float Rl_G = 1.0 / Rl_R;
    // Computing impedance for: S5;
    const float S5_res_value = params.S5_res_value;
    const float S5_cap_value = params.S5_cap_value;
    const float S5_R = (T / (2.0 * S5_cap_value)) + S5_res_value;
    const float S5_G = 1.0 / S5_R;
    impedances->S5_2RC_over_T_plus_2RC = (2.0 * S5_cap_value * S5_res_value) / (2.0 * S5_cap_value * S5_res_value + T);

    // Computing impedance for: Resc;
    const float Resc_value = params.Resc_value;
    const float Resc_R = Resc_value;
    const float Resc_G = 1.0 / Resc_R;
    // Computing impedance for: Resb;
    const float Resb_value = params.Resb_value;
    const float Resb_R = Resb_value;
    const float Resb_G = 1.0 / Resb_R;
    // Computing impedance for: P3;
    const float P3_res_value = params.P3_res_value;
    const float P3_cap_value = params.P3_cap_value;
    const float P3_2RC = 2.0 * P3_cap_value * P3_res_value;
    const float P3_R = P3_res_value * T / (P3_2RC + T);
    const float P3_G = 1.0 / P3_R;
    impedances->P3_2RC_over_2RC_plus_T = P3_2RC / (P3_2RC + T);

    // Computing impedance for: Resa;
    const float Resa_value = params.Resa_value;
    const float Resa_R = Resa_value;
    const float Resa_G = 1.0 / Resa_R;
    // Computing impedance for: P2;
    const float P2_res_value = params.P2_res_value;
    const float P2_cap_value = params.P2_cap_value;
    const float P2_2RC = 2.0 * P2_cap_value * P2_res_value;
    const float P2_R = P2_res_value * T / (P2_2RC + T);
    const float P2_G = 1.0 / P2_R;
    impedances->P2_2RC_over_2RC_plus_T = P2_2RC / (P2_2RC + T);

    // Computing impedance for: S3;
    const float S3_R = Resb_R + P3_R;
    const float S3_G = 1.0 / S3_R;
    impedances->S3_pr = Resb_R * S3_G;

    // Computing impedance for: S2;
    const float S2_R = Resa_R + P2_R;
    const float S2_G = 1.0 / S2_R;
    impedances->S2_pr = Resa_R * S2_G;

    // Computing impedance for: P1;
    const float P1_G = Rl_G + S5_G;
    const float P1_R = 1.0 / P1_G;
    impedances->P1_pr = Rl_G * P1_R;

    // Computing impedance for: R;
    const float R_R_calc = update_vars(&(impedances->R_vars), &(params.R_params), S4_R, S4_G, P1_R, P1_G, Resc_R, Resc_G, S3_R, S3_G, S2_R, S2_G);
    const float R_R = R_R_calc;
    const float R_G = 1.0 / R_R;
    
    // Computing impedance for: Vin;
    const float Vin_cap_value = params.Ca_value;
    impedances->Vin_ca = T / (T + 2.0 * R_R * Vin_cap_value);
    impedances->Vin_ce = (2.0 * R_R * Vin_cap_value) / (T + 2.0 * R_R * Vin_cap_value);

}

float process(struct State* state, const struct Impedances* impedances, float Vin) {
    const float P2_b = impedances->P2_2RC_over_2RC_plus_T * state->P2_z; // P2 reflected
    const float Resa_b = 0; // Resa reflected
    const float S2_b = -(Resa_b + P2_b); // S2 reflected
    const float P3_b = impedances->P3_2RC_over_2RC_plus_T * state->P3_z; // P3 reflected
    const float Resb_b = 0; // Resb reflected
    const float S3_b = -(Resb_b + P3_b); // S3 reflected
    const float Resc_b = 0; // Resc reflected
    const float S5_b = -state->S5_z; // S5 reflected
    const float Rl_b = 0; // Rl reflected
    const float P1_b = S5_b - impedances->P1_pr * (S5_b - Rl_b); // P1 reflected
    const float S4_b = -state->S4_z; // S4 reflected
    _Alignas (16) const float R_ins[5] = { S4_b, P1_b, Resc_b, S3_b, S2_b }; // R inputs
    const float R_b = reflected(&(impedances->R_vars), R_ins); // R reflected
    const float Vin_a = R_b; // Vin incident
    const float Vin_a0a1 = 2.0 * (Vin + state->Vin_z) + Vin_a; // Vin reflected
    const float Vin_b = impedances->Vin_ca * Vin_a - impedances->Vin_ce * Vin_a0a1; // Vin reflected
    state->Vin_z += -Vin_b - Vin_a0a1; // Vin_z state update
    const float R_a = Vin_b; // R incident
    _Alignas (16) float R_outs[5]; // 5 outputs 
    incident(&(impedances->R_vars), R_a, R_ins, R_outs); // R compute
    const float S4_a = R_outs[0];
    const float P1_a = R_outs[1];
    const float Resc_a = R_outs[2];
    const float S3_a = R_outs[3];
    const float S2_a = R_outs[4];
    state->S4_z = -S4_a + impedances->S4_2RC_over_T_plus_2RC * (S4_a + state->S4_z); // S4 state update
    const float Rl_a = P1_b - Rl_b + P1_a; // Rl incident
    const float S5_a = P1_b - S5_b + P1_a; // S5 incident
    state->S5_z = -S5_a + impedances->S5_2RC_over_T_plus_2RC * (S5_a + state->S5_z); // S5 state update
    const float Resb_a = Resb_b - impedances->S3_pr * (S3_a - S3_b); // Resb incident
    const float P3_a = -S3_a - Resb_b + impedances->S3_pr * (S3_a - S3_b); // P3 incident
    state->P3_z = P3_b + P3_a - state->P3_z; // P3 state update
    const float Resa_a = Resa_b - impedances->S2_pr * (S2_a - S2_b); // Resa incident
    const float P2_a = -S2_a - Resa_b + impedances->S2_pr * (S2_a - S2_b); // P2 incident
    state->P2_z = P2_b + P2_a - state->P2_z; // P2 state update
    
    const float v_Rl = (Rl_a + Rl_b) * (float) 0.5; // Rl voltage
    return v_Rl;
}

