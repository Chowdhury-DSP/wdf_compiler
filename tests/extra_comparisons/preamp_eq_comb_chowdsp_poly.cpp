#include "../chowdsp_wdf.h"
#include <iostream>
#include <fstream>
#include <random>

#include "../performance_counters/event_counter.h"

struct Reference_WDF
{
    chowdsp::wdf::ResistorCapacitorParallel<float> lfl_res_cap { 15.0e3f, 0.15e-6f };
    chowdsp::wdf::Resistor<float> r_shunt { 4'674.0f };
    chowdsp::wdf::WDFSeries<float> s_s { &lfl_res_cap, &r_shunt };

    chowdsp::wdf::Capacitor<float> hfc_cap { 3.3e-6f };
    chowdsp::wdf::WDFParallel<float> p_s { &s_s, &hfc_cap };

    chowdsp::wdf::ResistorCapacitorSeries<float> hfl_res_cap { 80.0e3f, 2.7e-9f };
    chowdsp::wdf::Resistor<float> r_series { 10.0e3f };
    chowdsp::wdf::WDFParallel<float> p_p { &hfl_res_cap, &r_series };

    chowdsp::wdf::WDFSeries<float> s_o { &p_s, &p_p };

    chowdsp::wdf::Capacitor<float> lfc_cap { 4.7e-9f };
    chowdsp::wdf::WDFSeries<float> s_i { &s_o, &lfc_cap };

    chowdsp::wdf::IdealVoltageSource<float> v_in { &s_i };

    void prepare (float fs)
    {
        lfl_res_cap.prepare (fs);
        hfc_cap.prepare (fs);
        hfl_res_cap.prepare (fs);
        lfc_cap.prepare (fs);
        reset();
    }

    void reset() noexcept
    {
        lfl_res_cap.reset();
        hfc_cap.reset();
        hfl_res_cap.reset();
        lfc_cap.reset();
    }

    float process (float V)
    {
        v_in.setVoltage (V);
        v_in.incident (s_i.reflected());
        s_i.incident (v_in.reflected());
        return hfc_cap.voltage();
    }
};

int main()
{
    std::cout << "Pre-amp EQ test (chowdsp_wdf polymorphic)\n";

    static constexpr float fs = 48000.0f;

    Reference_WDF ref {};
    ref.prepare (fs);

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
            for (int n = 0; n < M; ++n)
                data_out[n] = ref.process (data_in[n]);

            aggregate << collector.end();
            save_out += data_out[M-1];
        }
        std::cout << save_out << '\n';
        pretty_print (aggregate, M, "chowdsp_wdf poly");
    }

    free (data_in);
    free (data_out);

    return 0;
}
