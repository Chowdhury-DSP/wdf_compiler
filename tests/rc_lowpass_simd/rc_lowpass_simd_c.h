// Auto-generated with wdf_compiler version 1.0.0.8f6ce20.
// Command: wdf_compiler rc_lowpass_simd.wdf rc_lowpass_simd_c.h -lang c

#pragma once

struct Params {
    const float R1_value; // = 1.0e+03
    const float C1_value; // = 1.0e-06
};

struct Impedances {
    float Vin_ca;
    float Vin_ce;
};

struct State {
    float C1_z;
};

void calc_impedances(struct Impedances* impedances, float fs, struct Params params) {
    
    const float T = 1 / fs;
    
    // Computing impedance for: C1;
    const float C1_value = params.C1_value;
    const float C1_G = 2 * C1_value * fs;
    const float C1_R = 1 / C1_G;
    
    // Computing impedance for: Vin;
    const float Vin_res_value = params.R1_value;
    impedances->Vin_ca = (Vin_res_value - C1_R) / (Vin_res_value + C1_R);
    impedances->Vin_ce = (2 * C1_R) / (Vin_res_value + C1_R);

}

float process(struct State* state, const struct Impedances* impedances, float Vin) {
    const float C1_b = state->C1_z; // C1 reflected
    const float Vin_a = C1_b; // Vin incident
    const float Vin_b = impedances->Vin_ca * Vin_a + impedances->Vin_ce * -(Vin); // Vin reflected
    const float C1_a = Vin_b; // C1 incident
    state->C1_z = C1_a; // C1 state update
    
    const float v_C1 = (C1_a + C1_b) * (float) 0.5; // C1 voltage
    return v_C1;
}

