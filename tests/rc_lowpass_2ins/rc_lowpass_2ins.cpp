#include "rc_lowpass_2ins.h"

#include "../chowdsp_wdf.h"
#include <iostream>
#include <fstream>

struct Reference_WDF
{
    chowdsp::wdft::ResistiveVoltageSourceT<float> R1 { 1000.0f };
    chowdsp::wdft::CapacitorT<float> C1 { 1.0e-6f };
    chowdsp::wdft::WDFSeriesT<float, decltype (R1), decltype (C1)> S1 { R1, C1 };
    chowdsp::wdft::IdealVoltageSourceT<float, decltype (S1)> Vin { S1 };

    void prepare (float fs)
    {
        C1.prepare (fs);
    }

    float process (float V1, float V2)
    {
        Vin.setVoltage (V1);
        R1.setVoltage (V2);
        Vin.incident (S1.reflected());
        S1.incident (Vin.reflected());
        return chowdsp::wdft::voltage<float> (C1);
    }
};

int main()
{
    std::cout << "RC Lowpass 2-inputs test\n";

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
        const auto test_output = process (state, impedances, 1.0f, 2.0f);
        ref_output[i] = ref.process (1.0f, 2.0f);
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
