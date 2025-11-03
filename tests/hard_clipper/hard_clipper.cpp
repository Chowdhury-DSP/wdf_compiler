#include "hard_clipper.h"

#include "../chowdsp_wdf.h"
#include <chrono>
#include <iostream>
#include <fstream>

// clang diode_clipper.cpp --std=c++20 -lstdc++ -o diode_clipper.exe && ./diode_clipper.exe

struct Reference_WDF
{
    chowdsp::wdft::ResistiveVoltageSourceT<float> Vin { 1000.0f };
    chowdsp::wdft::CapacitorT<float> C1 { 1.0e-6f };
    chowdsp::wdft::WDFParallelT<float, decltype (Vin), decltype (C1)> P1 { Vin, C1 };
    static constexpr auto A = 0.75f;

    void prepare (float fs)
    {
        C1.prepare (fs);
    }

    float process (float V)
    {
        Vin.setVoltage (V);
        const auto a = P1.reflected();

        float b;
        if (std::abs (a) < A * 0.5f)
            b = a;
        else
            b = -a + (a >= 0.0f ? A : -A);

        P1.incident (b);
        return chowdsp::wdft::voltage<float> (C1);
    }
};

int main()
{
    std::cout << "Hard Clipper test\n";

    static constexpr float fs = 48000.0f;

    Reference_WDF ref {};
    ref.prepare (fs);

    Params params {
        .C1_value = 1.0e-6f,
        .Vin_res_value = 1.0e3f,
        .HC_params = { .A = Reference_WDF::A },
    };
    Impedances impedances {};
    calc_impedances (impedances, fs, params);
    State state {};

    static constexpr int N = 100;
    std::array<float, N> ref_output {};
    float max_error = 0.0f;
    for (int i = 0; i < 100; ++i)
    {
        const auto test_output = process (state, impedances, 1.0f);
        ref_output[i] = ref.process (1.0f);
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
    ofp.write(reinterpret_cast<const char*>(ref_output.data()), N * sizeof (float));
    ofp.close();

    return 0;
}
