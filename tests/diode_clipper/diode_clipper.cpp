#include "diode_clipper.h"

#include "../chowdsp_wdf.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>

#include "../performance_counters/event_counter.h"

// clang diode_clipper.cpp --std=c++20 -lstdc++ -I../../lib -o diode_clipper.exe && ./diode_clipper.exe

struct Reference_WDF
{
    chowdsp::wdft::ResistiveVoltageSourceT<float> Vin { 1000.0f };
    chowdsp::wdft::CapacitorT<float> C1 { 1.0e-6f };
    chowdsp::wdft::WDFParallelT<float, decltype (Vin), decltype (C1)> P1 { Vin, C1 };
    chowdsp::wdft::DiodePairT<float, decltype (P1)> DP { P1, 1.0e-9f };

    void prepare (float fs)
    {
        C1.prepare (fs);
    }

    float process (float V)
    {
        Vin.setVoltage (V);
        DP.incident (P1.reflected());
        P1.incident (DP.reflected());
        return chowdsp::wdft::voltage<float> (C1);
    }
};

int main()
{
    std::cout << "Diode Clipper test\n";

    static constexpr float fs = 48000.0f;

    Reference_WDF ref {};
    ref.prepare (fs);

    Params params {
        .C1_value = 1.0e-6f,
        .Vin_res_value = 1.0e3f,
        .DP_params = {
            .Is = 1.0e-9f,
        },
    };
    Impedances impedances {};
    calc_impedances (impedances, fs, params);
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

    if (max_error > 1.0e-3f)
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
