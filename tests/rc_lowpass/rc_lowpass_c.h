// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler rc_lowpass.net rc_lowpass_c.h -lang c -netlist

#pragma once

struct Params {
    float R1_value; // = 1.0e+03
    float C1_value; // = 1.0e-06
};

struct Impedances {
    float V1_ca;
    float V1_ce;
};

struct State {
    float C1_z;
};

void calc_impedances(struct Impedances* impedances, float fs, struct Params params) {
    #define wdf_compiler_sum(a, b) ((a) + (b))
    #define wdf_compiler_recip_sum(a, b) (((a) * (b)) / ((a) + (b)))
    
    const float T = 1.0 / fs;
    
    // Computing impedance for: C1;
    const float C1_value = params.C1_value;
    const float C1_G = 2.0 * C1_value * fs;
    const float C1_R = 1.0 / C1_G;
    
    // Computing impedance for: V1;
    const float V1_res_value = params.R1_value;
    impedances->V1_ca = (V1_res_value - C1_R) / (V1_res_value + C1_R);
    impedances->V1_ce = (2.0 * C1_R) / (V1_res_value + C1_R);

}

float process(struct State* state, const struct Impedances* impedances, float V1) {
    const float C1_b = state->C1_z; // C1 reflected
    const float V1_a = C1_b; // V1 incident
    const float V1_b = impedances->V1_ca * V1_a + impedances->V1_ce * -(V1); // V1 reflected
    const float C1_a = V1_b; // C1 incident
    state->C1_z = C1_a; // C1 state update
    
    const float v_C1 = (C1_a + C1_b) * (float) 0.5; // C1 voltage
    return v_C1;
}

