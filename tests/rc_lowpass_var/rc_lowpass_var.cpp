#include "rc_lowpass_var.h"

#include "../chowdsp_wdf.h"
#include <iostream>

struct Reference_WDF
{
    chowdsp::wdft::ResistiveVoltageSourceT<float> R1 {};
    chowdsp::wdft::CapacitorT<float> C1 { 1.0e-6f };
    chowdsp::wdft::WDFSeriesT<float, decltype (R1), decltype (C1)> S3 { R1, C1 };

    chowdsp::wdft::ResistiveVoltageSourceT<float> Vminus {};
    chowdsp::wdft::WDFSeriesT<float, decltype (Vminus), decltype (S3)> S2 { Vminus, S3 };

    chowdsp::wdft::ResistiveVoltageSourceT<float> Vin { 1000.0f };
    chowdsp::wdft::WDFSeriesT<float, decltype (Vin), decltype (S2)> S1 { Vin, S2 };

    chowdsp::wdft::IdealVoltageSourceT<float, decltype (S1)> Vplus { S1 };

    void prepare (float fs)
    {
        C1.prepare (fs);
        Vplus.setVoltage (10.0f);
        Vminus.setVoltage (-5.0f);
    }

    float process (float V)
    {
        Vin.setVoltage (V);
        Vplus.incident (S1.reflected());
        S1.incident (Vplus.reflected());
        return chowdsp::wdft::voltage<float> (C1);
    }
};

int main()
{
    std::cout << "RC Lowpass test\n";

    static constexpr float fs = 48000.0f;
    float R1_V = 0.5f;
    float R1_R = 15.0e3f;
    float Vminus_R = 25.0e3f;
    float C1 = 4.0e-6f;

    Reference_WDF ref {};
    ref.prepare (fs);
    ref.R1.setVoltage (R1_V);
    ref.R1.setResistanceValue (R1_R);
    ref.Vminus.setResistanceValue (Vminus_R);
    ref.C1.setCapacitanceValue (C1);

    Impedances impedances {};
    calc_impedances (impedances, fs, { Vminus_R, R1_V, R1_R, C1 });
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
