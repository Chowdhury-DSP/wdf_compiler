#include "include_rt_wdf.h"

#include <iostream>
#include <fstream>
#include <random>

#include "../performance_counters/event_counter.h"

struct Reference_WDF : public wdfTree
{
    std::unique_ptr<wdfTerminatedRes> lfl_res;
    std::unique_ptr<wdfTerminatedCap> lfl_cap;
    std::unique_ptr<wdfTerminatedParallel> lfl_res_cap;
    std::unique_ptr<wdfTerminatedRes> r_shunt;
    std::unique_ptr<wdfTerminatedSeries> s_s;

    std::unique_ptr<wdfTerminatedCap> hfc_cap;
    std::unique_ptr<wdfTerminatedParallel> p_s;

    std::unique_ptr<wdfTerminatedRes> hfl_res;
    std::unique_ptr<wdfTerminatedCap> hfl_cap;
    std::unique_ptr<wdfTerminatedSeries> hfl_res_cap;
    std::unique_ptr<wdfTerminatedRes> r_series;
    std::unique_ptr<wdfTerminatedParallel> p_p;

    std::unique_ptr<wdfTerminatedSeries> s_o;

    std::unique_ptr<wdfTerminatedCap> lfc_cap;
    std::unique_ptr<wdfTerminatedSeries> s_i;

    std::unique_ptr<wdfIdealVSource> v_in;

    Reference_WDF(double sampleRate)
    {
        lfl_res = std::make_unique<wdfTerminatedRes> (15.0e3);
        lfl_cap = std::make_unique<wdfTerminatedCap> (0.15e-6, sampleRate);
        lfl_res_cap = std::make_unique<wdfTerminatedParallel> (lfl_res.get(), lfl_cap.get());
        r_shunt = std::make_unique<wdfTerminatedRes> (4'674.0);
        s_s = std::make_unique<wdfTerminatedSeries> (lfl_res_cap.get(), r_shunt.get());

        hfc_cap = std::make_unique<wdfTerminatedCap> (3.3e-6, sampleRate);
        p_s = std::make_unique<wdfTerminatedParallel> (s_s.get(), hfc_cap.get());

        hfl_res = std::make_unique<wdfTerminatedRes> (80.0e3);
        hfl_cap = std::make_unique<wdfTerminatedCap> (2.7e-9, sampleRate);
        hfl_res_cap = std::make_unique<wdfTerminatedSeries> (hfl_res.get(), hfl_cap.get());
        r_series = std::make_unique<wdfTerminatedRes> (10.0);
        p_p = std::make_unique<wdfTerminatedParallel> (hfl_res_cap.get(), r_series.get());

        s_o = std::make_unique<wdfTerminatedSeries> (p_s.get(), p_p.get());

        lfc_cap = std::make_unique<wdfTerminatedCap> (4.7e-9, sampleRate);
        s_i = std::make_unique<wdfTerminatedSeries> (s_o.get(), lfc_cap.get());

        v_in = std::make_unique<wdfIdealVSource> (0.0);

        subtreeCount = 1;
        subtreeEntryNodes    = new wdfTreeNode*[subtreeCount];
        Rp                   = new double[subtreeCount];
        subtreeEntryNodes[0] = s_i.get();

        root = std::make_unique<wdfRootSimple> (v_in.get());

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

    const char* getTreeIdentifier() override { return "preamp eq"; }
    int setRootMatrData (matData* /*rootMats*/, double* /*Rp*/) override { return 0; }
    void setInputValue (double signalIn) override { v_in->Vs = signalIn; }
    double getOutputValue() override { return hfc_cap->upPort->getPortVoltage(); }
    void setParam (size_t /*paramID*/, double /*paramValue*/) override {}
};

int main()
{
    std::cout << "Pre-amp EQ test (RT-WDF)\n";

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
