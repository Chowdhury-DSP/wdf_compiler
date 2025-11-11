#include "../chowdsp_wdf.h"
#include <iostream>
#include <fstream>
#include <random>

#include "../performance_counters/event_counter.h"

struct Reference_WDF
{
    chowdsp::wdf::ResistiveVoltageSource<float> Vin { 1000.0f };
    chowdsp::wdf::Capacitor<float> C1 { 1.0e-6f };
    chowdsp::wdf::WDFParallel<float> P1 { &Vin, &C1 };
    chowdsp::wdf::DiodePair<float> DP { &P1, 1.0e-9f };

    void prepare (float fs)
    {
        C1.prepare (fs);
    }

    float process (float V)
    {
        Vin.setVoltage (V);
        DP.incident (P1.reflected());
        P1.incident (DP.reflected());
        return C1.voltage();
    }
};

int main()
{
    std::cout << "Diode Clipper test (chowdsp_wdf polymorphic)\n";

    static constexpr float fs = 48000.0f;

    Reference_WDF ref {};
    ref.prepare (fs);

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
        pretty_print (aggregate, M, "chowdsp_wdf poly");
        ref_time = aggregate.elapsed_ns();
    }

    free (data_in);
    free (data_out);

    return 0;
}
