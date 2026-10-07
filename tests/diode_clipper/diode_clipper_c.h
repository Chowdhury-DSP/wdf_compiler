// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler diode_clipper.net diode_clipper_c.h -lang c -netlist

#pragma once

#include <wdf_lib_diode_pair_c.h>

struct Params {
    float C1_value; // = 1.0e-06
    float Vin_res_value; // = 1.0e+03
    struct wdf_lib_diode_pair_Diode_Pair_Params DP_params;
};

struct Impedances {
    float P4_pr;
    struct wdf_lib_diode_pair_Diode_Pair_Vars DP_vars;
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
    // Computing impedance for: P4;
    const float P4_G = C1_G + Vin_G;
    const float P4_R = 1.0 / P4_G;
    impedances->P4_pr = C1_G * P4_R;

    // Computing impedance for: DP;
    wdf_lib_diode_pair_update_vars(&(impedances->DP_vars), &(params.DP_params), P4_R, P4_G);
}

float process(struct State* state, const struct Impedances* impedances, float Vin) {
    const float Vin_b = Vin; // Vin reflected
    const float C1_b = state->C1_z; // C1 reflected
    const float P4_b = Vin_b - impedances->P4_pr * (Vin_b - C1_b); // P4 reflected
    const float P4_a = wdf_lib_diode_pair_root_compute(&(impedances->DP_vars), P4_b); // P4 incident
    const float C1_a = P4_b - C1_b + P4_a; // C1 incident
    const float Vin_a = P4_b - Vin_b + P4_a; // Vin incident
    state->C1_z = C1_a; // C1 state update
    
    const float v_C1 = (C1_a + C1_b) * (float) 0.5; // C1 voltage
    return v_C1;
}

