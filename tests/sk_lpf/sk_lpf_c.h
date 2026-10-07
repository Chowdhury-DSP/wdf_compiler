// Auto-generated with wdf_compiler version 1.1.0.1b9dff5.
// Command: wdf_compiler sk_lpf.net sk_lpf_c.h -lang c -netlist

#pragma once

#include "R9_rtype.h"

struct Params {
    float R1_value; // = 1.0e+04
    float Rload_value; // = 1.0e+05
    float R2_value; // = 1.0e+04
    float C1_value; // = 9.999999e-08
    float C2_value; // = 9.999999e-08
    float Rf_value; // = 8.0e+03
    float Rg_value; // = 6.0e+03
    struct rtype_R9_R_Params R9_params;
};

struct Impedances {
    float Vin_ca;
    float Vin_ce;
    struct rtype_R9_R_Vars R9_vars;
};

struct State {
    float C1_z;
    float C2_z;
};

void calc_impedances(struct Impedances* impedances, float fs, struct Params params) {
    #define wdf_compiler_sum(a, b) ((a) + (b))
    #define wdf_compiler_recip_sum(a, b) (((a) * (b)) / ((a) + (b)))
    
    const float T = 1.0 / fs;
    
    // Computing impedance for: Rload;
    const float Rload_value = params.Rload_value;
    const float Rload_R = Rload_value;
    const float Rload_G = 1.0 / Rload_R;
    // Computing impedance for: R2;
    const float R2_value = params.R2_value;
    const float R2_R = R2_value;
    const float R2_G = 1.0 / R2_R;
    // Computing impedance for: C1;
    const float C1_value = params.C1_value;
    const float C1_G = 2.0 * C1_value * fs;
    const float C1_R = 1.0 / C1_G;
    // Computing impedance for: C2;
    const float C2_value = params.C2_value;
    const float C2_G = 2.0 * C2_value * fs;
    const float C2_R = 1.0 / C2_G;
    // Computing impedance for: Rf;
    const float Rf_value = params.Rf_value;
    const float Rf_R = Rf_value;
    const float Rf_G = 1.0 / Rf_R;
    // Computing impedance for: Rg;
    const float Rg_value = params.Rg_value;
    const float Rg_R = Rg_value;
    const float Rg_G = 1.0 / Rg_R;
    // Computing impedance for: R9;
    const float R9_R_calc = rtype_R9_update_vars(&(impedances->R9_vars), &(params.R9_params), Rload_R, Rload_G, R2_R, R2_G, C1_R, C1_G, C2_R, C2_G, Rf_R, Rf_G, Rg_R, Rg_G);
    const float R9_R = R9_R_calc;
    const float R9_G = 1.0 / R9_R;
    
    // Computing impedance for: Vin;
    const float Vin_res_value = params.R1_value;
    impedances->Vin_ca = (Vin_res_value - R9_R) / (Vin_res_value + R9_R);
    impedances->Vin_ce = (2.0 * R9_R) / (Vin_res_value + R9_R);

}

float process(struct State* state, const struct Impedances* impedances, float Vin) {
    const float Rg_b = 0; // Rg reflected
    const float Rf_b = 0; // Rf reflected
    const float C2_b = state->C2_z; // C2 reflected
    const float C1_b = state->C1_z; // C1 reflected
    const float R2_b = 0; // R2 reflected
    const float Rload_b = 0; // Rload reflected
    _Alignas (16) const float R9_ins[6] = { Rload_b, R2_b, C1_b, C2_b, Rf_b, Rg_b }; // R9 inputs
    const float R9_b = rtype_R9_reflected(&(impedances->R9_vars), R9_ins); // R9 reflected
    const float Vin_a = R9_b; // Vin incident
    const float Vin_b = impedances->Vin_ca * Vin_a + impedances->Vin_ce * -(Vin); // Vin reflected
    const float R9_a = Vin_b; // R9 incident
    _Alignas (16) float R9_outs[6]; // 6 outputs 
    rtype_R9_incident(&(impedances->R9_vars), R9_a, R9_ins, R9_outs); // R9 compute
    const float Rload_a = R9_outs[0];
    const float R2_a = R9_outs[1];
    const float C1_a = R9_outs[2];
    const float C2_a = R9_outs[3];
    const float Rf_a = R9_outs[4];
    const float Rg_a = R9_outs[5];
    state->C1_z = C1_a; // C1 state update
    state->C2_z = C2_a; // C2 state update
    
    const float v_Rload = (Rload_a + Rload_b) * (float) 0.5; // Rload voltage
    return v_Rload;
}

