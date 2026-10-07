// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler reductions_circuit2.wdf reductions_circuit2_c.h -lang c

#pragma once

struct Params {
    float Vb_v_value; // = 1.5e+00
    float Rl1_value; // = 1.0e+03
    float L1_value; // = 2.0e-01
    float Rl2_value; // = 1.0e+04
    float Rp_value; // = 4.0e+03
    float Vcc_v_value; // = 1.2e+00
    float Vcc_cap_value;
    float C1_value; // = 1.0e-08
    float Cp2_value; // = 7.0e-09
    float Vin_res_value; // = 5.0e+03
    float R1_value; // = 1.0e+04
    float R2_value;
    float Cp_value; // = 4.0e-06
    float Rp2_value; // = 7.0e+03
    float Rl_v_value; // = -1.1e+00
    float Rl_res_value; // = 1.0e+05
    float Rl_cap_value; // = 1.0e-05
};

struct Impedances {
    float Vb_V; // = Params->Vb_v_value
    float Sr_Ri;
    float S4_V; // = Params->Vcc_v_value
    float S8_T_over_2RC;
    float Rl_V; // = Params->Rl_v_value
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

void calc_impedances(struct Impedances* impedances, float fs, struct Params params) {
    #define wdf_compiler_sum(a, b) ((a) + (b))
    #define wdf_compiler_recip_sum(a, b) (((a) * (b)) / ((a) + (b)))
    
    const float T = 1.0 / fs;
    
    // Computing impedance for: Sr;
    const float Sr_res_value = wdf_compiler_sum(params.Rl1_value, params.Rl2_value);
    const float Sr_ind_value = params.L1_value;
    const float Sr_R = 2.0 * Sr_ind_value * fs + Sr_res_value;
    const float Sr_G = 1.0 / Sr_R;
    impedances->Sr_Ri = Sr_res_value * Sr_G;

    // Computing impedance for: Rp;
    const float Rp_value = params.Rp_value;
    const float Rp_R = Rp_value;
    const float Rp_G = 1.0 / Rp_R;
    // Computing voltage for: S4;
    impedances->S4_V = params.Vcc_v_value;

    // Computing impedance for: S4;
    const float S4_cap_value = wdf_compiler_recip_sum(params.Vcc_cap_value, params.C1_value);
    const float S4_G = 2.0 * S4_cap_value * fs;
    const float S4_R = 1.0 / S4_G;
    // Computing impedance for: Cp2;
    const float Cp2_value = params.Cp2_value;
    const float Cp2_G = 2.0 * Cp2_value * fs;
    const float Cp2_R = 1.0 / Cp2_G;
    // Computing impedance for: S8;
    const float S8_res_value = wdf_compiler_sum(wdf_compiler_sum(wdf_compiler_sum(params.Vin_res_value, params.R1_value), params.R2_value), params.Rp2_value);
    const float S8_cap_value = params.Cp_value;
    const float S8_R = (T / (2.0 * S8_cap_value)) + S8_res_value;
    const float S8_G = 1.0 / S8_R;
    impedances->S8_T_over_2RC = T / (2.0 * S8_cap_value * S8_res_value);

    // Computing voltage for: Rl;
    impedances->Rl_V = params.Rl_v_value;

    // Computing impedance for: Rl;
    const float Rl_res_value = params.Rl_res_value;
    const float Rl_cap_value = params.Rl_cap_value;
    const float Rl_R = (T / (2.0 * Rl_cap_value)) + Rl_res_value;
    const float Rl_G = 1.0 / Rl_R;
    impedances->Rl_T_over_2RC = T / (2.0 * Rl_cap_value * Rl_res_value);

    // Computing impedance for: S7;
    const float S7_R = Cp2_R + S8_R;
    const float S7_G = 1.0 / S7_R;
    impedances->S7_pr = Cp2_R * S7_G;

    // Computing impedance for: S6;
    const float S6_R = Rp_R + S4_R;
    const float S6_G = 1.0 / S6_R;
    impedances->S6_pr = Rp_R * S6_G;

    // Computing impedance for: S3;
    const float S3_R = S6_R + S7_R;
    const float S3_G = 1.0 / S3_R;
    impedances->S3_pr = S6_R * S3_G;

    // Computing impedance for: Sl;
    const float Sl_R = S3_R + Rl_R;
    const float Sl_G = 1.0 / Sl_R;
    impedances->Sl_pr = S3_R * Sl_G;

    // Computing impedance for: Pr;
    const float Pr_R = Sr_R + Sl_R;
    const float Pr_G = 1.0 / Pr_R;
    impedances->Pr_pr = Sr_R * Pr_G;

    
    // Computing voltage for: Vb;
    impedances->Vb_V = params.Vb_v_value;

}

float process(struct State* state, const struct Impedances* impedances, float S8) {
    const float Rl_b = -(state->Rl_z + impedances->Rl_V); // Rl reflected
    const float S8_b = -(state->S8_z + -(S8)); // S8 reflected
    const float Cp2_b = state->Cp2_z; // Cp2 reflected
    const float S7_b = -(Cp2_b + S8_b); // S7 reflected
    const float S4_b = state->S4_z + -(impedances->S4_V) - state->S4_v1; // S4 reflected
    state->S4_v1 = -(impedances->S4_V); // S4 state update
    const float Rp_b = 0; // Rp reflected
    const float S6_b = -(Rp_b + S4_b); // S6 reflected
    const float S3_b = -(S6_b + S7_b); // S3 reflected
    const float Sl_b = -(S3_b + Rl_b); // Sl reflected
    const float Sr_b = state->Sr_z; // Sr reflected
    const float Pr_b = -(Sr_b + Sl_b); // Pr reflected
    const float Vb_a = Pr_b; // Vb incident
    const float Vb_b = -Vb_a + 2.0 * impedances->Vb_V; // Vb reflected
    const float Pr_a = Vb_b; // Pr incident
    const float Sr_a = Sr_b - impedances->Pr_pr * (Pr_a - Pr_b); // Sr incident
    const float Sl_a = -Pr_a - Sr_b + impedances->Pr_pr * (Pr_a - Pr_b); // Sl incident
    state->Sr_z = -Sr_a + impedances->Sr_Ri * (Sr_a - state->Sr_z); // Sr state update
    const float S3_a = S3_b - impedances->Sl_pr * (Sl_a - Sl_b); // S3 incident
    const float Rl_a = -Sl_a - S3_b + impedances->Sl_pr * (Sl_a - Sl_b); // Rl incident
    const float S6_a = S6_b - impedances->S3_pr * (S3_a - S3_b); // S6 incident
    const float S7_a = -S3_a - S6_b + impedances->S3_pr * (S3_a - S3_b); // S7 incident
    const float Rp_a = Rp_b - impedances->S6_pr * (S6_a - S6_b); // Rp incident
    const float S4_a = -S6_a - Rp_b + impedances->S6_pr * (S6_a - S6_b); // S4 incident
    state->S4_z = S4_a; // S4 state update
    const float Cp2_a = Cp2_b - impedances->S7_pr * (S7_a - S7_b); // Cp2 incident
    const float S8_a = -S7_a - Cp2_b + impedances->S7_pr * (S7_a - S7_b); // S8 incident
    state->Cp2_z = Cp2_a; // Cp2 state update
    state->S8_z -= impedances->S8_T_over_2RC * (S8_a - S8_b); // S8 state update
    state->Rl_z -= impedances->Rl_T_over_2RC * (Rl_a - Rl_b); // Rl state update
    
    const float v_Rl = (Rl_a + Rl_b) * (float) 0.5; // Rl voltage
    return v_Rl;
}

