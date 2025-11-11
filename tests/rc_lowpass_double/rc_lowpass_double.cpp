#include "rc_lowpass_double.h"

#include "../chowdsp_wdf.h"
#include <iostream>
#include <fstream>
#include <random>

#include "../performance_counters/event_counter.h"

struct Reference_WDF
{
    chowdsp::wdft::ResistorT<double> R1 { 1000.0 };
    chowdsp::wdft::CapacitorT<double> C1 { 1.0e-6 };
    chowdsp::wdft::WDFSeriesT<double, decltype (R1), decltype (C1)> S1 { R1, C1 };
    chowdsp::wdft::IdealVoltageSourceT<double, decltype (S1)> Vin { S1 };

    void prepare (double fs)
    {
        C1.prepare (fs);
    }

    double process (double V)
    {
        Vin.setVoltage (V);
        Vin.incident (S1.reflected());
        S1.incident (Vin.reflected());
        return chowdsp::wdft::voltage<double> (C1);
    }
};

int main()
{
    std::cout << "RC Lowpass test (double-precision)\n";

    static constexpr double fs = 48000.0;

    Reference_WDF ref {};
    ref.prepare (fs);

    Impedances impedances {};
    calc_impedances (impedances, fs);
    State state {};

    static constexpr int N = 100;
    std::array<double, N> ref_output {};
    double max_error = 0.0;
    for (int i = 0; i < 100; ++i)
    {
        const auto test_output = process (state, impedances, 1.0);
        ref_output[i] = ref.process (1.0);
        const auto error = std::abs (test_output - ref_output[i]);
        max_error = std::max (error, max_error);
    }
    std::cout << "Max Error: " << max_error << '\n';

    if (max_error > 1.0e-6)
    {
        std::cout << "Error is too large... failing test!\n";
        return 1;
    }

    std::ofstream ofp { "data.bin", std::ios::out | std::ios::binary };
    ofp.write(reinterpret_cast<const char*>(ref_output.data()), N * sizeof (double));
    ofp.close();

#if RUN_BENCH
    static constexpr int M = 100'000'000;
    static constexpr int n_iter = 4;

    auto* data_in = (double*) malloc (M * sizeof (double));
    auto* data_out = (double*) malloc (M * sizeof (double));

    std::random_device rd {};
    std::default_random_engine gen { rd() };
    std::uniform_real_distribution<double> dist { -1.0, 1.0 };
    event_collector collector {};

    double ref_time, test_time;
    {
        event_aggregate aggregate {};
        double save_out = 0.0;
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
        double save_out = 0.0;
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
