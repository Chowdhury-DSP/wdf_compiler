// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler channel_hpf.wdf channel_hpf_c.h -lang c

#pragma once

struct Params {
    float Sw1_closed;
    float R8_value; // = 1.0e+04
    float Lh_value; // = 3.0e+00
    float Ll_value; // = 7.0e+00
    float C17_Vin_cap_value; // = 4.7e-07
    float C18_value; // = 4.7e-07
    float R9_value; // = 5.1e+03
};

struct Impedances {
    float Sw1_closed;
    float S2_pr;
    float P3_pr;
    float P2_pr;
    float S1_pr;
    float P1_pr;
};

struct State {
    float Lh_z;
    float Ll_z;
    float C17_Vin_z;
    float C17_Vin_v1;
    float C18_z;
};

void calc_impedances(struct Impedances* impedances, float fs, struct Params params) {
    #define wdf_compiler_sum(a, b) ((a) + (b))
    #define wdf_compiler_recip_sum(a, b) (((a) * (b)) / ((a) + (b)))
    
    const float T = 1.0 / fs;
    
    // Computing impedance for: R8;
    const float R8_value = params.R8_value;
    const float R8_R = R8_value;
    const float R8_G = 1.0 / R8_R;
    // Computing impedance for: Lh;
    const float Lh_value = params.Lh_value;
    const float Lh_R = 2.0 * Lh_value * fs;
    const float Lh_G = 1.0 / Lh_R;
    // Computing impedance for: Ll;
    const float Ll_value = params.Ll_value;
    const float Ll_R = 2.0 * Ll_value * fs;
    const float Ll_G = 1.0 / Ll_R;
    // Computing impedance for: C17_Vin;
    const float C17_Vin_cap_value = params.C17_Vin_cap_value;
    const float C17_Vin_G = 2.0 * C17_Vin_cap_value * fs;
    const float C17_Vin_R = 1.0 / C17_Vin_G;
    // Computing impedance for: C18;
    const float C18_value = params.C18_value;
    const float C18_G = 2.0 * C18_value * fs;
    const float C18_R = 1.0 / C18_G;
    // Computing impedance for: R9;
    const float R9_value = params.R9_value;
    const float R9_R = R9_value;
    const float R9_G = 1.0 / R9_R;
    // Computing impedance for: S2;
    const float S2_R = C18_R + R9_R;
    const float S2_G = 1.0 / S2_R;
    impedances->S2_pr = C18_R * S2_G;

    // Computing impedance for: P3;
    const float P3_G = C17_Vin_G + S2_G;
    const float P3_R = 1.0 / P3_G;
    impedances->P3_pr = C17_Vin_G * P3_R;

    // Computing impedance for: P2;
    const float P2_G = R8_G + Lh_G;
    const float P2_R = 1.0 / P2_G;
    impedances->P2_pr = R8_G * P2_R;

    // Computing impedance for: S1;
    const float S1_R = Ll_R + P3_R;
    const float S1_G = 1.0 / S1_R;
    impedances->S1_pr = Ll_R * S1_G;

    // Computing impedance for: P1;
    const float P1_G = P2_G + S1_G;
    const float P1_R = 1.0 / P1_G;
    impedances->P1_pr = P2_G * P1_R;

    
    // Computing state for: Sw1;
    impedances->Sw1_closed = params.Sw1_closed;

}

float process(struct State* state, const struct Impedances* impedances, float C17_Vin) {
    const float R9_b = 0; // R9 reflected
    const float C18_b = state->C18_z; // C18 reflected
    const float S2_b = -(C18_b + R9_b); // S2 reflected
    const float C17_Vin_b = state->C17_Vin_z + C17_Vin - state->C17_Vin_v1; // C17_Vin reflected
    state->C17_Vin_v1 = C17_Vin; // C17_Vin state update
    const float P3_b = S2_b - impedances->P3_pr * (S2_b - C17_Vin_b); // P3 reflected
    const float Ll_b = -state->Ll_z; // Ll reflected
    const float S1_b = -(Ll_b + P3_b); // S1 reflected
    const float Lh_b = -state->Lh_z; // Lh reflected
    const float R8_b = 0; // R8 reflected
    const float P2_b = Lh_b - impedances->P2_pr * (Lh_b - R8_b); // P2 reflected
    const float P1_b = S1_b - impedances->P1_pr * (S1_b - P2_b); // P1 reflected
    const float Sw1_a = P1_b; // Sw1 incident
    const float Sw1_b = impedances->Sw1_closed != 0 ? -Sw1_a : Sw1_a; // Sw1 reflected
    const float P1_a = Sw1_b; // P1 incident
    const float P2_a = P1_b - P2_b + P1_a; // P2 incident
    const float S1_a = P1_b - S1_b + P1_a; // S1 incident
    const float R8_a = P2_b - R8_b + P2_a; // R8 incident
    const float Lh_a = P2_b - Lh_b + P2_a; // Lh incident
    state->Lh_z = Lh_a; // Lh state update
    const float Ll_a = Ll_b - impedances->S1_pr * (S1_a - S1_b); // Ll incident
    const float P3_a = -S1_a - Ll_b + impedances->S1_pr * (S1_a - S1_b); // P3 incident
    state->Ll_z = Ll_a; // Ll state update
    const float C17_Vin_a = P3_b - C17_Vin_b + P3_a; // C17_Vin incident
    const float S2_a = P3_b - S2_b + P3_a; // S2 incident
    state->C17_Vin_z = C17_Vin_a; // C17_Vin state update
    const float C18_a = C18_b - impedances->S2_pr * (S2_a - S2_b); // C18 incident
    const float R9_a = -S2_a - C18_b + impedances->S2_pr * (S2_a - S2_b); // R9 incident
    state->C18_z = C18_a; // C18 state update
    
    const float v_R9 = (R9_a + R9_b) * (float) 0.5; // R9 voltage
    return v_R9;
}

