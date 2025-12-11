#include "reductions_circuit.h"

#include "../chowdsp_wdf.h"
#include <iostream>
#include <fstream>
#include <random>

#include "../performance_counters/event_counter.h"

struct Reference_WDF
{
    // Resistor combos
    chowdsp::wdft::ResistorT<float> R1 { 1.0e3f };
    chowdsp::wdft::ResistorT<float> R2 { 5.0e3f };
    chowdsp::wdft::WDFSeriesT<float, decltype (R1), decltype (R2)> S1 { R1, R2 };

    chowdsp::wdft::ResistorT<float> R3 { 4.0e3f };
    chowdsp::wdft::WDFSeriesT<float, decltype (S1), decltype (R3)> S2 { S1, R3 };

    chowdsp::wdft::ResistorT<float> R4 { 10.0e3f };
    chowdsp::wdft::ResistorT<float> R5 { 3.0e3f };
    chowdsp::wdft::WDFParallelT<float, decltype (R4), decltype (R5)> P1 { R4, R5 };

    chowdsp::wdft::WDFParallelT<float, decltype (S2), decltype (P1)> P2 { S2, P1 };

    // Capacitor combos
    chowdsp::wdft::CapacitorT<float> C1 { 1.0e-6f };
    chowdsp::wdft::CapacitorT<float> C2 { 5.0e-6f };
    chowdsp::wdft::WDFSeriesT<float, decltype (C1), decltype (C2)> S4 { C1, C2 };

    chowdsp::wdft::CapacitorT<float> C3 { 4.0e-9f };
    chowdsp::wdft::WDFSeriesT<float, decltype (S4), decltype (C3)> S5 { S4, C3 };

    chowdsp::wdft::CapacitorT<float> C4 { 10.0e-9f };
    chowdsp::wdft::CapacitorT<float> C5 { 3.0e-6f };
    chowdsp::wdft::WDFParallelT<float, decltype (C4), decltype (C5)> P3 { C4, C5 };

    chowdsp::wdft::WDFParallelT<float, decltype (S5), decltype (P3)> P4 { S5, P3 };

    // join resistor combos + capacitor combos
    chowdsp::wdft::WDFSeriesT<float, decltype (P2), decltype (P4)> S3 { P2, P4 };

    // Load
    chowdsp::wdft::ResistorT<float> Rl { 100.0e3f };
    chowdsp::wdft::WDFSeriesT<float, decltype (Rl), decltype (S3)> Sl { Rl, S3 };
    chowdsp::wdft::IdealVoltageSourceT<float, decltype (Sl)> Vin { Sl };

    void prepare (float fs)
    {
    }

    float process (float V)
    {
        Vin.setVoltage (V);
        Vin.incident (Sl.reflected());
        Sl.incident (Vin.reflected());
        return chowdsp::wdft::voltage<float> (Rl);
    }
};

int main()
{
    std::cout << "Reductions Circuit test\n";

    static constexpr float fs = 48000.0f;

    Reference_WDF ref {};
    ref.prepare (fs);

    Impedances impedances {};
    calc_impedances (
        impedances,
        fs,
        {
            .R2_value = ref.R2.wdf.R,
            .R4_value = ref.R4.wdf.R,
            .C2_value = ref.C2.C_value,
            .C4_value = ref.C4.C_value,
    });
    State state {};

    static constexpr int N = 100;
    std::array<float, N> ref_output {};
    float max_error = 0.0f;
    for (int i = 0; i < 100; ++i)
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
    static constexpr int M = 100'000'000;
    static constexpr int n_iter = 4;

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
