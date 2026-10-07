// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler rl_lowpass.wdf rl_lowpass_c.h -lang c

#pragma once

struct Params {
    float R1_value; // = 1.0e+03
    float L1_value; // = 1.0e-03
};

struct Impedances {
    float P1_R;
    float R1_G;
    float P1_pr;
};

struct State {
    float L1_z;
};

void calc_impedances(struct Impedances* impedances, float fs, struct Params params) {
    #define wdf_compiler_sum(a, b) ((a) + (b))
    #define wdf_compiler_recip_sum(a, b) (((a) * (b)) / ((a) + (b)))
    
    const float T = 1.0 / fs;
    
    // Computing impedance for: R1;
    const float R1_value = params.R1_value;
    const float R1_R = R1_value;
    const float R1_G = 1.0 / R1_R;
    impedances->R1_G = R1_G;
    // Computing impedance for: L1;
    const float L1_value = params.L1_value;
    const float L1_R = 2.0 * L1_value * fs;
    const float L1_G = 1.0 / L1_R;
    // Computing impedance for: P1;
    const float P1_G = R1_G + L1_G;
    const float P1_R = 1.0 / P1_G;
    impedances->P1_R = P1_R;
    impedances->P1_pr = R1_G * P1_R;

}

float process(struct State* state, const struct Impedances* impedances, float Iin) {
    const float L1_b = -state->L1_z; // L1 reflected
    const float R1_b = 0; // R1 reflected
    const float P1_b = L1_b - impedances->P1_pr * (L1_b - R1_b); // P1 reflected
    const float Iin_a = P1_b; // Iin incident
    const float Iin_b = 2.0 * impedances->P1_R * Iin + Iin_a; // Iin reflected (@TODO: 2.0 * R?)
    const float P1_a = Iin_b; // P1 incident
    const float R1_a = P1_b - R1_b + P1_a; // R1 incident
    const float L1_a = P1_b - L1_b + P1_a; // L1 incident
    state->L1_z = L1_a; // L1 state update
    
    const float i_R1 = (R1_a - R1_b) * ((float) 0.5 * impedances->R1_G); // R1 current
    return i_R1;
}

