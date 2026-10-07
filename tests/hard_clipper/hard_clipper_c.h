// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler hard_clipper.wdf hard_clipper_c.h -lang c

#pragma once

#include "custom_hard_clipper.h"

struct Params {
    float C1_value;
    float Vin_res_value;
    struct HC_Params HC_params;
};

struct Impedances {
    float P1_pr;
    struct HC_Vars HC_vars;
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
    // Computing impedance for: Vin;
    const float Vin_res_value = params.Vin_res_value;
    const float Vin_R = Vin_res_value;
    const float Vin_G = 1.0 / Vin_R;
    // Computing impedance for: P1;
    const float P1_G = C1_G + Vin_G;
    const float P1_R = 1.0 / P1_G;
    impedances->P1_pr = C1_G * P1_R;

    // Computing impedance for: HC;
    update_vars(&(impedances->HC_vars), &(params.HC_params), P1_R, P1_G);
}

float process(struct State* state, const struct Impedances* impedances, float Vin) {
    const float Vin_b = Vin; // Vin reflected
    const float C1_b = state->C1_z; // C1 reflected
    const float P1_b = Vin_b - impedances->P1_pr * (Vin_b - C1_b); // P1 reflected
    const float P1_a = root_compute(&(impedances->HC_vars), P1_b); // P1 incident
    const float C1_a = P1_b - C1_b + P1_a; // C1 incident
    const float Vin_a = P1_b - Vin_b + P1_a; // Vin incident
    state->C1_z = C1_a; // C1 state update
    
    const float v_C1 = (C1_a + C1_b) * (float) 0.5; // C1 voltage
    return v_C1;
}

