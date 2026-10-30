#include "include_rt_wdf.h"
#include <wdf_lib_diode_pair.h>

#include <iostream>
#include <fstream>
#include <random>

#include "../performance_counters/event_counter.h"

// "Custom" RT-WDF root node, using the diode pair from the wdf_compiler runtime library.
// Note that the library uses single-precision float while RT-WDF uses double-precision.
struct wdfDiodePairRoot : public wdfRootNode
{
    wdf_lib::diode_pair::Diode_Pair_Params params;
    wdf_lib::diode_pair::Diode_Pair_Vars vars {};

    explicit wdfDiodePairRoot (wdf_lib::diode_pair::Diode_Pair_Params params)
        : wdfRootNode (1), params (params)
    {
    }

    void setPortResistance (double Rp_new) override
    {
        Rp = Rp_new;
        wdf_lib::diode_pair::update_vars (&vars, &params, (float) Rp, (float) (1.0 / Rp));
    }

    void calculateDownB (vec* ascendingWaves, vec* descendingWaves, size_t* portIndex) override
    {
        const auto a = (float) (*ascendingWaves) (*portIndex);
        (*descendingWaves) (*portIndex) = (double) wdf_lib::diode_pair::root_compute (&vars, a);
        (*portIndex) += numPorts;
    }

    std::string getType() const override { return "Diode Pair (wdf_lib)"; }
};

struct Reference_WDF : public wdfTree
{
    std::unique_ptr<wdfTerminatedResVSource> Vin;
    std::unique_ptr<wdfTerminatedCap> C1;
    std::unique_ptr<wdfTerminatedParallel> P1;
    std::unique_ptr<wdfDiodePairRoot> DP;

    Reference_WDF(double sampleRate)
    {
        Vin = std::make_unique<wdfTerminatedResVSource> (0.0, 1.0e3);
        C1 = std::make_unique<wdfTerminatedCap> (1.0e-6, sampleRate);
        P1 = std::make_unique<wdfTerminatedParallel> (Vin.get(), C1.get());
        DP = std::make_unique<wdfDiodePairRoot> (wdf_lib::diode_pair::Diode_Pair_Params {
            .Is = 1.0e-9f,
            .Vt = 25.85e-3f,
            .nabla = 1.0f,
        });

        subtreeCount = 1;
        subtreeEntryNodes    = new wdfTreeNode*[subtreeCount];
        Rp                   = new double[subtreeCount];
        subtreeEntryNodes[0] = P1.get();

        root = std::make_unique<wdfRootSimple> (DP.get());

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

    const char* getTreeIdentifier() override { return "diode clipper"; }
    int setRootMatrData (matData* /*rootMats*/, double* /*Rp*/) override { return 0; }
    void setInputValue (double signalIn) override { Vin->Vs = signalIn; }
    double getOutputValue() override { return C1->upPort->getPortVoltage(); }
    void setParam (size_t /*paramID*/, double /*paramValue*/) override {}
};

int main()
{
    std::cout << "Diode Clipper test (RT-WDF)\n";

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
