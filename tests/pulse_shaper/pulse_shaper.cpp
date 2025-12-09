#include "pulse_shaper.h"

#include "../chowdsp_wdf.h"
#include <iostream>
#include <fstream>
#include <random>

#include "../performance_counters/event_counter.h"

struct Reference_WDF
{
    using Resistor = chowdsp::wdft::ResistorT<float>;
    using Capacitor = chowdsp::wdft::CapacitorAlphaT<float>;
    using ResVs = chowdsp::wdft::ResistiveVoltageSourceT<float>;

    ResVs Vs { 1.0e-9f };
    Resistor r162 { 4700.0f };
    Resistor r163 { 100000.0f };
    Capacitor c40 { 0.015e-6f, 48000.0f, 0.029f };

    chowdsp::wdft::WDFParallelT<float, Capacitor, Resistor> P1 { c40, r163 };
    chowdsp::wdft::WDFSeriesT<float, ResVs, decltype (P1)> S1 { Vs, P1 };
    chowdsp::wdft::PolarityInverterT<float, Resistor> I1 { r162 };
    chowdsp::wdft::WDFParallelT<float, decltype (I1), decltype (S1)> P2 { I1, S1 };

    struct O
    {
        template <typename T>
        static T omega (T x)
        {
            return chowdsp::Omega::omega3 (x);
        }
    };
    chowdsp::wdft::DiodeT<float, decltype (P2), chowdsp::wdft::DiodeQuality::Best, O> d53 { P2, 2.52e-9f }; // 1N4148 diode

    void prepare (float fs)
    {
        c40.prepare (fs);
    }

    float process (float V)
    {
        Vs.setVoltage (V);
        d53.incident (P2.reflected());
        P2.incident (d53.reflected());
        auto y = chowdsp::wdft::voltage<float> (r162);
        return y;
    }
};

int main()
{
    std::cout << "Pulse Shaper test\n";

    static constexpr float fs = 48000.0f;

    Reference_WDF ref {};
    ref.prepare (fs);

    Impedances impedances {};
    calc_impedances (impedances, fs, { .d53_params { .Is = 2.52e-9f } });
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
