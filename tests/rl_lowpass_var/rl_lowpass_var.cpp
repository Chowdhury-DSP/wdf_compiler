#include "rl_lowpass_var.h"

#include "../chowdsp_wdf.h"
#include <iostream>

struct Reference_WDF
{
    chowdsp::wdft::ResistorT<float> R1 { 1000.0f };
    chowdsp::wdft::InductorT<float> L1 { 1.0e-3f };
    chowdsp::wdft::WDFParallelT<float, decltype (R1), decltype (L1)> P3 { R1, L1 };
    
    chowdsp::wdft::ResistiveCurrentSourceT<float> Ivar { 2000.0f };
    chowdsp::wdft::WDFParallelT<float, decltype (Ivar), decltype (P3)> P2 { Ivar, P3 };
    
    chowdsp::wdft::ResistiveCurrentSourceT<float> Iin { 1000.0f };
    chowdsp::wdft::WDFParallelT<float, decltype (Ivar), decltype (P2)> P1 { Iin, P2 };

    chowdsp::wdft::IdealCurrentSourceT<float, decltype (P1)> Iplus { P1 };

    void prepare (float fs)
    {
        L1.prepare (fs);
    }

    float process (float I)
    {
        Iin.setCurrent (I);
        Iplus.incident (P1.reflected());
        P1.incident (Iplus.reflected());
        return chowdsp::wdft::current<float> (R1);
    }
};

int main()
{
    std::cout << "RL Lowpass test\n";

    static constexpr float fs = 48000.0f;
    static constexpr float Iplus_I = 1.0f;
    static constexpr float Ivar_I = -0.99f;
    static constexpr float R1 = 59.0e3f;

    Reference_WDF ref {};
    ref.prepare (fs);
    ref.Iplus.setCurrent(Iplus_I);
    ref.Ivar.setCurrent(Ivar_I);
    ref.R1.setResistanceValue(R1);

    Impedances impedances {};
    calc_impedances (impedances, fs, { Iplus_I, Ivar_I, R1 });
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
