#include "reductions_circuit2.h"

#include "../chowdsp_wdf.h"
#include <iostream>
#include <fstream>
#include <random>

#include "../performance_counters/event_counter.h"

struct Reference_WDF
{
    chowdsp::wdft::ResistorT<float> R1 { 10.0e3f };
    chowdsp::wdft::ResistiveVoltageSourceT<float> Vin { 5.0e3f };
    chowdsp::wdft::WDFSeriesT<float, decltype (R1), decltype (Vin)> S1 { R1, Vin };

    chowdsp::wdft::ResistorT<float> R2 { 11.0e3f };
    chowdsp::wdft::WDFSeriesT<float, decltype (R2), decltype (S1)> S2 { R2, S1 };

    chowdsp::wdft::CapacitorT<float> Cp { 4.0e-6f };
    chowdsp::wdft::WDFSeriesT<float, decltype (Cp), decltype (S2)> S5 { Cp, S2 };

    chowdsp::wdft::ResistorT<float> Rp2 { 7.0e3f };
    chowdsp::wdft::WDFSeriesT<float, decltype (Rp2), decltype (S5)> S8 { Rp2, S5 };

    chowdsp::wdft::CapacitorT<float> Cp2 { 7.0e-9f };
    chowdsp::wdft::WDFSeriesT<float, decltype (Cp2), decltype (S8)> S7 { Cp2, S8 };

    chowdsp::wdft::CapacitiveVoltageSourceT<float> Vcc { 1.0e-6f };
    chowdsp::wdft::CapacitorT<float> C1 { 10.0e-9f };
    chowdsp::wdft::WDFSeriesT<float, decltype (Vcc), decltype (C1)> S4 { Vcc, C1 };

    chowdsp::wdft::ResistorT<float> Rp { 4.0e3f };
    chowdsp::wdft::WDFSeriesT<float, decltype (Rp), decltype (S4)> S6 { Rp, S4 };

    chowdsp::wdft::WDFSeriesT<float, decltype (S6), decltype (S7)> S3 { S6, S7 };

    chowdsp::wdft::ResistiveCapacitiveVoltageSourceT<float> Rl { 100.0e3f, 10.0e-6f };
    chowdsp::wdft::WDFSeriesT<float, decltype (Rl), decltype (S3)> Sl { Rl, S3 };
    chowdsp::wdft::IdealVoltageSourceT<float, decltype (Sl)> Vb { Sl };

    void prepare (float fs)
    {
        Vb.setVoltage (1.5f);
        Vcc.setVoltage (1.2f);
        Rl.setVoltage (-1.1f);
    }

    float process (float V)
    {
        Vin.setVoltage (V);
        Vb.incident (Sl.reflected());
        Sl.incident (Vb.reflected());
        return chowdsp::wdft::voltage<float> (Rl);
    }
};

int main()
{
    std::cout << "Reductions Circuit 2 test\n";

    static constexpr float fs = 48000.0f;

    Reference_WDF ref {};
    ref.prepare (fs);

    Impedances impedances {};
    calc_impedances (
        impedances,
        fs,
        {
            .Vcc_cap_value = 1.0e-6f,
            .R2_value = 11.0e3f,
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
    double ref_cycles, test_cycles;
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
        ref_cycles = aggregate.best.cycles();
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
        test_cycles = aggregate.best.cycles();
    }
    std::cout << "wdf_compiler is " << ref_time / test_time << "x faster (time)\n";
    std::cout << "wdf_compiler is " << ref_cycles / test_cycles << "x faster (cycles)\n";

    free (data_in);
    free (data_out);
#endif

    return 0;
}
