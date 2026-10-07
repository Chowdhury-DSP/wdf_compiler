// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler rc_bandpass.net rc_bandpass_c.h -lang c -netlist

#pragma once

struct Params {
    float C1_value; // = 1.0e-06
    float R1_value; // = 1.0e+03
    float C2_value; // = 1.0e-06
    float R2_value; // = 1.0e+03
};

struct Impedances {
    float Vin_ca;
    float Vin_ce;
    float Vin_cr;
    float S7_pr;
};

struct State {
    float Vin_z;
    float C2_z;
};

void calc_impedances(struct Impedances* impedances, float fs, struct Params params) {
    #define wdf_compiler_sum(a, b) ((a) + (b))
    #define wdf_compiler_recip_sum(a, b) (((a) * (b)) / ((a) + (b)))
    
    const float T = 1.0 / fs;
    
    // Computing impedance for: C2;
    const float C2_value = params.C2_value;
    const float C2_G = 2.0 * C2_value * fs;
    const float C2_R = 1.0 / C2_G;
    // Computing impedance for: R2;
    const float R2_value = params.R2_value;
    const float R2_R = R2_value;
    const float R2_G = 1.0 / R2_R;
    // Computing impedance for: S7;
    const float S7_R = C2_R + R2_R;
    const float S7_G = 1.0 / S7_R;
    impedances->S7_pr = C2_R * S7_G;

    
    // Computing impedance for: Vin;
    const float Vin_res_value = params.R1_value;
    const float Vin_cap_value = params.C1_value;
    const float Vin_Gp = (1.0 / Vin_res_value) + S7_G;
    const float Vin_Rp = 1.0 / Vin_Gp;
    impedances->Vin_ca = T / (T + 2.0 * Vin_Rp * Vin_cap_value);
    impedances->Vin_ce = (2.0 * Vin_Rp * Vin_cap_value) / (T + 2.0 * Vin_Rp * Vin_cap_value);
    impedances->Vin_cr = Vin_Rp / Vin_res_value;

}

float process(struct State* state, const struct Impedances* impedances, float Vin) {
    const float R2_b = 0; // R2 reflected
    const float C2_b = state->C2_z; // C2 reflected
    const float S7_b = -(C2_b + R2_b); // S7 reflected
    const float Vin_a = S7_b; // Vin incident
    const float Vin_a_cvs = impedances->Vin_cr * Vin_a; // Vin_CVs incident
    const float Vin_a0a1 = 2.0 * (Vin + state->Vin_z) + Vin_a_cvs; // Vin_CVs reflected
    const float Vin_b_cvs = impedances->Vin_ca * Vin_a_cvs - impedances->Vin_ce * Vin_a0a1; // Vin_CVs reflected
    const float Vin_b = Vin_b_cvs - Vin_a_cvs; // Vin reflected
    state->Vin_z += -Vin_b_cvs - Vin_a0a1; // Vin_z state update
    const float S7_a = Vin_b; // S7 incident
    const float C2_a = C2_b - impedances->S7_pr * (S7_a - S7_b); // C2 incident
    const float R2_a = -S7_a - C2_b + impedances->S7_pr * (S7_a - S7_b); // R2 incident
    state->C2_z = C2_a; // C2 state update
    
    const float v_C2 = (C2_a + C2_b) * (float) 0.5; // C2 voltage
    return v_C2;
}

