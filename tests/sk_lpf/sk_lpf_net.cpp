#include "sk_lpf.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <fstream>

// clang sk_lpf.cpp --std=c++20 -lstdc++ -I../../lib -o sk_lpf.exe && ./sk_lpf.exe

// Sallen-Key low-pass filter with a non-inverting op-amp gain stage
// (R1=R2=10k, C1=C2=100n, Rf=8k, Rg=6k). This is the first test to exercise
// wdf_compiler's netlist -> R-type-adaptor -> VCVS codegen path, so there is
// no independent chowdsp_wdf-based reference implementation to compare
// against yet. Correctness is checked against textbook op-amp/filter theory
// instead:
//   - DC gain of a non-inverting amplifier stage is exactly 1 + Rf/Rg.
//   - The circuit is a low-pass filter, so a high-frequency sine should be
//     attenuated well below the DC gain.
int main()
{
    std::printf("Sallen-Key LPF (netlist) test\n");

    static constexpr float fs = 48000.0f;

    Params params{};
    Impedances impedances{};
    calc_impedances(impedances, fs, params);
    State state{};

    bool failed = false;

    // --- DC gain check --------------------------------------------------
    // Sustained unity step; let capacitors fully settle, then compare
    // against the exact non-inverting op-amp gain formula.
    float dc_out = 0.0f;
    for (int i = 0; i < 48000; ++i)
    {
        dc_out = process(state, impedances, 1.0f);
        if (std::isnan(dc_out) || std::isinf(dc_out))
        {
            std::printf("NaN/Inf encountered during DC settle at sample %d\n", i);
            return 1;
        }
    }

    const float expected_dc_gain = 1.0f + params.Rf_value / params.Rg_value;
    const float dc_error = std::fabs(dc_out - expected_dc_gain);
    std::printf("DC output: %.6f (expected %.6f, error %.6f)\n", dc_out, expected_dc_gain, dc_error);
    if (dc_error > 1.0e-3f)
    {
        std::printf("DC gain error is too large... failing test!\n");
        failed = true;
    }

    // --- Low-pass behaviour check ----------------------------------------
    // A well-above-cutoff sine should be attenuated well below the DC gain.
    State hf_state{};
    float hf_max_abs = 0.0f;
    static constexpr float hf_freq = 15000.0f;
    static constexpr int N = 9600;
    std::array<float, N> ref_output{};
    for (int i = 0; i < N; ++i)
    {
        const float in = std::sin(2.0f * 3.14159265f * hf_freq * (float) i / fs);
        const float out = process(hf_state, impedances, in);
        ref_output[(size_t) i] = out;
        if (std::isnan(out) || std::isinf(out))
        {
            std::printf("NaN/Inf encountered during HF sweep at sample %d\n", i);
            return 1;
        }
        if (i > N / 2)
            hf_max_abs = std::max(hf_max_abs, std::fabs(out));
    }
    std::printf("15kHz steady-state amplitude: %.6f (DC gain: %.6f)\n", hf_max_abs, expected_dc_gain);
    if (hf_max_abs > 0.5f * expected_dc_gain)
    {
        std::printf("High-frequency signal is not sufficiently attenuated... failing test!\n");
        failed = true;
    }

    if (failed)
        return 1;

    std::ofstream ofp{ "data.bin", std::ios::out | std::ios::binary };
    ofp.write(reinterpret_cast<const char*>(ref_output.data()), N * sizeof(float));
    ofp.close();

    std::printf("PASSED\n");
    return 0;
}
