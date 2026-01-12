#define FAUSTFLOAT double

#include "faust/dsp/dsp.h"
#include "faust/gui/MapUI.h"
#include "faust/gui/meta.h"

#include "rc_lowpass_faust.h"

#include <iostream>
#include <fstream>
#include <random>

#include "../performance_counters/event_counter.h"

int main()
{
    std::cout << "RC Lowpass test (Faust, double)\n";

    static constexpr float fs = 48000.0;
    mydsp ref {};
    ref.init (fs);

    static constexpr int M = 100'000'000;
    static constexpr int n_iter = 4;

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
            ref.compute(M, &data_in, &data_out);

            aggregate << collector.end();
            save_out += data_out[M-1];
        }
        std::cout << save_out << '\n';
        pretty_print (aggregate, M, "Faust");
    }

    free (data_in);
    free (data_out);

    return 0;
}
