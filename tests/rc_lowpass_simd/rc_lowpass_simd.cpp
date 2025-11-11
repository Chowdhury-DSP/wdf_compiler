#include <xsimd/xsimd.hpp>
using WDF_Float = xsimd::batch<float>;

#include "rc_lowpass_simd.h"

#include "../chowdsp_wdf.h"
#include <iostream>
#include <fstream>
#include <random>

#include "../performance_counters/event_counter.h"

// borrowed from PFFFT
static void * pffft_aligned_malloc(size_t nb_bytes, size_t alignment) {
  void *p, *p0 = malloc(nb_bytes + alignment);
  if (!p0) return (void *) 0;
  p = (void *) (((size_t) p0 + alignment) & (~((size_t) (alignment-1))));
  *((void **) p - 1) = p0;
  return p;
}

static void pffft_aligned_free(void *p) {
  if (p) free(*((void **) p - 1));
}

struct Reference_WDF
{
    chowdsp::wdft::ResistorT<WDF_Float> R1 { 1000.0f };
    chowdsp::wdft::CapacitorT<WDF_Float> C1 { 1.0e-6f };
    chowdsp::wdft::WDFSeriesT<WDF_Float, decltype (R1), decltype (C1)> S1 { R1, C1 };
    chowdsp::wdft::IdealVoltageSourceT<WDF_Float, decltype (S1)> Vin { S1 };

    void prepare (float fs)
    {
        C1.prepare (fs);
    }

    WDF_Float process (WDF_Float V)
    {
        Vin.setVoltage (V);
        Vin.incident (S1.reflected());
        S1.incident (Vin.reflected());
        return chowdsp::wdft::voltage<WDF_Float> (C1);
    }
};

int main()
{
    std::cout << "RC Lowpass test (SIMD)\n";

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
        const auto test_output = process (state, impedances, 1.0f).get (0);
        ref_output[i] = ref.process (1.0f).get (0);
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

    auto* data_in = (WDF_Float*) pffft_aligned_malloc (M * sizeof (WDF_Float), 16);
    auto* data_out = (WDF_Float*) pffft_aligned_malloc (M * sizeof (WDF_Float), 16);

    std::random_device rd {};
    std::default_random_engine gen { rd() };
    std::uniform_real_distribution<float> dist { -1.0f, 1.0f };
    event_collector collector {};

    double ref_time, test_time;
    {
        event_aggregate aggregate {};
        WDF_Float save_out = 0.0f;
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
        WDF_Float save_out = 0.0f;
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

    pffft_aligned_free (data_in);
    pffft_aligned_free (data_out);
#endif

    return 0;
}
