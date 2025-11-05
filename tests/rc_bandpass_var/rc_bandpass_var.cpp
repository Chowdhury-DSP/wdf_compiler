#include "rc_bandpass_var.h"

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
    chowdsp::wdft::PolarityInverterT<float, decltype (S1)> I1 { S1 };
    chowdsp::wdft::IdealVoltageSourceT<float, decltype (I1)> Vin { I1 };

    void prepare (float fs)
    {
        C1.prepare (fs);
        C2.prepare (fs);
    }

    float process (float V)
    {
        Vin.setVoltage (V);
        Vin.incident (I1.reflected());
        I1.incident (Vin.reflected());
        return chowdsp::wdft::voltage<float> (C2);
    }
};

int main()
{
    std::cout << "RC Bandpass test\n";

    static constexpr float fs = 48000.0f;

    Reference_WDF ref {};
    ref.prepare (fs);

    Params params {
        .C1_value = 1.0e-6f,
        .R1_value = 1.0e3f,
        .R2_value = 1.0e3f,
        .C2_value = 1.0e-6f,
    };
    Impedances impedances {};
    calc_impedances (impedances, fs, params);
    State state {};

    static constexpr int N = 300;
    std::array<float, N> ref_output {};
    float max_error = 0.0f;
    for (int i = 0; i < 100; ++i)
    {
        const auto test_output = process (state, impedances, i == 0 ? 1.0f : 0.0f);
        ref_output[i] = ref.process (i == 0 ? 1.0f : 0.0f);
        const auto error = std::abs (test_output - ref_output[i]);
        max_error = std::max (error, max_error);
    }

    ref.R2.setResistanceValue (100.0f);
    ref.C2.setCapacitanceValue (1.0e-9f);
    params.R2_value = 100.0f;
    params.C2_value = 1.0e-9f;
    calc_impedances_lpf2 (impedances, fs, params);
    for (int i = 0; i < 100; ++i)
    {
        const auto test_output = process (state, impedances, i == 0 ? 1.0f : 0.0f);
        ref_output[i + 100] = ref.process (i == 0 ? 1.0f : 0.0f);
        const auto error = std::abs (test_output - ref_output[i + 100]);
        max_error = std::max (error, max_error);
    }

    ref.R1.setResistanceValue (10.0e3f);
    ref.C1.setCapacitanceValue (1.0e-4f);
    params.R1_value = 10.0e3f;
    params.C1_value = 1.0e-4f;
    calc_impedances_lpf1 (impedances, fs, params);
    for (int i = 0; i < 100; ++i)
    {
        const auto test_output = process (state, impedances, i == 0 ? 1.0f : 0.0f);
        ref_output[i + 200] = ref.process (i == 0 ? 1.0f : 0.0f);
        const auto error = std::abs (test_output - ref_output[i + 200]);
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
