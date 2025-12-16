#include "diode_circuit.h"

#include "../chowdsp_wdf.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>

// clang diode_circuit.cpp --std=c++20 -lstdc++ -I../../lib -o diode_circuit.exe && ./diode_circuit.exe

struct Reference_WDF
{
    chowdsp::wdft::ResistiveVoltageSourceT<float> Vin { 100.0f };
    chowdsp::wdft::CapacitorT<float> C1 { 1.0e-9f };
    chowdsp::wdft::WDFParallelT<float, decltype (Vin), decltype (C1)> P1 { Vin, C1 };
    chowdsp::wdft::DiodeT<float, decltype (P1)> DP { P1, 1.0e-9f };

    void prepare (float fs)
    {
        C1.prepare (fs);
    }

    float process (float V)
    {
        Vin.setVoltage (V);
        DP.incident (P1.reflected());
        P1.incident (DP.reflected());
        return chowdsp::wdft::voltage<float> (C1);
    }
};

int main()
{
    std::cout << "Diode Circuit test\n";

    static constexpr float fs = 48000.0f;

    Reference_WDF ref {};
    ref.prepare (fs);

    Params params {
        .C1_value = 1.0e-9f,
        .Vin_res_value = 100.0f,
        .DP_params = {
            .Is = 1.0e-9f,
        },
    };
    Impedances impedances {};
    calc_impedances (impedances, fs, params);
    State state {};

    static constexpr int N = 100;
    std::array<float, N> input {};
    std::array<float, N> ref_output {};
    float max_error = 0.0f;
    for (int i = 0; i < N; ++i)
    {
        input[i] = 10.0f * std::sin ((float) i * 0.1f);
        const auto test_output = process (state, impedances, input[i]);
        ref_output[i] = ref.process (input[i]);
        const auto error = std::abs (test_output - ref_output[i]);
        max_error = std::max (error, max_error);
    }
    std::cout << "Max Error: " << max_error << '\n';

    if (max_error > 1.0e-3f)
    {
        std::cout << "Error is too large... failing test!\n";
        return 1;
    }

    std::ofstream ofp { "data.bin", std::ios::out | std::ios::binary };
    ofp.write(reinterpret_cast<const char*>(input.data()), N * sizeof (float));
    ofp.write(reinterpret_cast<const char*>(ref_output.data()), N * sizeof (float));
    ofp.close();

    return 0;
}
