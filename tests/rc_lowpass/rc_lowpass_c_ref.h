// Auto-generated with wdf_compiler version 1.0.0.e5eb401.
// Command: wdf_compiler rc_lowpass.wdf rc_lowpass.h

#pragma once

// params struct: no default values
// calc_impedances: pass impedances struct as pointer (add "struct" keyword)
// calc_impedances: impedance references with "->"
// calc_impedances: sum and recip_sum
// calc_impedances: "const float" (not "const auto")
// calc_impedances: no "inline"
// process: pass state and impedances structs as pointers (add "struct" keyword)
// process: "const float" (not "const auto")

struct Params {
    const float R1_value; // = 1.0e+03;
    const float C1_value; // = 1.0e-06;
};

struct Impedances {
    float Vin_ca;
    float Vin_ce;
};

struct State {
    float C1_z;
};

void calc_impedances(struct Impedances* impedances, float fs, struct Params params) {
    // [[maybe_unused]] static constexpr auto sum = [] (auto a, auto b) { return a + b; };
    // [[maybe_unused]] static constexpr auto recip_sum = [] (auto a, auto b) { return (a * b) / (a + b); };

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
