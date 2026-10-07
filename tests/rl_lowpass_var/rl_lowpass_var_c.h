// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler rl_lowpass_var.net rl_lowpass_var_c.h -lang c -netlist

#pragma once

struct Params {
    float Iplus_i_value;
    float Iin_res_value;
    float R_In_value; // = 1.0e+03
    float Ivar_i_value;
    float Ivar_res_value;
    float R_Ivar_value; // = 2.0e+03
    float R1_value;
    float L1_value; // = 1.0e-03
};

struct Impedances {
    float Iplus_I;
    float P7_R;
    float Iin_R;
    float Ivar_R;
    float Ivar_I;
    float R1_G;
    float P11_pr;
    float P10_pr;
    float P9_pr;
    float P8_pr;
    float P7_pr;
};

struct State {
    float L1_z;
};

void calc_impedances(struct Impedances* impedances, float fs, struct Params params) {
    #define wdf_compiler_sum(a, b) ((a) + (b))
    #define wdf_compiler_recip_sum(a, b) (((a) * (b)) / ((a) + (b)))
    
    const float T = 1.0 / fs;
    
    // Computing impedance for: Iin;
    const float Iin_R = params.Iin_res_value;
    impedances->Iin_R = Iin_R;
    const float Iin_G = 1.0 / Iin_R;
    // Computing impedance for: R_In;
    const float R_In_value = params.R_In_value;
    const float R_In_R = R_In_value;
    const float R_In_G = 1.0 / R_In_R;
    // Computing current for: Ivar;
    impedances->Ivar_I = params.Ivar_i_value;

    // Computing impedance for: Ivar;
    const float Ivar_R = params.Ivar_res_value;
    impedances->Ivar_R = Ivar_R;
    const float Ivar_G = 1.0 / Ivar_R;
    // Computing impedance for: R_Ivar;
    const float R_Ivar_value = params.R_Ivar_value;
    const float R_Ivar_R = R_Ivar_value;
    const float R_Ivar_G = 1.0 / R_Ivar_R;
    // Computing impedance for: R1;
    const float R1_value = params.R1_value;
    const float R1_R = R1_value;
    const float R1_G = 1.0 / R1_R;
    impedances->R1_G = R1_G;
    // Computing impedance for: L1;
    const float L1_value = params.L1_value;
    const float L1_R = 2.0 * L1_value * fs;
    const float L1_G = 1.0 / L1_R;
    // Computing impedance for: P11;
    const float P11_G = R1_G + L1_G;
    const float P11_R = 1.0 / P11_G;
    impedances->P11_pr = R1_G * P11_R;

    // Computing impedance for: P10;
    const float P10_G = R_Ivar_G + P11_G;
    const float P10_R = 1.0 / P10_G;
    impedances->P10_pr = R_Ivar_G * P10_R;

    // Computing impedance for: P9;
    const float P9_G = Ivar_G + P10_G;
    const float P9_R = 1.0 / P9_G;
    impedances->P9_pr = Ivar_G * P9_R;

    // Computing impedance for: P8;
    const float P8_G = R_In_G + P9_G;
    const float P8_R = 1.0 / P8_G;
    impedances->P8_pr = R_In_G * P8_R;

    // Computing impedance for: P7;
    const float P7_G = Iin_G + P8_G;
    const float P7_R = 1.0 / P7_G;
    impedances->P7_R = P7_R;
    impedances->P7_pr = Iin_G * P7_R;

    
    // Computing current for: Iplus;
    impedances->Iplus_I = params.Iplus_i_value;

}

float process(struct State* state, const struct Impedances* impedances, float Iin) {
    const float L1_b = -state->L1_z; // L1 reflected
    const float R1_b = 0; // R1 reflected
    const float P11_b = L1_b - impedances->P11_pr * (L1_b - R1_b); // P11 reflected
    const float R_Ivar_b = 0; // R_Ivar reflected
    const float P10_b = P11_b - impedances->P10_pr * (P11_b - R_Ivar_b); // P10 reflected
    const float Ivar_b = impedances->Ivar_R * impedances->Ivar_I; // Ivar reflected
    const float P9_b = P10_b - impedances->P9_pr * (P10_b - Ivar_b); // P9 reflected
    const float R_In_b = 0; // R_In reflected
    const float P8_b = P9_b - impedances->P8_pr * (P9_b - R_In_b); // P8 reflected
    const float Iin_b = impedances->Iin_R * Iin; // Iin reflected
    const float P7_b = P8_b - impedances->P7_pr * (P8_b - Iin_b); // P7 reflected
    const float Iplus_a = P7_b; // Iplus incident
    const float Iplus_b = 2.0 * impedances->P7_R * impedances->Iplus_I + Iplus_a; // Iplus reflected (@TODO: 2.0 * R?)
    const float P7_a = Iplus_b; // P7 incident
    const float Iin_a = P7_b - Iin_b + P7_a; // Iin incident
    const float P8_a = P7_b - P8_b + P7_a; // P8 incident
    const float R_In_a = P8_b - R_In_b + P8_a; // R_In incident
    const float P9_a = P8_b - P9_b + P8_a; // P9 incident
    const float Ivar_a = P9_b - Ivar_b + P9_a; // Ivar incident
    const float P10_a = P9_b - P10_b + P9_a; // P10 incident
    const float R_Ivar_a = P10_b - R_Ivar_b + P10_a; // R_Ivar incident
    const float P11_a = P10_b - P11_b + P10_a; // P11 incident
    const float R1_a = P11_b - R1_b + P11_a; // R1 incident
    const float L1_a = P11_b - L1_b + P11_a; // L1 incident
    state->L1_z = L1_a; // L1 state update
    
    const float i_R1 = (R1_a - R1_b) * ((float) 0.5 * impedances->R1_G); // R1 current
    return i_R1;
}

