#include "rl_lowpass.h"

#include "../chowdsp_wdf.h"
#include <iostream>

struct Reference_WDF
{
    chowdsp::wdft::ResistorT<float> R1 { 1000.0f };
    chowdsp::wdft::InductorT<float> L1 { 1.0e-3f };
    chowdsp::wdft::WDFSeriesT<float, decltype (R1), decltype (L1)> S1 { R1, L1 };
    chowdsp::wdft::IdealCurrentSourceT<float, decltype (S1)> Iin { S1 };

    void prepare (float fs)
    {
        L1.prepare (fs);
    }

    float process (float I)
    {
        Iin.setCurrent (I);
        Iin.incident (S1.reflected());
        S1.incident (Iin.reflected());
        return chowdsp::wdft::current<float> (R1);
    }
};

int main()
{
    std::cout << "RL Lowpass test\n";

    static constexpr float fs = 48000.0f;

    Reference_WDF ref {};
    ref.prepare (fs);

    Impedances impedances {};
    calc_impedances (impedances, fs);
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
