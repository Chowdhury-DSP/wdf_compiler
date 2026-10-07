// Auto-generated with wdf_compiler version 1.1.0.636c5ce.
// Command: wdf_compiler rc_lowpass_double.wdf rc_lowpass_double_c.h -lang c -dtype double

#pragma once

struct Params {
    double R1_value; // = 1.0e+03
    double C1_value; // = 1.0e-06
};

struct Impedances {
    double Vin_ca;
    double Vin_ce;
};

struct State {
    double C1_z;
};

void calc_impedances(struct Impedances* impedances, double fs, struct Params params) {
    #define wdf_compiler_sum(a, b) ((a) + (b))
    #define wdf_compiler_recip_sum(a, b) (((a) * (b)) / ((a) + (b)))
    
    const double T = 1.0 / fs;
    
    // Computing impedance for: C1;
    const double C1_value = params.C1_value;
    const double C1_G = 2.0 * C1_value * fs;
    const double C1_R = 1.0 / C1_G;
    
    // Computing impedance for: Vin;
    const double Vin_res_value = params.R1_value;
    impedances->Vin_ca = (Vin_res_value - C1_R) / (Vin_res_value + C1_R);
    impedances->Vin_ce = (2.0 * C1_R) / (Vin_res_value + C1_R);

}

double process(struct State* state, const struct Impedances* impedances, double Vin) {
    const double C1_b = state->C1_z; // C1 reflected
    const double Vin_a = C1_b; // Vin incident
    const double Vin_b = impedances->Vin_ca * Vin_a + impedances->Vin_ce * -(Vin); // Vin reflected
    const double C1_a = Vin_b; // C1 incident
    state->C1_z = C1_a; // C1 state update
    
    const double v_C1 = (C1_a + C1_b) * (double) 0.5; // C1 voltage
    return v_C1;
}

