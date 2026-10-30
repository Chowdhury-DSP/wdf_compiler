#include "include_rt_wdf.h"

#include <iostream>
#include <fstream>
#include <random>

#include "../performance_counters/event_counter.h"

struct Reference_WDF : public wdfTree
{
    std::unique_ptr<wdfTerminatedRes> R1, R2, R3, R4, R5, R6, R7, R8, Rl, Rv1, Rv2;
    std::unique_ptr<wdfTerminatedCap> C1, C2, C3, C4, C5, C6, C7, C8;
    std::unique_ptr<wdfTerminatedSeries> S1, S2, S3, S4, S5, S6, S7, S8, Sl, Sv1, Sv2;
    std::unique_ptr<wdfTerminatedParallel> P1, P2, P3, P4, P5, P6, P7;
    std::unique_ptr<wdfIdealVSource> Vin;

    Reference_WDF(double sampleRate)
    {
        // Resistor combos
        R1 = std::make_unique<wdfTerminatedRes> (1.0e3);
        R2 = std::make_unique<wdfTerminatedRes> (5.0e3);
        S1 = std::make_unique<wdfTerminatedSeries> (R1.get(), R2.get());
        R3 = std::make_unique<wdfTerminatedRes> (4.0e3);
        S2 = std::make_unique<wdfTerminatedSeries> (S1.get(), R3.get());
        R4 = std::make_unique<wdfTerminatedRes> (10.0e3);
        R5 = std::make_unique<wdfTerminatedRes> (3.0e3);
        P1 = std::make_unique<wdfTerminatedParallel> (R4.get(), R5.get());
        P2 = std::make_unique<wdfTerminatedParallel> (S2.get(), P1.get());

        // Capacitor combos
        C1 = std::make_unique<wdfTerminatedCap> (1.0e-6, sampleRate);
        C2 = std::make_unique<wdfTerminatedCap> (5.0e-6, sampleRate);
        S4 = std::make_unique<wdfTerminatedSeries> (C1.get(), C2.get());
        C3 = std::make_unique<wdfTerminatedCap> (4.0e-9, sampleRate);
        S5 = std::make_unique<wdfTerminatedSeries> (S4.get(), C3.get());
        C4 = std::make_unique<wdfTerminatedCap> (10.0e-9, sampleRate);
        C5 = std::make_unique<wdfTerminatedCap> (3.0e-6, sampleRate);
        P3 = std::make_unique<wdfTerminatedParallel> (C4.get(), C5.get());
        P4 = std::make_unique<wdfTerminatedParallel> (S5.get(), P3.get());

        // join resistor combos + capacitor combos
        S3 = std::make_unique<wdfTerminatedSeries> (P2.get(), P4.get());

        // RCSeries + R, RCSeries + C
        R6 = std::make_unique<wdfTerminatedRes> (10.0e3);
        S6 = std::make_unique<wdfTerminatedSeries> (S3.get(), R6.get());
        C6 = std::make_unique<wdfTerminatedCap> (4.0e-6, sampleRate);
        S7 = std::make_unique<wdfTerminatedSeries> (S6.get(), C6.get());

        // RCParallel, RCParallel + R, RCParallel + C
        R7 = std::make_unique<wdfTerminatedRes> (10.0e3);
        C7 = std::make_unique<wdfTerminatedCap> (10.0e-6, sampleRate);
        P5 = std::make_unique<wdfTerminatedParallel> (R7.get(), C7.get());
        R8 = std::make_unique<wdfTerminatedRes> (100.0e3);
        P6 = std::make_unique<wdfTerminatedParallel> (P5.get(), R8.get());
        C8 = std::make_unique<wdfTerminatedCap> (100.0e-9, sampleRate);
        P7 = std::make_unique<wdfTerminatedParallel> (P6.get(), C8.get());

        // RCSeries + RCParallel
        S8 = std::make_unique<wdfTerminatedSeries> (S7.get(), P7.get());

        // Load
        Rl = std::make_unique<wdfTerminatedRes> (100.0e3);
        Sl = std::make_unique<wdfTerminatedSeries> (Rl.get(), S8.get());
        Rv2 = std::make_unique<wdfTerminatedRes> (5.0e3);
        Sv2 = std::make_unique<wdfTerminatedSeries> (Rv2.get(), Sl.get());
        Rv1 = std::make_unique<wdfTerminatedRes> (10.0e3);
        Sv1 = std::make_unique<wdfTerminatedSeries> (Rv1.get(), Sv2.get());

        Vin = std::make_unique<wdfIdealVSource> (0.0);

        subtreeCount = 1;
        subtreeEntryNodes    = new wdfTreeNode*[subtreeCount];
        Rp                   = new double[subtreeCount];
        subtreeEntryNodes[0] = Sv1.get();

        root = std::make_unique<wdfRootSimple> (Vin.get());

        initTree();
        setSamplerate (sampleRate);
        adaptTree();
    }

    ~Reference_WDF() override
    {
        delete[] subtreeEntryNodes;
        delete[] Rp;
    }

    float process (float V)
    {
        setInputValue ((double) V);
        cycleWave();
        return (float) getOutputValue();
    }

    const char* getTreeIdentifier() override { return "reductions circuit"; }
    int setRootMatrData (matData* /*rootMats*/, double* /*Rp*/) override { return 0; }
    void setInputValue (double signalIn) override { Vin->Vs = signalIn; }
    double getOutputValue() override { return Rl->upPort->getPortVoltage(); }
    void setParam (size_t /*paramID*/, double /*paramValue*/) override {}
};

int main()
{
    std::cout << "Reductions Circuit test (RT-WDF)\n";

    static constexpr float fs = 48000.0;
    Reference_WDF ref { fs };

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
        pretty_print (aggregate, M, "RT-WDF");
    }

    free (data_in);
    free (data_out);

    return 0;
}
