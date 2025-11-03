#include "rc_bandpass.h"

#include "../chowdsp_wdf.h"
#include <iostream>
#include <fstream>

struct Reference_WDF
{
    chowdsp::wdft::ResistorT<float> R2 { 1000.0f };
    chowdsp::wdft::CapacitorT<float> C2 { 1.0e-6f };
    chowdsp::wdft::WDFSeriesT<float, decltype (R2), decltype (C2)> S2 { R2, C2 };
    chowdsp::wdft::ResistorT<float> R1 { 1000.0f };
    chowdsp::wdft::WDFParallelT<float, decltype (R1), decltype (S2)> P1 { R1, S2 };
    chowdsp::wdft::CapacitorT<float> C1 { 1.0e-6f };
    chowdsp::wdft::WDFSeriesT<float, decltype (C1), decltype (P1)> S1 { C1, P1 };
    chowdsp::wdft::IdealVoltageSourceT<float, decltype (S1)> Vin { S1 };

    void prepare (float fs)
    {
        C1.prepare (fs);
        C2.prepare (fs);
    }

    float process (float V)
    {
        Vin.setVoltage (V);
        Vin.incident (S1.reflected());
        S1.incident (Vin.reflected());
        return chowdsp::wdft::voltage<float> (C2);
    }
};

int main()
{
    std::cout << "RC Bandpass test\n";

    static constexpr float fs = 48000.0f;

    Reference_WDF ref {};
    ref.prepare (fs);

    Impedances impedances {};
    calc_impedances (impedances, fs);
    State state {};

    static constexpr int N = 100;
    std::array<float, N> ref_output {};
    float max_error = 0.0f;
    for (int i = 0; i < N; ++i)
    {
        const auto test_output = process (state, impedances, i == 0 ? 1.0f : 0.0f);
        ref_output[i] = ref.process (i == 0 ? 1.0f : 0.0f);
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
