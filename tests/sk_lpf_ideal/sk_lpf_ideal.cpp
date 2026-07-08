#include "sk_lpf_ideal.h"

#if RUN_BENCH
#include <xsimd/xsimd.hpp>
#include "../performance_counters/event_counter.h"
#endif

#include "../chowdsp_wdf.h"
#include <iostream>
#include <fstream>
#include <random>
#include <tuple>

struct Reference_WDF
{
    void prepare (float fs)
    {
        C1.prepare (fs);
        C2.prepare (fs);
    }

    inline float process (float x)
    {
        Vin.setVoltage (x);

        Vin.incident (S1.reflected());
        S1.incident (Vin.reflected());

        return chowdsp::wdft::voltage<float> (Rload);
    }

    // Port A ("R2" in the scattering-matrix formula)
    chowdsp::wdft::ResistorT<float> R2 { 10.0e3f };
    // Port B ("R3")
    chowdsp::wdft::CapacitorT<float> C2 { 100.0e-9f };
    // Port C ("R4")
    chowdsp::wdft::CapacitorT<float> C1 { 100.0e-9f };
    // Port D ("R5")
    chowdsp::wdft::ResistorT<float> Rg { 6.0e3f };
    // Port E ("R6")
    chowdsp::wdft::ResistorT<float> Rf { 8.0e3f };
    // Port F ("R7")
    chowdsp::wdft::ResistorT<float> Rload { 100.0e3f };

    struct ImpedanceCalc
    {
        template <typename RType>
        static float calcImpedance (RType& R)
        {
            const auto&& impedances = R.getPortImpedances();
            const auto R2 = impedances[0];
            const auto R3 = impedances[1];
            const auto R4 = impedances[2];
            const auto R5 = impedances[3];
            const auto R6 = impedances[4];
            // const auto R7 = impedances[5];

            const auto R1 = (R3*(R2 + R4)*R5)/((R2 + R3)*R5 - R4*R6);

            R.setSMatrixData({
                {0,(R1*(-(R3*R5) + R4*(R5 + R6)))/(R3*(R2 + R4)*R5),-(R1/R3),-1 - (R1*R6)/(R3*R5),(R1*R6)/(R3*R5),R1/R3,0},
                {-(R2/(R2 + R4)),(R1*R3*R5 - R2*R3*R5 - R1*R4*R6)/(R2*R3*R5 + R3*R4*R5),(R1*R2)/(R2*R3 + R3*R4),(R2*(-(R3*R5) + R1*R6))/(R3*(R2 + R4)*R5),-((R1*R2*R6)/(R2*R3*R5 + R3*R4*R5)),-((R1*R2)/(R2*R3 + R3*R4)),0},
                {(-(R2*R5) + R4*R6)/((R2 + R4)*R5),(R1*R5 + R4*(R5 + R6))/((R2 + R4)*R5),-(R1/(R2 + R4)),-((R1*R6 + R2*(R5 + R6))/((R2 + R4)*R5)),((R1 + R2 + R4)*R6)/((R2 + R4)*R5),1 + R1/(R2 + R4),0},
                {-(R4/(R2 + R4)),-(((R1 + R3)*R4)/(R3*(R2 + R4))),(R1*R4)/(R2*R3 + R3*R4),(R2*R3*R5 + R1*R4*R6)/(R2*R3*R5 + R3*R4*R5),-((R1*R4*R6)/(R2*R3*R5 + R3*R4*R5)),-((R1*R4)/(R2*R3 + R3*R4)),0},
                {-(R4/(R2 + R4)),-(((R1 + R3)*R4)/(R3*(R2 + R4))),(R1*R4)/(R2*R3 + R3*R4),(R2*R3 + R1*(R2 + R3))/(R3*(R2 + R4)),-1 - (R1*R4*R6)/(R2*R3*R5 + R3*R4*R5),-((R1*R4)/(R2*R3 + R3*R4)),0},
                {(R4*R6)/(R2*R5 + R4*R5),((R1 + R3)*R4*R6)/(R3*(R2 + R4)*R5),-((R1*R4*R6)/(R2*R3*R5 + R3*R4*R5)),-(((R2*R3 + R1*(R2 + R3))*R6)/(R3*(R2 + R4)*R5)),((R1*(R2 + R3) + R3*(R2 + R4))*R6)/(R3*(R2 + R4)*R5),1 + (R1*R4*R6)/(R2*R3*R5 + R3*R4*R5),0},
                {-((R4*(R5 + R6))/((R2 + R4)*R5)),-(((R1 + R3)*R4*(R5 + R6))/(R3*(R2 + R4)*R5)),(R1*R4*(R5 + R6))/(R3*(R2 + R4)*R5),((R2*R3 + R1*(R2 + R3))*(R5 + R6))/(R3*(R2 + R4)*R5),-(((R3*(R2 + R4) + R1*(R2 + R3 + R4))*R6)/(R3*(R2 + R4)*R5)),-((2*R2*R3*R5 + R1*R4*R5 + 2*R3*R4*R5 + R1*R4*R6)/(R2*R3*R5 + R3*R4*R5)),-1},
            });

            return R1;
        }
    };

    using RType = chowdsp::wdft::RtypeAdaptor<float, 0, ImpedanceCalc, decltype (R2), decltype (C2), decltype (C1), decltype (Rg), decltype (Rf), decltype (Rload)>;
    RType R { R2, C2, C1, Rg, Rf, Rload };

    // Outer series resistor between Vin and the R-type block ("R1" in both
    // sk_lpf.wdf and sk_lpf.net).
    chowdsp::wdft::ResistorT<float> Rs { 10.0e3f };
    chowdsp::wdft::WDFSeriesT<float, decltype (Rs), decltype (R)> S1 { Rs, R };
    chowdsp::wdft::IdealVoltageSourceT<float, decltype (S1)> Vin { S1 };
};

int main()
{
    std::cout << "Sallen-Key LPF (Ideal op-amp) test\n";

    static constexpr float fs = 48000.0f;

    Reference_WDF ref {};
    ref.prepare (fs);

    Params params {};
    Impedances impedances {};
    calc_impedances (impedances, fs, params);
    State state {};

    static constexpr int N = 100;
    std::array<float, N> ref_output {};
    float max_error = 0.0f;
    for (int i = 0; i < N; ++i)
    {
        const auto test_output = process (state, impedances, 1.0f);
        ref_output[i] = ref.process (1.0f);
        const auto error = std::abs (test_output - ref_output[i]);
        max_error = std::max (error, max_error);
    }
    std::cout << "Max Error: " << max_error << '\n';

    if (max_error > 1.0e-4f)
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
