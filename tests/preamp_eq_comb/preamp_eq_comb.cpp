#include "preamp_eq_comb.h"

#include "../chowdsp_wdf.h"
#include <iostream>

struct Reference_WDF
{
    chowdsp::wdft::ResistorCapacitorParallelT<float> lfl_res_cap { 15.0e3f, 0.15e-6f };
    chowdsp::wdft::ResistorT<float> r_shunt { 4'674.0f };
    chowdsp::wdft::WDFSeriesT<float, decltype (lfl_res_cap), decltype (r_shunt)> s_s { lfl_res_cap, r_shunt };

    chowdsp::wdft::CapacitorT<float> hfc_cap { 3.3e-6f };
    chowdsp::wdft::WDFParallelT<float, decltype (s_s), decltype (hfc_cap)> p_s { s_s, hfc_cap };

    chowdsp::wdft::ResistorCapacitorSeriesT<float> hfl_res_cap { 80.0e3f, 2.7e-9f };
    chowdsp::wdft::ResistorT<float> r_series { 10.0e3f };
    chowdsp::wdft::WDFParallelT<float, decltype (hfl_res_cap), decltype (r_series)> p_p { hfl_res_cap, r_series };

    chowdsp::wdft::WDFSeriesT<float, decltype (p_s), decltype (p_p)> s_o { p_s, p_p };

    chowdsp::wdft::CapacitorT<float> lfc_cap { 4.7e-9f };
    chowdsp::wdft::WDFSeriesT<float, decltype (s_o), decltype (lfc_cap)> s_i { s_o, lfc_cap };

    chowdsp::wdft::IdealVoltageSourceT<float, decltype (s_i)> v_in { s_i };

    void prepare (float fs)
    {
        lfl_res_cap.prepare (fs);
        hfc_cap.prepare (fs);
        hfl_res_cap.prepare (fs);
        lfc_cap.prepare (fs);
        reset();
    }

    void reset() noexcept
    {
        lfl_res_cap.reset();
        hfc_cap.reset();
        hfl_res_cap.reset();
        lfc_cap.reset();
    }

    float process (float V)
    {
        v_in.setVoltage (V);
        v_in.incident (s_i.reflected());
        s_i.incident (v_in.reflected());
        return chowdsp::wdft::voltage<float> (hfc_cap);
    }
};

int main()
{
    std::cout << "Pre-amp EQ test\n";

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

    if (max_error > 5.0e-5f)
    {
        std::cout << "Error is too large... failing test!\n";
        return 1;
    }

    return 0;
}
