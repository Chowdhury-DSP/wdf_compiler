#include "baxandall_eq.h"

#if RUN_BENCH
#include <xsimd/xsimd.hpp>
#include "../performance_counters/event_counter.h"
#endif

#include "../chowdsp_wdf.h"
#include <iostream>
#include <fstream>
#include <random>

const auto parallel = [] (float R1, float R2)
{
    return R1 * R2 / (R1 + R2);
};

/**
 * Implentation based on Werner et. al:
 * https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=8371321
 */
struct Reference_WDF
{
    void prepare (float fs)
    {
        Ca.prepare (fs);
        P2.prepare (fs);
        P3.prepare (fs);
        S4.prepare (fs);
        S5.prepare (fs);
    }

    void setParams (float bassParam, float trebleParam)
    {
        {
            using DeferImpedance = chowdsp::wdft::ScopedDeferImpedancePropagation<decltype (P1), decltype (S2), decltype (S3), decltype (S4)>;
            DeferImpedance deferImpedance { P1, S2, S3, S4 };

            P2.setResistanceValue (Pb * bassParam);
            P3.setResistanceValue (Pb * (1.0f - bassParam));

            S4.setResistanceValue (parallel (Pt * trebleParam, 10.0e3f));
            S5.setResistanceValue (parallel (Pt * (1.0f - trebleParam), 1.0e3f));
        }

        // propagate impedance change through R-type adaptor
        R.propagateImpedanceChange();
    }

    inline float process (float x)
    {
        Vin.setVoltage (x);

        Vin.incident (S1.reflected());
        S1.incident (Vin.reflected());

        return chowdsp::wdft::voltage<float> (Rl);
    }

    static constexpr auto Pt = 100.0e3f;
    static constexpr auto Pb = 100.0e3f;

    // Port A
    chowdsp::wdft::ResistorCapacitorSeriesT<float> S4 { 1.0f, 6.4e-9f };

    // Port B
    chowdsp::wdft::ResistorCapacitorSeriesT<float> S5 { 1.0f, 64.0e-9f };
    chowdsp::wdft::ResistorT<float> Rl { 1.0e6f };
    chowdsp::wdft::WDFParallelT<float, decltype (Rl), decltype (S5)> P1 { Rl, S5 };

    // Port C
    chowdsp::wdft::ResistorT<float> Resc { 10.0e3f };

    // Port D
    chowdsp::wdft::ResistorCapacitorParallelT<float> P3 { 1.0f, 220.0e-9f };
    chowdsp::wdft::ResistorT<float> Resb { 1.0e3f };
    chowdsp::wdft::WDFSeriesT<float, decltype (Resb), decltype (P3)> S3 { Resb, P3 };

    // Port E
    chowdsp::wdft::ResistorCapacitorParallelT<float> P2 { 1.0f, 22.0e-9f };
    chowdsp::wdft::ResistorT<float> Resa { 10.0e3f };
    chowdsp::wdft::WDFSeriesT<float, decltype (Resa), decltype (P2)> S2 { Resa, P2 };

    struct ImpedanceCalc
    {
        template <typename RType>
        static float calcImpedance (RType& R)
        {
            const auto&& impedances = R.getPortImpedances();
            const auto Ra = impedances[0];
            const auto Rb = impedances[1];
            const auto Rc = impedances[2];
            const auto Rd = impedances[3];
            const auto Re = impedances[4];

            // This scattering matrix was derived using the R-Solver python script (https://github.com/jatinchowdhury18/R-Solver),
            // invoked with command: r_solver.py --datum 0 --adapt 5 --out scratch/baxandall_scatt.txt netlists/baxandall.txt
            R.setSMatrixData ({ { -((Ra * Ra * Rb + Ra * Ra * Rc - Rb * Rc * Rc) * Rd * Rd - (Rb * Rb * Rc + Rb * Rc * Rc + Rb * Rd * Rd + (Rb * Rb + 2 * Rb * Rc) * Rd) * Re * Re + (Ra * Ra * Rb * Rb + 2 * Ra * Ra * Rb * Rc + (Ra * Ra - Rb * Rb) * Rc * Rc) * Rd + (Ra * Ra * Rb * Rb + 2 * Ra * Ra * Rb * Rc + (Ra * Ra - Rb * Rb) * Rc * Rc + (Ra * Ra - 2 * Rb * Rc) * Rd * Rd + 2 * (Ra * Ra * Rb - Rb * Rc * Rc + (Ra * Ra - Rb * Rb) * Rc) * Rd) * Re) / ((Ra * Ra * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb) * Rc) * Rd * Rd + (Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra + Rb) * Rd * Rd + (2 * Ra * Rb + Rb * Rb) * Rc + (2 * Ra * Rb + Rb * Rb + 2 * (Ra + Rb) * Rc) * Rd) * Re * Re + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc) * Rd + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb + 2 * (Ra + Rb) * Rc) * Rd * Rd + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 3 * Ra * Rb + Rb * Rb) * Rc) * Rd) * Re), -((Ra * Ra * Rc + Ra * Rc * Rc) * Rd * Rd + (Ra * Rb * Rc + Ra * Rc * Rc + Ra * Rd * Rd + (Ra * Rb + 2 * Ra * Rc) * Rd) * Re * Re + 2 * (Ra * Ra * Rb * Rc + (Ra * Ra + Ra * Rb) * Rc * Rc) * Rd + (2 * Ra * Ra * Rb * Rc + 2 * (Ra * Ra + Ra * Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rc) * Rd * Rd + (Ra * Ra * Rb + 2 * Ra * Rc * Rc + 3 * (Ra * Ra + Ra * Rb) * Rc) * Rd) * Re) / ((Ra * Ra * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb) * Rc) * Rd * Rd + (Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra + Rb) * Rd * Rd + (2 * Ra * Rb + Rb * Rb) * Rc + (2 * Ra * Rb + Rb * Rb + 2 * (Ra + Rb) * Rc) * Rd) * Re * Re + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc) * Rd + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb + 2 * (Ra + Rb) * Rc) * Rd * Rd + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 3 * Ra * Rb + Rb * Rb) * Rc) * Rd) * Re), ((2 * Ra * Ra * Rb + (Ra * Ra + 2 * Ra * Rb) * Rc) * Rd * Rd + (Ra * Rb * Rb + Ra * Rb * Rc + Ra * Rb * Rd) * Re * Re + 2 * (Ra * Ra * Rb * Rb + (Ra * Ra * Rb + Ra * Rb * Rb) * Rc) * Rd + (2 * Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb) * Rd * Rd + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc + (3 * Ra * Ra * Rb + 2 * Ra * Rb * Rb + (Ra * Ra + 3 * Ra * Rb) * Rc) * Rd) * Re) / ((Ra * Ra * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb) * Rc) * Rd * Rd + (Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra + Rb) * Rd * Rd + (2 * Ra * Rb + Rb * Rb) * Rc + (2 * Ra * Rb + Rb * Rb + 2 * (Ra + Rb) * Rc) * Rd) * Re * Re + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc) * Rd + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb + 2 * (Ra + Rb) * Rc) * Rd * Rd + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 3 * Ra * Rb + Rb * Rb) * Rc) * Rd) * Re), -((Ra * Rb * Rb + Ra * Rb * Rc + Ra * Rb * Rd) * Re * Re - (Ra * Ra * Rb * Rc + (Ra * Ra + Ra * Rb) * Rc * Rc) * Rd + (Ra * Ra * Rb * Rb + Ra * Rb * Rb * Rc - (Ra * Ra + Ra * Rb) * Rc * Rc + (Ra * Ra * Rb - Ra * Ra * Rc) * Rd) * Re) / ((Ra * Ra * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb) * Rc) * Rd * Rd + (Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra + Rb) * Rd * Rd + (2 * Ra * Rb + Rb * Rb) * Rc + (2 * Ra * Rb + Rb * Rb + 2 * (Ra + Rb) * Rc) * Rd) * Re * Re + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc) * Rd + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb + 2 * (Ra + Rb) * Rc) * Rd * Rd + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 3 * Ra * Rb + Rb * Rb) * Rc) * Rd) * Re), ((2 * Ra * Ra * Rb + (Ra * Ra + 2 * Ra * Rb) * Rc) * Rd * Rd + (2 * Ra * Ra * Rb * Rb + (Ra * Ra + Ra * Rb) * Rc * Rc + (3 * Ra * Ra * Rb + 2 * Ra * Rb * Rb) * Rc) * Rd + (Ra * Ra * Rb * Rb + (Ra * Ra + Ra * Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb) * Rd * Rd + (2 * Ra * Ra * Rb + Ra * Rb * Rb) * Rc + (2 * Ra * Ra * Rb + 2 * Ra * Rb * Rb + (2 * Ra * Ra + 3 * Ra * Rb) * Rc) * Rd) * Re) / ((Ra * Ra * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb) * Rc) * Rd * Rd + (Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra + Rb) * Rd * Rd + (2 * Ra * Rb + Rb * Rb) * Rc + (2 * Ra * Rb + Rb * Rb + 2 * (Ra + Rb) * Rc) * Rd) * Re * Re + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc) * Rd + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb + 2 * (Ra + Rb) * Rc) * Rd * Rd + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 3 * Ra * Rb + Rb * Rb) * Rc) * Rd) * Re), -(Ra * Rc * Rd + (Ra * Rb + Ra * Rc + Ra * Rd) * Re) / ((Ra * Rb + (Ra + Rb) * Rc) * Rd + (Ra * Rb + (Ra + Rb) * Rc + (Ra + Rb) * Rd) * Re) },
                                { -((Ra * Rb * Rc + Rb * Rc * Rc) * Rd * Rd + (Rb * Rb * Rc + Rb * Rc * Rc + Rb * Rd * Rd + (Rb * Rb + 2 * Rb * Rc) * Rd) * Re * Re + 2 * (Ra * Rb * Rb * Rc + (Ra * Rb + Rb * Rb) * Rc * Rc) * Rd + (2 * Ra * Rb * Rb * Rc + 2 * (Ra * Rb + Rb * Rb) * Rc * Rc + (Ra * Rb + 2 * Rb * Rc) * Rd * Rd + (Ra * Rb * Rb + 2 * Rb * Rc * Rc + 3 * (Ra * Rb + Rb * Rb) * Rc) * Rd) * Re) / ((Ra * Ra * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb) * Rc) * Rd * Rd + (Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra + Rb) * Rd * Rd + (2 * Ra * Rb + Rb * Rb) * Rc + (2 * Ra * Rb + Rb * Rb + 2 * (Ra + Rb) * Rc) * Rd) * Re * Re + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc) * Rd + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb + 2 * (Ra + Rb) * Rc) * Rd * Rd + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 3 * Ra * Rb + Rb * Rb) * Rc) * Rd) * Re), ((Ra * Ra * Rc + Ra * Rc * Rc) * Rd * Rd - (Ra * Rb * Rb + Rb * Rb * Rc - Ra * Rc * Rc - Ra * Rd * Rd + (Rb * Rb - 2 * Ra * Rc) * Rd) * Re * Re - (Ra * Ra * Rb * Rb + 2 * Ra * Rb * Rb * Rc - (Ra * Ra - Rb * Rb) * Rc * Rc) * Rd - (Ra * Ra * Rb * Rb + 2 * Ra * Rb * Rb * Rc - (Ra * Ra - Rb * Rb) * Rc * Rc - (Ra * Ra + 2 * Ra * Rc) * Rd * Rd + 2 * (Ra * Rb * Rb - Ra * Rc * Rc - (Ra * Ra - Rb * Rb) * Rc) * Rd) * Re) / ((Ra * Ra * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb) * Rc) * Rd * Rd + (Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra + Rb) * Rd * Rd + (2 * Ra * Rb + Rb * Rb) * Rc + (2 * Ra * Rb + Rb * Rb + 2 * (Ra + Rb) * Rc) * Rd) * Re * Re + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc) * Rd + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb + 2 * (Ra + Rb) * Rc) * Rd * Rd + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 3 * Ra * Rb + Rb * Rb) * Rc) * Rd) * Re), -((Ra * Ra * Rb + Ra * Rb * Rc) * Rd * Rd + (2 * Ra * Rb * Rb + (2 * Ra * Rb + Rb * Rb) * Rc + (2 * Ra * Rb + Rb * Rb) * Rd) * Re * Re + 2 * (Ra * Ra * Rb * Rb + (Ra * Ra * Rb + Ra * Rb * Rb) * Rc) * Rd + (2 * Ra * Ra * Rb * Rb + Ra * Rb * Rd * Rd + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc + (2 * Ra * Ra * Rb + 3 * Ra * Rb * Rb + (3 * Ra * Rb + Rb * Rb) * Rc) * Rd) * Re) / ((Ra * Ra * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb) * Rc) * Rd * Rd + (Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra + Rb) * Rd * Rd + (2 * Ra * Rb + Rb * Rb) * Rc + (2 * Ra * Rb + Rb * Rb + 2 * (Ra + Rb) * Rc) * Rd) * Re * Re + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc) * Rd + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb + 2 * (Ra + Rb) * Rc) * Rd * Rd + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 3 * Ra * Rb + Rb * Rb) * Rc) * Rd) * Re), ((2 * Ra * Rb * Rb + (2 * Ra * Rb + Rb * Rb) * Rc + (2 * Ra * Rb + Rb * Rb) * Rd) * Re * Re + (Ra * Ra * Rb * Rb + (Ra * Rb + Rb * Rb) * Rc * Rc + (Ra * Ra * Rb + 2 * Ra * Rb * Rb) * Rc) * Rd + (2 * Ra * Ra * Rb * Rb + (Ra * Rb + Rb * Rb) * Rc * Rc + (2 * Ra * Ra * Rb + 3 * Ra * Rb * Rb) * Rc + (2 * Ra * Ra * Rb + 2 * Ra * Rb * Rb + (3 * Ra * Rb + 2 * Rb * Rb) * Rc) * Rd) * Re) / ((Ra * Ra * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb) * Rc) * Rd * Rd + (Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra + Rb) * Rd * Rd + (2 * Ra * Rb + Rb * Rb) * Rc + (2 * Ra * Rb + Rb * Rb + 2 * (Ra + Rb) * Rc) * Rd) * Re * Re + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc) * Rd + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb + 2 * (Ra + Rb) * Rc) * Rd * Rd + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 3 * Ra * Rb + Rb * Rb) * Rc) * Rd) * Re), -((Ra * Ra * Rb + Ra * Rb * Rc) * Rd * Rd + (Ra * Ra * Rb * Rb + Ra * Ra * Rb * Rc - (Ra * Rb + Rb * Rb) * Rc * Rc) * Rd - (Ra * Rb * Rb * Rc - Ra * Rb * Rd * Rd + (Ra * Rb + Rb * Rb) * Rc * Rc - (Ra * Rb * Rb - Rb * Rb * Rc) * Rd) * Re) / ((Ra * Ra * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb) * Rc) * Rd * Rd + (Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra + Rb) * Rd * Rd + (2 * Ra * Rb + Rb * Rb) * Rc + (2 * Ra * Rb + Rb * Rb + 2 * (Ra + Rb) * Rc) * Rd) * Re * Re + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc) * Rd + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb + 2 * (Ra + Rb) * Rc) * Rd * Rd + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 3 * Ra * Rb + Rb * Rb) * Rc) * Rd) * Re), -((Ra * Rb + Rb * Rc) * Rd + (Rb * Rc + Rb * Rd) * Re) / ((Ra * Rb + (Ra + Rb) * Rc) * Rd + (Ra * Rb + (Ra + Rb) * Rc + (Ra + Rb) * Rd) * Re) },
                                { ((2 * Ra * Rb * Rc + (Ra + 2 * Rb) * Rc * Rc) * Rd * Rd + (Rb * Rb * Rc + Rb * Rc * Rc + Rb * Rc * Rd) * Re * Re + 2 * (Ra * Rb * Rb * Rc + (Ra * Rb + Rb * Rb) * Rc * Rc) * Rd + (2 * Ra * Rb * Rb * Rc + (Ra + 2 * Rb) * Rc * Rd * Rd + 2 * (Ra * Rb + Rb * Rb) * Rc * Rc + ((Ra + 3 * Rb) * Rc * Rc + (3 * Ra * Rb + 2 * Rb * Rb) * Rc) * Rd) * Re) / ((Ra * Ra * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb) * Rc) * Rd * Rd + (Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra + Rb) * Rd * Rd + (2 * Ra * Rb + Rb * Rb) * Rc + (2 * Ra * Rb + Rb * Rb + 2 * (Ra + Rb) * Rc) * Rd) * Re * Re + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc) * Rd + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb + 2 * (Ra + Rb) * Rc) * Rd * Rd + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 3 * Ra * Rb + Rb * Rb) * Rc) * Rd) * Re), -((Ra * Ra * Rc + Ra * Rc * Rc) * Rd * Rd + (2 * Ra * Rb * Rc + (2 * Ra + Rb) * Rc * Rc + (2 * Ra + Rb) * Rc * Rd) * Re * Re + 2 * (Ra * Ra * Rb * Rc + (Ra * Ra + Ra * Rb) * Rc * Rc) * Rd + (2 * Ra * Ra * Rb * Rc + Ra * Rc * Rd * Rd + 2 * (Ra * Ra + Ra * Rb) * Rc * Rc + ((3 * Ra + Rb) * Rc * Rc + (2 * Ra * Ra + 3 * Ra * Rb) * Rc) * Rd) * Re) / ((Ra * Ra * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb) * Rc) * Rd * Rd + (Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra + Rb) * Rd * Rd + (2 * Ra * Rb + Rb * Rb) * Rc + (2 * Ra * Rb + Rb * Rb + 2 * (Ra + Rb) * Rc) * Rd) * Re * Re + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc) * Rd + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb + 2 * (Ra + Rb) * Rc) * Rd * Rd + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 3 * Ra * Rb + Rb * Rb) * Rc) * Rd) * Re), ((Ra * Ra * Rb - (Ra + Rb) * Rc * Rc) * Rd * Rd + (Ra * Rb * Rb - (Ra + Rb) * Rc * Rc + (Ra + Rb) * Rd * Rd + (2 * Ra * Rb + Rb * Rb) * Rd) * Re * Re + (Ra * Ra * Rb * Rb - (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc) * Rd + (Ra * Ra * Rb * Rb - (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb) * Rd * Rd + 2 * (Ra * Ra * Rb + Ra * Rb * Rb - (Ra + Rb) * Rc * Rc) * Rd) * Re) / ((Ra * Ra * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb) * Rc) * Rd * Rd + (Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra + Rb) * Rd * Rd + (2 * Ra * Rb + Rb * Rb) * Rc + (2 * Ra * Rb + Rb * Rb + 2 * (Ra + Rb) * Rc) * Rd) * Re * Re + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc) * Rd + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb + 2 * (Ra + Rb) * Rc) * Rd * Rd + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 3 * Ra * Rb + Rb * Rb) * Rc) * Rd) * Re), ((2 * (Ra + Rb) * Rc * Rc + 2 * (Ra + Rb) * Rc * Rd + (2 * Ra * Rb + Rb * Rb) * Rc) * Re * Re + (Ra * Ra * Rb * Rc + (Ra * Ra + Ra * Rb) * Rc * Rc) * Rd + ((2 * Ra * Ra + 3 * Ra * Rb + Rb * Rb) * Rc * Rc + (2 * Ra * Ra * Rb + Ra * Rb * Rb) * Rc + (2 * (Ra + Rb) * Rc * Rc + (2 * Ra * Ra + 3 * Ra * Rb) * Rc) * Rd) * Re) / ((Ra * Ra * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb) * Rc) * Rd * Rd + (Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra + Rb) * Rd * Rd + (2 * Ra * Rb + Rb * Rb) * Rc + (2 * Ra * Rb + Rb * Rb + 2 * (Ra + Rb) * Rc) * Rd) * Re * Re + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc) * Rd + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb + 2 * (Ra + Rb) * Rc) * Rd * Rd + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 3 * Ra * Rb + Rb * Rb) * Rc) * Rd) * Re), -((2 * (Ra + Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb) * Rc) * Rd * Rd + ((Ra * Ra + 3 * Ra * Rb + 2 * Rb * Rb) * Rc * Rc + (Ra * Ra * Rb + 2 * Ra * Rb * Rb) * Rc) * Rd + (Ra * Rb * Rb * Rc + 2 * (Ra + Rb) * Rc * Rd * Rd + (Ra * Rb + Rb * Rb) * Rc * Rc + (2 * (Ra + Rb) * Rc * Rc + (3 * Ra * Rb + 2 * Rb * Rb) * Rc) * Rd) * Re) / ((Ra * Ra * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb) * Rc) * Rd * Rd + (Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra + Rb) * Rd * Rd + (2 * Ra * Rb + Rb * Rb) * Rc + (2 * Ra * Rb + Rb * Rb + 2 * (Ra + Rb) * Rc) * Rd) * Re * Re + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc) * Rd + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb + 2 * (Ra + Rb) * Rc) * Rd * Rd + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 3 * Ra * Rb + Rb * Rb) * Rc) * Rd) * Re), -(Ra * Rc * Rd - Rb * Rc * Re) / ((Ra * Rb + (Ra + Rb) * Rc) * Rd + (Ra * Rb + (Ra + Rb) * Rc + (Ra + Rb) * Rd) * Re) },
                                { ((Ra * Rb * Rc + (Ra + Rb) * Rc * Rc) * Rd * Rd - (Rb * Rd * Rd + (Rb * Rb + Rb * Rc) * Rd) * Re * Re - ((Ra * Rb - Ra * Rc) * Rd * Rd + (Ra * Rb * Rb + Rb * Rb * Rc - (Ra + Rb) * Rc * Rc) * Rd) * Re) / ((Ra * Ra * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb) * Rc) * Rd * Rd + (Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra + Rb) * Rd * Rd + (2 * Ra * Rb + Rb * Rb) * Rc + (2 * Ra * Rb + Rb * Rb + 2 * (Ra + Rb) * Rc) * Rd) * Re * Re + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc) * Rd + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb + 2 * (Ra + Rb) * Rc) * Rd * Rd + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 3 * Ra * Rb + Rb * Rb) * Rc) * Rd) * Re), ((Ra * Ra * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb) * Rc) * Rd * Rd + ((2 * Ra + Rb) * Rd * Rd + (2 * Ra * Rb + (2 * Ra + Rb) * Rc) * Rd) * Re * Re + ((2 * Ra * Ra + 2 * Ra * Rb + (3 * Ra + 2 * Rb) * Rc) * Rd * Rd + (2 * Ra * Ra * Rb + (Ra + Rb) * Rc * Rc + (2 * Ra * Ra + 3 * Ra * Rb) * Rc) * Rd) * Re) / ((Ra * Ra * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb) * Rc) * Rd * Rd + (Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra + Rb) * Rd * Rd + (2 * Ra * Rb + Rb * Rb) * Rc + (2 * Ra * Rb + Rb * Rb + 2 * (Ra + Rb) * Rc) * Rd) * Re * Re + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc) * Rd + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb + 2 * (Ra + Rb) * Rc) * Rd * Rd + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 3 * Ra * Rb + Rb * Rb) * Rc) * Rd) * Re), ((Ra * Ra * Rb + (Ra * Ra + Ra * Rb) * Rc) * Rd * Rd + (2 * (Ra + Rb) * Rd * Rd + (2 * Ra * Rb + Rb * Rb + 2 * (Ra + Rb) * Rc) * Rd) * Re * Re + ((2 * Ra * Ra + 3 * Ra * Rb + 2 * (Ra + Rb) * Rc) * Rd * Rd + (2 * Ra * Ra * Rb + Ra * Rb * Rb + (2 * Ra * Ra + 3 * Ra * Rb + Rb * Rb) * Rc) * Rd) * Re) / ((Ra * Ra * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb) * Rc) * Rd * Rd + (Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra + Rb) * Rd * Rd + (2 * Ra * Rb + Rb * Rb) * Rc + (2 * Ra * Rb + Rb * Rb + 2 * (Ra + Rb) * Rc) * Rd) * Re * Re + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc) * Rd + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb + 2 * (Ra + Rb) * Rc) * Rd * Rd + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 3 * Ra * Rb + Rb * Rb) * Rc) * Rd) * Re), -((Ra * Ra * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb) * Rc) * Rd * Rd - (Ra * Rb * Rb + (Ra + Rb) * Rc * Rc - (Ra + Rb) * Rd * Rd + (2 * Ra * Rb + Rb * Rb) * Rc) * Re * Re - (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc - (Ra * Ra + 2 * Ra * Rb + 2 * (Ra + Rb) * Rc) * Rd * Rd + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc) * Re) / ((Ra * Ra * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb) * Rc) * Rd * Rd + (Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra + Rb) * Rd * Rd + (2 * Ra * Rb + Rb * Rb) * Rc + (2 * Ra * Rb + Rb * Rb + 2 * (Ra + Rb) * Rc) * Rd) * Re * Re + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc) * Rd + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb + 2 * (Ra + Rb) * Rc) * Rd * Rd + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 3 * Ra * Rb + Rb * Rb) * Rc) * Rd) * Re), -((Ra * Ra * Rb + 2 * (Ra + Rb) * Rc * Rc + (Ra * Ra + 3 * Ra * Rb) * Rc) * Rd * Rd + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc) * Rd + ((Ra * Rb + 2 * (Ra + Rb) * Rc) * Rd * Rd + (Ra * Rb * Rb + 2 * (Ra + Rb) * Rc * Rc + (3 * Ra * Rb + Rb * Rb) * Rc) * Rd) * Re) / ((Ra * Ra * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb) * Rc) * Rd * Rd + (Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra + Rb) * Rd * Rd + (2 * Ra * Rb + Rb * Rb) * Rc + (2 * Ra * Rb + Rb * Rb + 2 * (Ra + Rb) * Rc) * Rd) * Re * Re + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc) * Rd + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb + 2 * (Ra + Rb) * Rc) * Rd * Rd + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 3 * Ra * Rb + Rb * Rb) * Rc) * Rd) * Re), -(Rb * Rd * Re + (Ra * Rb + (Ra + Rb) * Rc) * Rd) / ((Ra * Rb + (Ra + Rb) * Rc) * Rd + (Ra * Rb + (Ra + Rb) * Rc + (Ra + Rb) * Rd) * Re) },
                                { ((Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra + 2 * Rb) * Rd * Rd + (2 * Ra * Rb + Rb * Rb) * Rc + (2 * Ra * Rb + 2 * Rb * Rb + (2 * Ra + 3 * Rb) * Rc) * Rd) * Re * Re + ((2 * Ra * Rb + (Ra + 2 * Rb) * Rc) * Rd * Rd + (2 * Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (3 * Ra * Rb + 2 * Rb * Rb) * Rc) * Rd) * Re) / ((Ra * Ra * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb) * Rc) * Rd * Rd + (Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra + Rb) * Rd * Rd + (2 * Ra * Rb + Rb * Rb) * Rc + (2 * Ra * Rb + Rb * Rb + 2 * (Ra + Rb) * Rc) * Rd) * Re * Re + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc) * Rd + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb + 2 * (Ra + Rb) * Rc) * Rd * Rd + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 3 * Ra * Rb + Rb * Rb) * Rc) * Rd) * Re), ((Ra * Rb * Rc + (Ra + Rb) * Rc * Rc - Ra * Rd * Rd - (Ra * Rb - Rb * Rc) * Rd) * Re * Re - ((Ra * Ra + Ra * Rc) * Rd * Rd + (Ra * Ra * Rb + Ra * Ra * Rc - (Ra + Rb) * Rc * Rc) * Rd) * Re) / ((Ra * Ra * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb) * Rc) * Rd * Rd + (Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra + Rb) * Rd * Rd + (2 * Ra * Rb + Rb * Rb) * Rc + (2 * Ra * Rb + Rb * Rb + 2 * (Ra + Rb) * Rc) * Rd) * Re * Re + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc) * Rd + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb + 2 * (Ra + Rb) * Rc) * Rd * Rd + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 3 * Ra * Rb + Rb * Rb) * Rc) * Rd) * Re), -((Ra * Rb * Rb + 2 * (Ra + Rb) * Rd * Rd + (Ra * Rb + Rb * Rb) * Rc + (3 * Ra * Rb + 2 * Rb * Rb + 2 * (Ra + Rb) * Rc) * Rd) * Re * Re + ((Ra * Ra + 2 * Ra * Rb + 2 * (Ra + Rb) * Rc) * Rd * Rd + (Ra * Ra * Rb + 2 * Ra * Rb * Rb + (Ra * Ra + 3 * Ra * Rb + 2 * Rb * Rb) * Rc) * Rd) * Re) / ((Ra * Ra * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb) * Rc) * Rd * Rd + (Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra + Rb) * Rd * Rd + (2 * Ra * Rb + Rb * Rb) * Rc + (2 * Ra * Rb + Rb * Rb + 2 * (Ra + Rb) * Rc) * Rd) * Re * Re + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc) * Rd + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb + 2 * (Ra + Rb) * Rc) * Rd * Rd + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 3 * Ra * Rb + Rb * Rb) * Rc) * Rd) * Re), -((Ra * Rb * Rb + 2 * (Ra + Rb) * Rc * Rc + (3 * Ra * Rb + Rb * Rb) * Rc + (Ra * Rb + 2 * (Ra + Rb) * Rc) * Rd) * Re * Re + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc + (Ra * Ra * Rb + 2 * (Ra + Rb) * Rc * Rc + (Ra * Ra + 3 * Ra * Rb) * Rc) * Rd) * Re) / ((Ra * Ra * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb) * Rc) * Rd * Rd + (Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra + Rb) * Rd * Rd + (2 * Ra * Rb + Rb * Rb) * Rc + (2 * Ra * Rb + Rb * Rb + 2 * (Ra + Rb) * Rc) * Rd) * Re * Re + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc) * Rd + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb + 2 * (Ra + Rb) * Rc) * Rd * Rd + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 3 * Ra * Rb + Rb * Rb) * Rc) * Rd) * Re), ((Ra * Ra * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb) * Rc) * Rd * Rd - (Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra + Rb) * Rd * Rd + (2 * Ra * Rb + Rb * Rb) * Rc + (2 * Ra * Rb + Rb * Rb + 2 * (Ra + Rb) * Rc) * Rd) * Re * Re + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc) * Rd) / ((Ra * Ra * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb) * Rc) * Rd * Rd + (Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra + Rb) * Rd * Rd + (2 * Ra * Rb + Rb * Rb) * Rc + (2 * Ra * Rb + Rb * Rb + 2 * (Ra + Rb) * Rc) * Rd) * Re * Re + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc) * Rd + (Ra * Ra * Rb * Rb + (Ra * Ra + 2 * Ra * Rb + Rb * Rb) * Rc * Rc + (Ra * Ra + 2 * Ra * Rb + 2 * (Ra + Rb) * Rc) * Rd * Rd + 2 * (Ra * Ra * Rb + Ra * Rb * Rb) * Rc + 2 * (Ra * Ra * Rb + Ra * Rb * Rb + (Ra + Rb) * Rc * Rc + (Ra * Ra + 3 * Ra * Rb + Rb * Rb) * Rc) * Rd) * Re), -(Ra * Rb + (Ra + Rb) * Rc + Ra * Rd) * Re / ((Ra * Rb + (Ra + Rb) * Rc) * Rd + (Ra * Rb + (Ra + Rb) * Rc + (Ra + Rb) * Rd) * Re) },
                                { -(Rc * Rd + (Rb + Rc + Rd) * Re) / (Ra * Rb + (Ra + Rb) * Rc + (Ra + Rc) * Rd + (Rb + Rc + Rd) * Re), -((Ra + Rc) * Rd + (Rc + Rd) * Re) / (Ra * Rb + (Ra + Rb) * Rc + (Ra + Rc) * Rd + (Rb + Rc + Rd) * Re), -(Ra * Rd - Rb * Re) / (Ra * Rb + (Ra + Rb) * Rc + (Ra + Rc) * Rd + (Rb + Rc + Rd) * Re), -(Ra * Rb + (Ra + Rb) * Rc + Rb * Re) / (Ra * Rb + (Ra + Rb) * Rc + (Ra + Rc) * Rd + (Rb + Rc + Rd) * Re), -(Ra * Rb + (Ra + Rb) * Rc + Ra * Rd) / (Ra * Rb + (Ra + Rb) * Rc + (Ra + Rc) * Rd + (Rb + Rc + Rd) * Re), 0 } });

            auto Rf = ((Ra * Rb + (Ra + Rb) * Rc) * Rd + (Ra * Rb + (Ra + Rb) * Rc + (Ra + Rb) * Rd) * Re) / (Ra * Rb + (Ra + Rb) * Rc + (Ra + Rc) * Rd + (Rb + Rc + Rd) * Re);
            return Rf;
        }
    };

    using RType = chowdsp::wdft::RtypeAdaptor<float, 5, ImpedanceCalc, decltype (S4), decltype (P1), decltype (Resc), decltype (S3), decltype (S2)>;
    RType R { S4, P1, Resc, S3, S2 };

    // Port F
    chowdsp::wdft::CapacitorT<float> Ca { 1.0e-6f };
    chowdsp::wdft::WDFSeriesT<float, decltype (R), decltype (Ca)> S1 { R, Ca };
    chowdsp::wdft::IdealVoltageSourceT<float, decltype (S1)> Vin { S1 };
};

int main()
{
    std::cout << "Baxandall EQ test\n";

    static constexpr float fs = 48000.0f;

    Reference_WDF ref {};
    const auto bass_param = 0.5f;
    const auto treble_param = 0.5f;
    ref.setParams (bass_param, treble_param);
    ref.prepare (fs);

    Impedances impedances {};
    Params params {
        .S4_res_value = parallel (Reference_WDF::Pt * treble_param, 10.0e3f),
        .S5_res_value = parallel (Reference_WDF::Pt * (1.0f - treble_param), 1.0e3f),
        .P3_res_value = Reference_WDF::Pb * (1.0f - bass_param),
        .P2_res_value = Reference_WDF::Pb * bass_param,
    };
    // printf ("%f\n", params.S4_res_value);
    // printf ("%f\n", params.S5_res_value);
    // printf ("%f\n", params.P3_res_value);
    // printf ("%f\n", params.P2_res_value);
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
