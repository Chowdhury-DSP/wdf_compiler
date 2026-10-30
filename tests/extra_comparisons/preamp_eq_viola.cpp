#include "viola_bench_config.h"
#include "viola_preamp_eq.h"
#include "viola_preamp_eq_terminate.h"

#include <iostream>
#include <fstream>
#include <random>

#include "../performance_counters/event_counter.h"

static void process_viola (const double* in, double* out, int num_samples)
{
    for (int start = 0; start < num_samples; start += VIOLA_BLOCK_SIZE)
        viola_preamp_eq (in + start, VIOLA_BLOCK_SIZE, false, out + start);
}

int main()
{
    std::cout << "Pre-amp EQ test (VIOLA)\n";

    static_assert (VIOLA_SAMPLE_RATE == 48000);
    double zeros[VIOLA_BLOCK_SIZE] {};
    double unused[VIOLA_BLOCK_SIZE] {};
    viola_preamp_eq (zeros, 0, true, unused);

    static constexpr int M = 100'000'000;
    static constexpr int n_iter = 4;
    static_assert (M % VIOLA_BLOCK_SIZE == 0);

    auto* data_in = (double*) malloc (M * sizeof (double));
    auto* data_out = (double*) malloc (M * sizeof (double));

    std::random_device rd {};
    std::default_random_engine gen { rd() };
    std::uniform_real_distribution<double> dist { -1.0, 1.0 };
    event_collector collector {};

    {
        event_aggregate aggregate {};
        float save_out = 0.0f;
        for (int iter = 0; iter < n_iter; ++iter)
        {
            for (int n = 0; n < M; ++n)
                data_in[n] = dist (gen);

            collector.start();
            process_viola (data_in, data_out, M);

            aggregate << collector.end();
            save_out += (float) data_out[M-1];
        }
        std::cout << save_out << '\n';
        pretty_print (aggregate, M, "VIOLA");
    }

    viola_preamp_eq_terminate();

    free (data_in);
    free (data_out);

    return 0;
}
