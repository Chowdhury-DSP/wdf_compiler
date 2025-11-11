#include "include_rt_wdf.h"

#include <iostream>
#include <fstream>
#include <random>

#include "../performance_counters/event_counter.h"

struct Reference_WDF : public wdfTree
{
    std::unique_ptr<wdfTerminatedRes> R1;
    std::unique_ptr<wdfTerminatedCap> C1;
    std::unique_ptr<wdfTerminatedSeries> S1;
    std::unique_ptr<wdfIdealVSource> Vin;

    Reference_WDF(double sampleRate)
    {
        subtreeCount = 1;

        R1 = std::make_unique<wdfTerminatedRes> (1.0e3);
        C1 = std::make_unique<wdfTerminatedCap> (1.0e-6, sampleRate);
        S1 = std::make_unique<wdfTerminatedSeries> (C1.get(), R1.get());
        Vin = std::make_unique<wdfIdealVSource> (0.0);

        subtreeEntryNodes    = new wdfTreeNode*[subtreeCount];
        Rp                   = new double[subtreeCount];
        subtreeEntryNodes[0] = S1.get();

        root = std::make_unique<wdfRootSimple> (Vin.get());

        initTree();
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

    const char* getTreeIdentifier() override { return "lpf"; }
    int setRootMatrData (matData* /*rootMats*/, double* /*Rp*/) override { return 0; }
    void setInputValue (double signalIn) override { Vin->Vs = signalIn; }
    double getOutputValue() override { return C1->upPort->getPortVoltage(); }
    void setParam (size_t /*paramID*/, double /*paramValue*/) override {}
};

int main()
{
    std::cout << "RC Lowpass test (RT-WDF)\n";

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
        pretty_print (aggregate, M, "RT-WDF");
        ref_time = aggregate.elapsed_ns();
    }

    free (data_in);
    free (data_out);

    return 0;
}
