#include "preamp_eq_comb.h"

#include "../chowdsp_wdf.h"
#include <chrono>
#include <iostream>
#include <fstream>
#include <random>

#include "../performance_counters/event_counter.h"

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

    static constexpr int N = 100;
    std::array<float, N> ref_output {};
    float max_error = 0.0f;
    for (int i = 0; i < N; ++i)
    {
        const auto test_output = process (state, impedances, 1.0f);
        ref_output[i] = ref.process (1.0f);
        const auto error = std::abs (test_output - ref_output[i]);
        max_error = std::max (error, max_error);
    }
    std::cout << "Max Error: " << max_error << '\n';

    if (max_error > 2.5e-7f)
    {
        std::cout << "Error is too large... failing test!\n";
        return 1;
    }

    std::ofstream ofp { "data.bin", std::ios::out | std::ios::binary };
    ofp.write(reinterpret_cast<const char*>(ref_output.data()), N * sizeof (float));
    ofp.close();

#if RUN_BENCH
    static constexpr int M = 10'000'000;
    static constexpr int n_iter = 10;

    auto* data_in = (float*) malloc (M * sizeof (float));
    auto* data_out = (float*) malloc (M * sizeof (float));

    std::random_device rd {};
    std::default_random_engine gen { rd() };
    std::uniform_real_distribution<float> dist { -1.0f, 1.0f };
    event_collector collector {};

    double ref_time, test_time;
    {
        event_aggregate aggregate {};
        float save_out = 0.0f;
        for (int iter = 0; iter < n_iter; ++iter)
        {
            for (int n = 0; n < M; ++n)
                data_in[n] = dist (gen);

            collector.start();
            for (int n = 0; n < M; ++n)
                data_out[n] = ref.process (data_in[n]);

            aggregate << collector.end();
            save_out += data_out[M-1];
        }
        std::cout << save_out << '\n';
        pretty_print (aggregate, M, "chowdsp_wdf");
        ref_time = aggregate.elapsed_ns();
    }

    {
        event_aggregate aggregate {};
        float save_out = 0.0f;
        for (int iter = 0; iter < n_iter; ++iter)
        {
            for (int n = 0; n < M; ++n)
                data_in[n] = dist (gen);

            collector.start();
            for (int n = 0; n < M; ++n)
                data_out[n] = process (state, impedances, data_in[n]);

            aggregate << collector.end();
            save_out += data_out[M-1];
        }
        std::cout << save_out << '\n';
        pretty_print (aggregate, M, "wdf_compiler");
        test_time = aggregate.elapsed_ns();
    }
    std::cout << "wdf_compiler is " << ref_time / test_time << "x faster\n";

    free (data_in);
    free (data_out);
#endif

    return 0;
}
