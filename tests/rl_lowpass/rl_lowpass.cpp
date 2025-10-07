#include "rl_lowpass.h"

#include "../chowdsp_wdf.h"
#include <iostream>

struct Reference_WDF
{
    chowdsp::wdft::ResistorT<float> R1 { 1000.0f };
    chowdsp::wdft::InductorT<float> L1 { 1.0e-3f };
    chowdsp::wdft::WDFParallelT<float, decltype (R1), decltype (L1)> P1 { R1, L1 };
    chowdsp::wdft::IdealCurrentSourceT<float, decltype (P1)> Iin { P1 };

    void prepare (float fs)
    {
        L1.prepare (fs);
    }

    float process (float I)
    {
        Iin.setCurrent (I);
        Iin.incident (P1.reflected());
        P1.incident (Iin.reflected());
        return chowdsp::wdft::current<float> (R1);
    }
};

int main()
{
    std::cout << "RL Lowpass test\n";

    static constexpr float fs = 48000.0f;

    Reference_WDF ref {};
    ref.prepare (fs);

    test::rl_lowpass::Impedances impedances {};
    test::rl_lowpass::calc_impedances (impedances, fs);
    test::rl_lowpass::State state {};

    float max_error = 0.0f;
    for (int i = 0; i < 100; ++i)
    {
        const auto test_output = test::rl_lowpass::process (state, impedances, 1.0f);
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
