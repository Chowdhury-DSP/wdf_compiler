#include "include_rt_wdf.h"

#include <iostream>
#include <fstream>
#include <random>

#include "../performance_counters/event_counter.h"

struct Reference_WDF : public wdfTree
{
    struct Dangelo_Triode : public wdfRoot
    {
        std::unique_ptr<matData> rootMatrixData;

        Dangelo_Triode()
        {
            rootMatrixData.reset( new matData );
            rootMatrixData->Smat.set_size( 3, 1 );
            rootMatrixData->Emat.set_size(0, 0);
            rootMatrixData->Fmat.set_size(0, 0);
            rootMatrixData->Mmat.set_size(0, 0);
            rootMatrixData->Nmat.set_size(0, 0);
        }

        void setPortResistances( double *Rp ) override
        {
            rootMatrixData->Smat.at(0, 0) = Rp[0];
            rootMatrixData->Smat.at(1, 0) = Rp[1];
            rootMatrixData->Smat.at(2, 0) = Rp[2];
        }

        void processAscendingWaves(vec* ascendingWaves,
                                   vec* descendingWaves) override
        {
            const auto ag = (*ascendingWaves)[0];
            const auto ak = (*ascendingWaves)[1];
            const auto ap = (*ascendingWaves)[2];
            const auto R0k = rootMatrixData->Smat.at(1, 0);
            const auto R0p = rootMatrixData->Smat.at(2, 0);

            static constexpr float kp = 1.014e-5f;
            static constexpr float kpg = 1.076e-5f;
            static constexpr float kp2 = 5.498e-8f;

            const auto bk_bp = R0k / R0p;
            const auto k_eta = 1.0f / (bk_bp * (0.5f * kpg + kp2) + kp2);
            const auto k_delta = kp2 * k_eta * k_eta / (R0p + R0p);
            const auto k_bp_s = k_eta * std::sqrt ((kp2 + kp2) / R0p);
            const auto bp_k = 1.0f / (R0p + R0k);
            const auto bp_ap_0 = bp_k * (R0k - R0p);
            const auto bp_ak_0 = bp_k * (R0p + R0p);

            // everything before this can be pre-computed
            const auto v1 = 0.5f * ap;
            const auto v2 = ak + v1 * bk_bp;
            const auto alpha = kpg * (ag - v2) + kp;
            const auto beta = kp2 * (v1 - v2);
            const auto eta = k_eta * (beta + beta + alpha);
            const auto v3 = eta + k_delta;
            const auto delta = ap + v3;

            float bg, bk, bp, Vpk;
            if (delta >= 0.0f)
            {
                bp = k_bp_s * std::sqrt (delta) - v3 - k_delta;
                const auto d = bk_bp * (ap - bp);
                bk = ak + d;
                const auto Vpk2 = ap + bp - ak - bk;

                if (kpg * (ag - ak - 0.5f * d) + kp2 * Vpk2 + kp < 0.0f)
                {
                    bp = ap;
                    bk = ak;
                    Vpk = ap - ak;
                }
                else
                {
                    Vpk = 0.5f * Vpk2;
                }
            }
            else
            {
                bp = ap;
                bk = ak;
                Vpk = ap - ak;
            }

            if (Vpk < 0.0f)
                bp = bp_ap_0 * ap + bp_ak_0 * ak;

            bg = ag;

            (*descendingWaves)[0] = bg;
            (*descendingWaves)[1] = bk;
            (*descendingWaves)[2] = bp;
        }

        std::string getType() const override { return "D'Angelo Triode"; }
    };

    std::unique_ptr<wdfTerminatedResVSource> Vin;
    std::unique_ptr<wdfTerminatedRes> Rk;
    std::unique_ptr<wdfTerminatedResVSource> Vplus;
    std::unique_ptr<wdfTerminatedRes> Rl;
    std::unique_ptr<wdfTerminatedCap> Cp;
    std::unique_ptr<wdfTerminatedSeries> Sp;
    std::unique_ptr<wdfTerminatedParallel> Pp;

    Reference_WDF(double sampleRate)
    {
        Vin = std::make_unique<wdfTerminatedResVSource> (0.0, 25.0e3);
        Rk = std::make_unique<wdfTerminatedRes> (200.0);
        Vplus = std::make_unique<wdfTerminatedResVSource> (200.0f, 47.0e3);
        Rl = std::make_unique<wdfTerminatedRes> (330.0e3);
        Cp = std::make_unique<wdfTerminatedCap> (0.047e-6, sampleRate);
        Sp = std::make_unique<wdfTerminatedSeries> (Rl.get(), Cp.get());
        Pp = std::make_unique<wdfTerminatedParallel> (Vplus.get(), Sp.get());

        subtreeCount = 3;
        subtreeEntryNodes    = new wdfTreeNode*[subtreeCount];
        Rp                   = new double[subtreeCount];
        subtreeEntryNodes[0] = Vin.get();
        subtreeEntryNodes[1] = Rk.get();
        subtreeEntryNodes[2] = Pp.get();

        root = std::make_unique<Dangelo_Triode>();

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

    const char* getTreeIdentifier() override { return "simple triode"; }
    int setRootMatrData (matData* /*rootMats*/, double* /*Rp*/) override { return 0; }
    void setInputValue (double signalIn) override { Vin->Vs = signalIn; }
    double getOutputValue() override { return Rl->upPort->getPortVoltage(); }
    void setParam (size_t /*paramID*/, double /*paramValue*/) override {}
};

int main()
{
    std::cout << "Simple Triode test (RT-WDF)\n";

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
