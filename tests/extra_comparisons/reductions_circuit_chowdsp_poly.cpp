#include "../chowdsp_wdf.h"
#include <iostream>
#include <fstream>
#include <random>

#include "../performance_counters/event_counter.h"

struct Reference_WDF
{
    // Resistor combos
    chowdsp::wdf::Resistor<float> R1 { 1.0e3f };
    chowdsp::wdf::Resistor<float> R2 { 5.0e3f };
    chowdsp::wdf::WDFSeries<float> S1 { &R1, &R2 };

    chowdsp::wdf::Resistor<float> R3 { 4.0e3f };
    chowdsp::wdf::WDFSeries<float> S2 { &S1, &R3 };

    chowdsp::wdf::Resistor<float> R4 { 10.0e3f };
    chowdsp::wdf::Resistor<float> R5 { 3.0e3f };
    chowdsp::wdf::WDFParallel<float> P1 { &R4, &R5 };

    chowdsp::wdf::WDFParallel<float> P2 { &S2, &P1 };

    // Capacitor combos
    chowdsp::wdf::Capacitor<float> C1 { 1.0e-6f };
    chowdsp::wdf::Capacitor<float> C2 { 5.0e-6f };
    chowdsp::wdf::WDFSeries<float> S4 { &C1, &C2 };

    chowdsp::wdf::Capacitor<float> C3 { 4.0e-9f };
    chowdsp::wdf::WDFSeries<float> S5 { &S4, &C3 };

    chowdsp::wdf::Capacitor<float> C4 { 10.0e-9f };
    chowdsp::wdf::Capacitor<float> C5 { 3.0e-6f };
    chowdsp::wdf::WDFParallel<float> P3 { &C4, &C5 };

    chowdsp::wdf::WDFParallel<float> P4 { &S5, &P3 };

    // join resistor combos + capacitor combos
    chowdsp::wdf::WDFSeries<float> S3 { &P2, &P4 };

    // RCSeries + R
    chowdsp::wdf::Resistor<float> R6 { 10.0e3f };
    chowdsp::wdf::WDFSeries<float> S6 { &S3, &R6 };

    // RCSeries + C
    chowdsp::wdf::Capacitor<float> C6 { 4.0e-6f };
    chowdsp::wdf::WDFSeries<float> S7 { &S6, &C6 };

    // RCParallel
    chowdsp::wdf::Resistor<float> R7 { 10.0e3f };
    chowdsp::wdf::Capacitor<float> C7 { 10.0e-6f };
    chowdsp::wdf::WDFParallel<float> P5 { &R7, &C7 };

    // RCParallel + R
    chowdsp::wdf::Resistor<float> R8 { 100.0e3f };
    chowdsp::wdf::WDFParallel<float> P6 { &P5, &R8 };

    // RCParallel + C
    chowdsp::wdf::Capacitor<float> C8 { 100.0e-9f };
    chowdsp::wdf::WDFParallel<float> P7 { &P6, &C8 };

    // RCSeries + RCParallel
    chowdsp::wdf::WDFSeries<float> S8 { &S7, &P7 };

    // Load
    chowdsp::wdf::Resistor<float> Rl { 100.0e3f };
    chowdsp::wdf::WDFSeries<float> Sl { &Rl, &S8 };

    chowdsp::wdf::Resistor<float> Rv2 { 5.0e3f };
    chowdsp::wdf::WDFSeries<float> Sv2 { &Rv2, &Sl };

    chowdsp::wdf::Resistor<float> Rv1 { 10.0e3f };
    chowdsp::wdf::WDFSeries<float> Sv1 { &Rv1, &Sv2 };

    chowdsp::wdf::IdealVoltageSource<float> Vin { &Sv1 };

    void prepare (float fs)
    {
        for (auto* C : { &C1, &C2, &C3, &C4, &C5, &C6, &C7, &C8 })
            C->prepare (fs);
    }

    float process (float V)
    {
        Vin.setVoltage (V);
        Vin.incident (Sv1.reflected());
        Sv1.incident (Vin.reflected());
        return Rl.voltage();
    }
};

int main()
{
    std::cout << "Reductions Circuit test (chowdsp_wdf polymorphic)\n";

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
    }

    free (data_in);
    free (data_out);

    return 0;
}
