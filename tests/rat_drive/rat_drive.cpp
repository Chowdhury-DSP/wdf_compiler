#include "rat_drive.h"

#if RUN_BENCH
#include <xsimd/xsimd.hpp>
#include "../performance_counters/event_counter.h"
#endif

#include "../chowdsp_wdf.h"
#include <iostream>
#include <fstream>
#include <random>
#include <tuple>

namespace wdft = chowdsp::wdft;

struct Reference_WDF
{
    void prepare (double sampleRate)
    {
        Vin_C1.prepare ((float) sampleRate);
        C2.prepare ((float) sampleRate);
        Rd_C4.prepare ((float) sampleRate);
        R4_C5.prepare ((float) sampleRate);
        R5_C6.prepare ((float) sampleRate);
        R6_C7.prepare ((float) sampleRate);

        R2.setVoltage (4.5f);
    }

    inline float process (float x) noexcept
    {
        Vin_C1.setVoltage (x);
        diodes.incident (Sd.reflected());
        Sd.incident (diodes.reflected());
        const auto y = wdft::voltage<float> (diodes);
        return y;
    }

    // Port A
    wdft::CapacitiveVoltageSourceT<float> Vin_C1 { 22.0e-9f };
    wdft::ResistiveVoltageSourceT<float> R2 { 1.0e6f };
    wdft::WDFParallelT<float, decltype (Vin_C1), decltype (R2)> P1 { Vin_C1, R2 };

    wdft::ResistorT<float> R3 { 1.0e3f };
    wdft::WDFSeriesT<float, decltype (P1), decltype (R3)> S2 { P1, R3 };

    wdft::CapacitorT<float> C2 { 1.0e-9f };
    wdft::WDFParallelT<float, decltype (S2), decltype (C2)> Pa { S2, C2 };

    // Port B
    wdft::ResistorCapacitorSeriesT<float> R4_C5 { 47.0f, 2.2e-6f };
    wdft::ResistorCapacitorSeriesT<float> R5_C6 { 560.0f, 4.7e-6f };
    wdft::WDFParallelT<float, decltype (R4_C5), decltype (R5_C6)> Pb { R4_C5, R5_C6 };

    // Port C
    static constexpr float Rdistortion = 100.0e3f;
    wdft::ResistorCapacitorParallelT<float> Rd_C4 { 0.5f * Rdistortion, 100.0e-12f };

    // R-Type
    struct ImpedanceCalc
    {
        template <typename RType>
        static float calcImpedance (RType& R)
        {
            constexpr float Ag = 100.0f; // op-amp gain
            constexpr float Ri = 10.0e6f; // op-amp input impedance
            constexpr float Ro = 1.0e-1f; // op-amp output impedance

            const auto [Ra, Rb, Rc] = R.getPortImpedances();
            const auto Rd = -(((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro) / (Ra * Rb + Ra * Rc + Rb * Rc + Rb * Ri + Ag * Rb * Ri + Rc * Ri - (Ra + Rb + Ri) * Ro));

            R.setSMatrixData ({ { (Ra * Rd * (Rb + Rc - Ro) + Rc * Ri * Ro + Rb * (Rc + Ri) * Ro) / ((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro), (Ra * (-(Rc * Rd) + (Rc + Rd) * Ro)) / ((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro), (Ra * Rb * (Rd - Ro)) / ((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro), -((Ra * Rb) / (Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri))) },
                                { (-(Rb * Rd * (Rc + Ag * Ri)) + Rb * (Rc + Rd) * Ro) / ((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro), -((Rc * Rd * (Ra + Ri) + (Ra * (Rb - Rd) - Rd * Ri + Rb * (Rc + Ri)) * Ro) / ((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro)), (Rb * (Ra + Ri) * (Rd - Ro)) / ((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro), -((Rb * (Ra + Ri)) / (Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri))) },
                                { -((Rc * (Ag * Rd * Ri + Rb * (-Rd + Ro))) / ((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro)), (Rc * Rd * (Ra + Ri + Ag * Ri) - Rc * (Ra + Ri) * Ro) / ((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro), (Rc * Rd * (Ra + Rb + Ri) + Rb * (Ra + Ri) * Ro) / ((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro), -((Rc * (Ra + Rb + Ri)) / (Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri))) },
                                { (Rd * (Ag * (Rb + Rc) * Ri - Rb * Ro)) / ((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro), -((Rd * (Ag * Rc * Ri + (Ra + Ri) * Ro)) / ((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro)), -((Rd + Ro) / Ro), 0 } });

            return Rd;
        }
    };
    wdft::RtypeAdaptor<float, 3, ImpedanceCalc, decltype (Pa), decltype (Pb), decltype (Rd_C4)> R { Pa, Pb, Rd_C4 };

    // Port D
    wdft::ResistorCapacitorSeriesT<float> R6_C7 { 1.0e3f, 4.7e-6f };
    wdft::WDFSeriesT<float, decltype (R), decltype (R6_C7)> Sd { R, R6_C7 };

    wdft::DiodePairT<float, decltype (Sd), wdft::DiodeQuality::Best> diodes { Sd, 5.0e-9f, 25.85e-3f, 2.0f };
};

int main()
{
    std::cout << "RAT Drive test\n";

    static constexpr float fs = 48000.0f;

    Reference_WDF ref {};
    ref.prepare (fs);

    Impedances impedances {};
    Params params {
#if NETLIST
        .Vplus_v_value = 4.5f,
        .R26_params = {
            .Ro = 1.0e-1f,
            .Ag = 100.0f,
            .Ri = 10.0e6f
        },
#endif
        .Rd_value = 0.5f * Reference_WDF::Rdistortion,
        .XP_params = {
            .Is = 5.0e-9f,
            .Vt = 25.85e-3f,
            .nabla = 2.0f,
        },
    };
    calc_impedances (impedances, fs, params);
    State state {};

    static constexpr int N = 100;
    std::array<float, N> ref_output {};
    float max_error = 0.0f;
    for (int i = 0; i < N; ++i)
    {
        const auto test_output = process (state, impedances, 0.1f);
        ref_output[i] = ref.process (0.1f);
        const auto error = std::abs (test_output - ref_output[i]);
        max_error = std::max (error, max_error);
    }
    std::cout << "Max Error: " << max_error << '\n';

    if (max_error > 1.0e-4f)
    {
        std::cout << "Error is too large... failing test!\n";
        return 1;
    }

#if !NETLIST
    std::ofstream ofp { "data.bin", std::ios::out | std::ios::binary };
    ofp.write(reinterpret_cast<const char*>(ref_output.data()), N * sizeof (float));
    ofp.close();
#endif

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
    double ref_cycles, test_cycles;
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
        ref_cycles = aggregate.best.cycles();
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
        test_cycles = aggregate.best.cycles();
    }
    std::cout << "wdf_compiler is " << ref_time / test_time << "x faster (time)\n";
    std::cout << "wdf_compiler is " << ref_cycles / test_cycles << "x faster (cycles)\n";

    free (data_in);
    free (data_out);
#endif

    return 0;
}
