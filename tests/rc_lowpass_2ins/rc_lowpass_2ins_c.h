// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler rc_lowpass_2ins.wdf rc_lowpass_2ins_c.h -lang c

#pragma once

struct Params {
    float R1_res_value; // = 1.0e+03
    float C1_value; // = 1.0e-06
};

struct Impedances {
    float S1_pr;
};

struct State {
    float C1_z;
};

void calc_impedances(struct Impedances* impedances, float fs, struct Params params) {
    #define wdf_compiler_sum(a, b) ((a) + (b))
    #define wdf_compiler_recip_sum(a, b) (((a) * (b)) / ((a) + (b)))
    
    const float T = 1.0 / fs;
    
    // Computing impedance for: R1;
    const float R1_res_value = params.R1_res_value;
    const float R1_R = R1_res_value;
    const float R1_G = 1.0 / R1_R;
    // Computing impedance for: C1;
    const float C1_value = params.C1_value;
    const float C1_G = 2.0 * C1_value * fs;
    const float C1_R = 1.0 / C1_G;
    // Computing impedance for: S1;
    const float S1_R = R1_R + C1_R;
    const float S1_G = 1.0 / S1_R;
    impedances->S1_pr = R1_R * S1_G;

}

float process(struct State* state, const struct Impedances* impedances, float Vin, float R1) {
    const float C1_b = state->C1_z; // C1 reflected
    const float R1_b = R1; // R1 reflected
    const float S1_b = -(R1_b + C1_b); // S1 reflected
    const float Vin_a = S1_b; // Vin incident
    const float Vin_b = -Vin_a + 2.0 * Vin; // Vin reflected
    const float S1_a = Vin_b; // S1 incident
    const float R1_a = R1_b - impedances->S1_pr * (S1_a - S1_b); // R1 incident
    const float C1_a = -S1_a - R1_b + impedances->S1_pr * (S1_a - S1_b); // C1 incident
    state->C1_z = C1_a; // C1 state update
    
    const float v_C1 = (C1_a + C1_b) * (float) 0.5; // C1 voltage
    return v_C1;
}

