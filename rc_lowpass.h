#pragma once

struct Impedances {
    float S1_R;
    float S1_G;
    float R1_R;
    float R1_G;
    float C1_R;
    float C1_G;
};

struct State {
    float Vin_a;
    float Vin_b;
    float S1_a;
    float S1_b;
    float R1_a;
    float R1_b;
    float C1_a;
    float C1_b;
    float C1_z;
};

void calc_impedances(Impedances& impedances, float fs) {
    // Computing impedance for: R1;
    static constexpr float R1_value = 1000;
    impedances.R1_R = R1_value;
    impedances.R1_G = impedances.R1_R;

    // Computing impedance for: C1;
    static constexpr float C1_value = 0.000001;
    impedances.C1_G = 2.0f * C1_value * fs;
    impedances.C1_R = 1.0f / impedances.C1_G;

    // Computing impedance for: S1;
    impedances.S1_R = impedances.R1_R + impedances.C1_R;
    impedances.S1_G = 1.0f / impedances.S1_R;

}

float process(State& state, const Impedances& impedances, float Vin) {
    state.C1_b = state.C1_z;
    state.R1_b = 0;
    state.S1_b = -(state.R1_b + state.C1_b);
    state.Vin_a = state.S1_b;
    state.Vin_b = -state.Vin_a + 2 * Vin;
    state.S1_a = state.Vin_b;
    const auto pr_S1 = impedances.R1_R * impedances.S1_G;
    state.R1_a = state.R1_b - pr_S1 * (state.S1_a + state.R1_b + state.C1_b);
    state.C1_a = -(state.S1_a + state.R1_a);
    state.C1_z = state.C1_a;
    
    return (state.C1_a + state.C1_b) * 0.5f;
}
