#include "rc_lowpass.h"

#include "../chowdsp_wdf.h"
#include <iostream>
#include <fstream>
#include <random>

#include "../performance_counters/event_counter.h"

// TOTAL (Apple M1): Reference: https://dougallj.github.io/applecpu/firestorm.html
//   - FMSUB    -> (4/0.25  cycles/instruction)
//   - FMUL     -> (4/0.25  cycles/instruction) [@TODO: double check this]
//   - FADD     -> (2/0.25  cycles/instruction)
//   - FNEG     -> (2/0.25  cycles/instruction)
//   - LDR      -> (~2.5/0.333 cycles/instruction)
//   - STR      -> (0.5/1   cycles/instruction)

// add = add or negate or sub
// Capacitor:
//   - 1 load (state)
//   - 1 store (state)
//   - 2 add
// Resistor:
//   - no-op
// Ideal Vs:
//   - 1 mul
//   - 1 add
// Series:
//   - 1 load (1-multiply coeff)
//   - 1 mul
//   - 4 add
// Output:
//   - 1 mul
//   - 1 add

// C1 reflected:
//   - 1 load (C1 state)
// R1 reflected: (no-op)
// S1 reflected:
//   - 1 add
// Vin incident: (no-op)?
// Vin reflected:
//   - 1 fmsub
// S1 incident:
//   - 1 load (1-multiply coeff)
//   - 1 fmsub
//   - 2 add
// C1 incident:
//   - 1 add
//   - 1 negate
// C1 state update:
//   - 1 store
// Voltage:
//   - 1 mult
//   - 1 add
//
// TOTAL (Apple M1): Reference: https://dougallj.github.io/applecpu/firestorm.html
//   - FMSUB    -> (4/0.25  cycles/instruction)
//   - FADD     -> (2/0.25  cycles/instruction)
//   - FNEG     -> (2/0.25  cycles/instruction)
//   - LDR      -> (~2.5/0.333 cycles/instruction)
//   - STR      -> (0.5/1   cycles/instruction)
// Compiler output (ARM):
// process(State&, Impedances const&, float):
//   cycle (~2.5):
//     movi    d1, #0000000000000000
//     ldr     s2, [x0]
//     fmov    s4, #2.00000000
//   cycle (2):
//     fadd    s3, s2, s1
//   cycle (4):
//     fmadd   s0, s0, s4, s3
//   cycle (~2.5):
//     ldr     s4, [x1]
//     fadd    s3, s0, s1
//   cycle (2):
//     fadd    s3, s2, s3
//   cycle (2):
//     fmsub   s1, s4, s3, s1
//   cycle (2):
//     fadd    s1, s0, s1
//   cycle (2):
//     fsub    s0, s2, s1
//     fmov    s2, #0.50000000
//   cycle (2):
//     fneg    s1, s1
//     fmul    s0, s0, s2
//   cycle (1):
//     str     s1, [x0]
//     ret
// TOTAL: 22 cycles/sample
// Measured:
//   - chowdsp_wdf:  ~22.06 cycles/sample
//   - wdf_compiler: ~22.04 cycles/sample

struct Reference_WDF
{
    chowdsp::wdft::ResistorT<float> R1 { 1000.0f };
    chowdsp::wdft::CapacitorT<float> C1 { 1.0e-6f };
    chowdsp::wdft::WDFSeriesT<float, decltype (R1), decltype (C1)> S1 { R1, C1 };
    chowdsp::wdft::IdealVoltageSourceT<float, decltype (S1)> Vin { S1 };

    void prepare (float fs)
    {
        C1.prepare (fs);
    }

    float process (float V)
    {
        Vin.setVoltage (V);
        Vin.incident (S1.reflected());
        S1.incident (Vin.reflected());
        return chowdsp::wdft::voltage<float> (C1);
    }
};

int main()
{
    std::cout << "RC Lowpass test\n";

    static constexpr float fs = 48000.0f;

    Reference_WDF ref {};
    ref.prepare (fs);

    Impedances impedances {};
    calc_impedances (impedances, fs);
    State state {};

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
    static constexpr int M = 100'000'000;
    static constexpr int n_iter = 4;

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
