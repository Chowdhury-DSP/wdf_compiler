#include "diode_clipper.h"

#include "../chowdsp_wdf.h"
#include <iostream>

// clang diode_clipper.cpp --std=c++20 -lstdc++ -o diode_clipper.exe && ./diode_clipper.exe

struct Reference_WDF
{
    chowdsp::wdft::ResistiveVoltageSourceT<float> Vin { 1000.0f };
    chowdsp::wdft::CapacitorT<float> C1 { 1.0e-6f };
    chowdsp::wdft::WDFParallelT<float, decltype (Vin), decltype (C1)> P1 { Vin, C1 };
    chowdsp::wdft::DiodePairT<float, decltype (P1)> DP { P1, 1.0e-9f };

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
    std::cout << "Diode Clipper test\n";

    static constexpr float fs = 48000.0f;

    Reference_WDF ref {};
    ref.prepare (fs);

    Params params {
        .C1_value = 1.0e-6f,
        .Vin_res_value = 1.0e3f,
        .DP_params = {
            .Is = 1.0e-9f,
        },
    };
    Impedances impedances {};
    calc_impedances (impedances, fs, params);
    State state {};

    float max_error = 0.0f;
    for (int i = 0; i < 100; ++i)
    {
        const auto test_output = process (state, impedances, 1.0f);
        const auto ref_output = ref.process (1.0f);
        const auto error = std::abs (test_output - ref_output);
        max_error = std::max (error, max_error);
    }
    std::cout << "Max Error: " << max_error << '\n';

    if (max_error > 1.0e-3f)
    {
        std::cout << "Error is too large... failing test!\n";
        return 1;
    }

    return 0;
}
