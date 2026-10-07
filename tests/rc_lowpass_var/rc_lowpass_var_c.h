// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler rc_lowpass_var.wdf rc_lowpass_var_c.h -lang c

#pragma once

struct Params {
    float Vplus_v_value; // = 1.0e+01
    float Vin_res_value; // = 1.0e+03
    float Vminus_v_value; // = -5.0e+00
    float Vminus_res_value;
    float R1_v_value;
    float R1_res_value;
    float C1_value;
};

struct Impedances {
    float Vplus_V; // = Params->Vplus_v_value
    float Vminus_V; // = Params->Vminus_v_value
    float R1_V;
    float S3_pr;
    float S2_pr;
    float S1_pr;
};

struct State {
    float C1_z;
};

void calc_impedances(struct Impedances* impedances, float fs, struct Params params) {
    #define wdf_compiler_sum(a, b) ((a) + (b))
    #define wdf_compiler_recip_sum(a, b) (((a) * (b)) / ((a) + (b)))
    
    const float T = 1.0 / fs;
    
    // Computing impedance for: Vin;
    const float Vin_res_value = params.Vin_res_value;
    const float Vin_R = Vin_res_value;
    const float Vin_G = 1.0 / Vin_R;
    // Computing voltage for: Vminus;
    impedances->Vminus_V = params.Vminus_v_value;

    // Computing impedance for: Vminus;
    const float Vminus_res_value = params.Vminus_res_value;
    const float Vminus_R = Vminus_res_value;
    const float Vminus_G = 1.0 / Vminus_R;
    // Computing voltage for: R1;
    impedances->R1_V = params.R1_v_value;

    // Computing impedance for: R1;
    const float R1_res_value = params.R1_res_value;
    const float R1_R = R1_res_value;
    const float R1_G = 1.0 / R1_R;
    // Computing impedance for: C1;
    const float C1_value = params.C1_value;
    const float C1_G = 2.0 * C1_value * fs;
    const float C1_R = 1.0 / C1_G;
    // Computing impedance for: S3;
    const float S3_R = R1_R + C1_R;
    const float S3_G = 1.0 / S3_R;
    impedances->S3_pr = R1_R * S3_G;

    // Computing impedance for: S2;
    const float S2_R = Vminus_R + S3_R;
    const float S2_G = 1.0 / S2_R;
    impedances->S2_pr = Vminus_R * S2_G;

    // Computing impedance for: S1;
    const float S1_R = Vin_R + S2_R;
    const float S1_G = 1.0 / S1_R;
    impedances->S1_pr = Vin_R * S1_G;

    
    // Computing voltage for: Vplus;
    impedances->Vplus_V = params.Vplus_v_value;

}

float process(struct State* state, const struct Impedances* impedances, float Vin) {
    const float C1_b = state->C1_z; // C1 reflected
    const float R1_b = impedances->R1_V; // R1 reflected
    const float S3_b = -(R1_b + C1_b); // S3 reflected
    const float Vminus_b = impedances->Vminus_V; // Vminus reflected
    const float S2_b = -(Vminus_b + S3_b); // S2 reflected
    const float Vin_b = Vin; // Vin reflected
    const float S1_b = -(Vin_b + S2_b); // S1 reflected
    const float Vplus_a = S1_b; // Vplus incident
    const float Vplus_b = -Vplus_a + 2.0 * impedances->Vplus_V; // Vplus reflected
    const float S1_a = Vplus_b; // S1 incident
    const float Vin_a = Vin_b - impedances->S1_pr * (S1_a - S1_b); // Vin incident
    const float S2_a = -S1_a - Vin_b + impedances->S1_pr * (S1_a - S1_b); // S2 incident
    const float Vminus_a = Vminus_b - impedances->S2_pr * (S2_a - S2_b); // Vminus incident
    const float S3_a = -S2_a - Vminus_b + impedances->S2_pr * (S2_a - S2_b); // S3 incident
    const float R1_a = R1_b - impedances->S3_pr * (S3_a - S3_b); // R1 incident
    const float C1_a = -S3_a - R1_b + impedances->S3_pr * (S3_a - S3_b); // C1 incident
    state->C1_z = C1_a; // C1 state update
    
    const float v_C1 = (C1_a + C1_b) * (float) 0.5; // C1 voltage
    return v_C1;
}

