#define _USE_MATH_DEFINES 1

#include "channel_hpf.h"

#include "../chowdsp_wdf.h"
#include <iostream>
#include <fstream>
#include <random>

namespace wdft = chowdsp::wdft;

class Reference_WDF
{
public:
    void prepare (float sampleRate)
    {
        C18.prepare (sampleRate);
        C17_Vin.prepare (sampleRate);
    }

    inline float process (float x) noexcept
    {
        C17_Vin.setVoltage (x);

        Sw1.incident (P1.reflected());
        P1.incident (Sw1.reflected());

        return wdft::voltage<float> (R9);
    }

    wdft::CapacitorT<float> C18 { 470.0e-9f };
    wdft::ResistorT<float> R9 { 5.1e3f };
    wdft::WDFSeriesT<float, decltype (C18), decltype (R9)> S2 { C18, R9 };

    wdft::CapacitiveVoltageSourceT<float> C17_Vin { 470.0e-9f };
    wdft::WDFParallelT<float, decltype (C17_Vin), decltype (S2)> P3 { C17_Vin, S2 };

    wdft::InductorT<float> Ll { 7.0f };
    wdft::WDFSeriesT<float, decltype (Ll), decltype (P3)> S1 { Ll, P3 };

    wdft::ResistorT<float> R8 { 10.0e3f };
    wdft::InductorT<float> Lh { 3.0f };
    wdft::WDFParallelT<float, decltype (R8), decltype (Lh)> P2 { R8, Lh };

    wdft::WDFParallelT<float, decltype (P2), decltype (S1)> P1 { P2, S1 };
    wdft::SwitchT<float, decltype (P1)> Sw1 { P1 };
};

int main()
{
    std::cout << "Channel HPF test\n";

    static constexpr float fs = 48000.0f;

    Reference_WDF ref {};
    ref.prepare (fs);
    ref.Sw1.setClosed (true);

    Impedances impedances {};
    calc_impedances (impedances, fs, { .Sw1_value = 1.0f });
    State state {};

    static constexpr int N = 100;
    std::array<float, 2 * N> ref_output {};
    float max_error = 0.0f;
    for (int i = 0; i < N; ++i)
    {
        const auto test_output = process (state, impedances, i == 0 ? 1.0f : 0.0f);
        ref_output[i] = ref.process (i == 0 ? 1.0f : 0.0f);
        const auto error = std::abs (test_output - ref_output[i]);
        max_error = std::max (error, max_error);
    }

    ref.Sw1.setClosed (false);
    calc_impedances (impedances, fs, { .Sw1_value = 0.0f });

    for (int i = 0; i < N; ++i)
    {
        const auto test_output = process (state, impedances, i == 0 ? 1.0f : 0.0f);
        ref_output[N + i] = ref.process (i == 0 ? 1.0f : 0.0f);
        const auto error = std::abs (test_output - ref_output[N + i]);
        max_error = std::max (error, max_error);
    }
    std::cout << "Max Error: " << max_error << '\n';

    if (max_error > 1.0e-4f)
    {
        std::cout << "Error is too large... failing test!\n";
        return 1;
    }

    std::ofstream ofp { "data.bin", std::ios::out | std::ios::binary };
    ofp.write(reinterpret_cast<const char*>(ref_output.data()), 2 * N * sizeof (float));
    ofp.close();

    return 0;
}
