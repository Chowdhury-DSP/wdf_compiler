#include "paper_diode_clipper.h"

#include <iostream>
#include <fstream>
#include <random>

#include "../performance_counters/event_counter.h"

int main()
{
    std::cout << "Diode Clipper test (Ciaramella)\n";

    static constexpr float fs = 48000.0;
    paper_diode_clipper ref {};
    ref.setSampleRate (fs);
    ref.reset();

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
            ref.process (data_in, data_out, M);

            aggregate << collector.end();
            save_out += data_out[M-1];
        }
        std::cout << save_out << '\n';
        pretty_print (aggregate, M, "Ciaramella");
    }

    free (data_in);
    free (data_out);

    return 0;
}
