#include "simple_triode.h"

#include "../chowdsp_wdf.h"
#include <iostream>
#include <fstream>
#include <random>

#include "../performance_counters/event_counter.h"

struct Reference_WDF
{
    // Reference: https://dafx.de/paper-archive/2023/DAFx23_paper_15.pdf

    // Grid circuit
    chowdsp::wdft::ResistiveVoltageSourceT<float> Vin { 25.0e3f };

    // Cathode circuit
    chowdsp::wdft::ResistorT<float> Rk { 200.0f };

    // Plate circuit
    chowdsp::wdft::ResistiveVoltageSourceT<float> Vplus { 47.0e3f };

    chowdsp::wdft::ResistorT<float> Rl { 330.0e3f };
    chowdsp::wdft::CapacitorT<float> Cp { 0.047e-6f };
    chowdsp::wdft::WDFSeriesT<float, decltype (Rl), decltype (Cp)> Sp { Rl, Cp };

    chowdsp::wdft::WDFParallelT<float, decltype (Vplus), decltype (Sp)> Pp { Vplus, Sp };

    void prepare (float fs)
    {
        Vplus.setVoltage (200.0f);
        Cp.prepare (fs);
        Cp.incident (-118.959396f);
    }

    static inline auto triode (float ag, float ak, float ap, float R0k, float R0p)
    {
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

        return std::make_tuple (bg, bk, bp);
    }

    float process (float V)
    {
        Vin.setVoltage (V);

        const auto wT_ag = Vin.reflected();
        const auto wT_ak = Rk.reflected();
        const auto wT_ap = Pp.reflected();

        const auto [wT_bg, wT_bk, wT_bp] = triode (wT_ag, wT_ak, wT_ap, Rk.wdf.R, Pp.wdf.R);

        Vin.incident (wT_bg);
        Rk.incident (wT_bk);
        Pp.incident (wT_bp);

        return chowdsp::wdft::voltage<float> (Rl);
    }
};

int main()
{
    std::cout << "Simple Triode test\n";

    static constexpr float fs = 48000.0f;

    Reference_WDF ref {};
    ref.prepare (fs);

    Impedances impedances {};
    calc_impedances (impedances, fs);
    State state {
        .Cp_z = -118.959396f,
    };

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
    static constexpr int M = 10'000'000;
    static constexpr int n_iter = 10;

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
