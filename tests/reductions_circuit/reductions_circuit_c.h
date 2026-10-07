// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler reductions_circuit.wdf reductions_circuit_c.h -lang c

#pragma once

struct Params {
    float Rv1_value; // = 1.0e+04
    float Rv2_value; // = 5.0e+03
    float Rl_value; // = 1.0e+05
    float R7_value; // = 1.0e+04
    float C7_value; // = 1.0e-05
    float R8_value; // = 1.0e+05
    float C8_value; // = 9.999999e-08
    float R1_value; // = 1.0e+03
    float R2_value;
    float R3_value; // = 4.0e+03
    float R4_value;
    float R5_value; // = 3.0e+03
    float C1_value; // = 1.0e-06
    float C2_value;
    float C3_value; // = 4.0e-09
    float C4_value;
    float C5_value; // = 3.0e-06
    float R6_value; // = 1.0e+04
    float C6_value; // = 4.0e-06
};

struct Impedances {
    float Vin_ca;
    float Vin_ce;
    float P7_2RC_over_2RC_plus_T;
    float S7_2RC_over_T_plus_2RC;
    float S8_pr;
    float Sl_pr;
};

struct State {
    float P7_z;
    float S7_z;
};

void calc_impedances(struct Impedances* impedances, float fs, struct Params params) {
    #define wdf_compiler_sum(a, b) ((a) + (b))
    #define wdf_compiler_recip_sum(a, b) (((a) * (b)) / ((a) + (b)))
    
    const float T = 1.0 / fs;
    
    // Computing impedance for: Rl;
    const float Rl_value = params.Rl_value;
    const float Rl_R = Rl_value;
    const float Rl_G = 1.0 / Rl_R;
    // Computing impedance for: P7;
    const float P7_res_value = wdf_compiler_recip_sum(params.R7_value, params.R8_value);
    const float P7_cap_value = wdf_compiler_sum(params.C7_value, params.C8_value);
    const float P7_2RC = 2.0 * P7_cap_value * P7_res_value;
    const float P7_R = P7_res_value * T / (P7_2RC + T);
    const float P7_G = 1.0 / P7_R;
    impedances->P7_2RC_over_2RC_plus_T = P7_2RC / (P7_2RC + T);

    // Computing impedance for: S7;
    const float S7_res_value = wdf_compiler_sum(wdf_compiler_recip_sum(wdf_compiler_sum(wdf_compiler_sum(params.R1_value, params.R2_value), params.R3_value), wdf_compiler_recip_sum(params.R4_value, params.R5_value)), params.R6_value);
    const float S7_cap_value = wdf_compiler_recip_sum(wdf_compiler_sum(wdf_compiler_recip_sum(wdf_compiler_recip_sum(params.C1_value, params.C2_value), params.C3_value), wdf_compiler_sum(params.C4_value, params.C5_value)), params.C6_value);
    const float S7_R = (T / (2.0 * S7_cap_value)) + S7_res_value;
    const float S7_G = 1.0 / S7_R;
    impedances->S7_2RC_over_T_plus_2RC = (2.0 * S7_cap_value * S7_res_value) / (2.0 * S7_cap_value * S7_res_value + T);

    // Computing impedance for: S8;
    const float S8_R = P7_R + S7_R;
    const float S8_G = 1.0 / S8_R;
    impedances->S8_pr = P7_R * S8_G;

    // Computing impedance for: Sl;
    const float Sl_R = Rl_R + S8_R;
    const float Sl_G = 1.0 / Sl_R;
    impedances->Sl_pr = Rl_R * Sl_G;

    
    // Computing impedance for: Vin;
    const float Vin_res_value = wdf_compiler_sum(params.Rv1_value, params.Rv2_value);
    impedances->Vin_ca = (Vin_res_value - Sl_R) / (Vin_res_value + Sl_R);
    impedances->Vin_ce = (2.0 * Sl_R) / (Vin_res_value + Sl_R);

}

float process(struct State* state, const struct Impedances* impedances, float Vin) {
    const float S7_b = -state->S7_z; // S7 reflected
    const float P7_b = impedances->P7_2RC_over_2RC_plus_T * state->P7_z; // P7 reflected
    const float S8_b = -(P7_b + S7_b); // S8 reflected
    const float Rl_b = 0; // Rl reflected
    const float Sl_b = -(Rl_b + S8_b); // Sl reflected
    const float Vin_a = Sl_b; // Vin incident
    const float Vin_b = impedances->Vin_ca * Vin_a + impedances->Vin_ce * Vin; // Vin reflected
    const float Sl_a = Vin_b; // Sl incident
    const float Rl_a = Rl_b - impedances->Sl_pr * (Sl_a - Sl_b); // Rl incident
    const float S8_a = -Sl_a - Rl_b + impedances->Sl_pr * (Sl_a - Sl_b); // S8 incident
    const float P7_a = P7_b - impedances->S8_pr * (S8_a - S8_b); // P7 incident
    const float S7_a = -S8_a - P7_b + impedances->S8_pr * (S8_a - S8_b); // S7 incident
    state->P7_z = P7_b + P7_a - state->P7_z; // P7 state update
    state->S7_z = -S7_a + impedances->S7_2RC_over_T_plus_2RC * (S7_a + state->S7_z); // S7 state update
    
    const float v_Rl = (Rl_a + Rl_b) * (float) 0.5; // Rl voltage
    return v_Rl;
}

