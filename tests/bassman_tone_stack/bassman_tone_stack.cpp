#include "bassman_tone_stack.h"

#include "../chowdsp_wdf.h"
#include <iostream>
#include <random>

#if CHOWDSP_WDF_TEST_WITH_XSIMD
#include <xsimd/xsimd.hpp>
#endif

namespace chowdsp
{
namespace wdft
{
    /**
     *  A non-adaptable R-Type adaptor.
     *  For more information see: https://searchworks.stanford.edu/view/11891203, chapter 2
     *
     *  The ImpedanceCalculator template argument with a static method of the form:
     *  @code
     *  template <typename RType>
     *  static void calcImpedance (RType& R);
     *  @endcode
     */
    template <typename T, typename ImpedanceCalculator, typename... PortTypes>
    class BassmanRootRtypeAdaptor : public RootWDF
    {
    public:
        /** Number of ports connected to RootRtypeAdaptor */
        static constexpr auto numPorts = int (sizeof...(PortTypes));

        explicit BassmanRootRtypeAdaptor (PortTypes&... dps) : downPorts (std::tie (dps...))
        {
            b_vec.clear();
            a_vec.clear();

            rtype_detail::forEachInTuple ([&] (auto& port, size_t) { port.connectToParent (this); },
                                          downPorts);
        }

        /** Recomputes internal variables based on the incoming impedances */
        void calcImpedance() override
        {
            ImpedanceCalculator::calcImpedance (*this);
        }

        constexpr auto getPortImpedances()
        {
            std::array<T, numPorts> portImpedances {};
            rtype_detail::forEachInTuple ([&] (auto& port, size_t i) { portImpedances[i] = port.wdf.R; },
                                          downPorts);

            return portImpedances;
        }

        /** Use this function to set the scattering matrix data. */
        void setSMatrixData (const T (&mat)[numPorts][numPorts])
        {
            for (int i = 0; i < numPorts; ++i)
                for (int j = 0; j < numPorts; ++j)
                    S_matrix[j][i] = mat[i][j];
        }

        /** Computes both the incident and reflected waves at this root node. */
        inline void compute() noexcept
        {
            // @TODO: T
            rtype_detail::forEachInTuple ([&] (auto& port, size_t i) {
                                          a_vec[i] = port.reflected(); },
                                          downPorts);
            rtype_detail::RtypeScatter (S_matrix, a_vec, b_vec);
            rtype_detail::forEachInTuple ([&] (auto& port, size_t i) {
                                          port.incident (b_vec[i]); },
                                          downPorts);
        }

    private:
        std::tuple<PortTypes&...> downPorts; // tuple of ports connected to RtypeAdaptor

        rtype_detail::Matrix<T, numPorts> S_matrix; // square matrix representing S
        rtype_detail::AlignedArray<T, numPorts> a_vec; // temp matrix of inputs to Rport
        rtype_detail::AlignedArray<T, numPorts> b_vec; // temp matrix of outputs from Rport
    };
} // namespace wdft
} // namespace chowdsp

struct Reference_WDF
{
    void prepare (float fs)
    {
        Cap1.prepare (fs);
        Cap2.prepare (fs);
        Cap3.prepare (fs);
    }

    float process (float V_in)
    {
        Vin_Res3m.setVoltage (V_in);
        R.compute();

        // @TODO...
        // return chowdsp::wdft::voltage<float> (Res1m) + chowdsp::wdft::voltage<float> (S2) + chowdsp::wdft::voltage<float> (Res3m);
        return chowdsp::wdft::voltage<float> (S2);
    }

    void setParams (float highPot, float lowPot, float midPot)
    {
        {
            using DeferImpedance = chowdsp::wdft::ScopedDeferImpedancePropagation<decltype (Vin_Res3m), decltype (S3), decltype (S4)>;
            DeferImpedance deferImpedance { Vin_Res3m, S3, S4 };

            Res1m.setResistanceValue (highPot * R1);
            Res1p.setResistanceValue ((1.0f - highPot) * R1);

            Res2.setResistanceValue ((1.0f - lowPot) * R2);

            Vin_Res3m.setResistanceValue (midPot * R3);
            Res3p.setResistanceValue ((1.0f - midPot) * R3);
        }

        S2.propagateImpedanceChange();
    }

    chowdsp::wdft::CapacitorT<float> Cap1 { 250e-12 };
    chowdsp::wdft::CapacitorT<float> Cap2 { 20e-9 }; // Port D
    chowdsp::wdft::CapacitorT<float> Cap3 { 20e-9 }; // Port F

    chowdsp::wdft::ResistorT<float> Res1p { 1.0 };
    chowdsp::wdft::ResistorT<float> Res1m { 1.0 };
    chowdsp::wdft::ResistorT<float> Res2 { 1.0 };
    chowdsp::wdft::ResistorT<float> Res3p { 1.0 };
    chowdsp::wdft::ResistorT<float> Res3m { 1.0 };
    chowdsp::wdft::ResistorT<float> Res4 { 56e3 }; // Port E


    // Port A
    chowdsp::wdft::ResistiveVoltageSourceT<float> Vin_Res3m { 1.0 };

    // Port B
    chowdsp::wdft::WDFSeriesT<float, decltype (Res2), decltype (Res3p)> S3 { Res2, Res3p };

    // Port C
    chowdsp::wdft::WDFSeriesT<float, decltype (Res1p), decltype (Res1m)> S4 { Res1p, Res1m };
    chowdsp::wdft::WDFSeriesT<float, decltype (Cap1), decltype (S4)> S2 { Cap1, S4 };

    static constexpr float R1 = 250e3f;
    static constexpr float R2 = 1e6f;
    static constexpr float R3 = 25e3f;

    struct ImpedanceCalc
    {
        template <typename RType>
        static void calcImpedance (RType& R)
        {
            const auto&& impedances = R.getPortImpedances();
            const auto Ra = impedances[0];
            const auto Rb = impedances[1];
            const auto Rc = impedances[2];
            const auto Rd = impedances[3];
            const auto Re = impedances[4];
            const auto Rf = impedances[5];
            const auto Ga = (float) 1 / Ra;
            const auto Gb = (float) 1 / Rb;
            const auto Gc = (float) 1 / Rc;
            const auto Gd = (float) 1 / Rd;
            const auto Ge = (float) 1 / Re;
            const auto Gf = (float) 1 / Rf;

            // This scattering matrix was derived using the R-Solver python script (https://github.com/jatinchowdhury18/R-Solver),
            // with netlist input: netlists/bassman.txt
            R.setSMatrixData ({ { 2 * Ra * (-Ga * Gb * Gc * Gd - Ga * Gb * Gc * Ge - Ga * Gb * Gc * Gf - Ga * Gb * Gd * Ge - Ga * Gb * Ge * Gf - Ga * Gc * Gd * Gf - Ga * Gc * Ge * Gf - Ga * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf) + 1, 2 * Ra * (Ga * Gb * Gc * Gd + Ga * Gb * Gc * Ge + Ga * Gb * Gc * Gf + Ga * Gb * Gd * Ge) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Ra * (Ga * Gb * Gc * Gd + Ga * Gb * Gc * Ge + Ga * Gb * Gc * Gf + Ga * Gc * Gd * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Ra * (Ga * Gb * Gd * Ge - Ga * Gc * Gd * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Ra * (-Ga * Gb * Gd * Ge - Ga * Gb * Ge * Gf - Ga * Gc * Ge * Gf - Ga * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Ra * (-Ga * Gb * Ge * Gf - Ga * Gc * Gd * Gf - Ga * Gc * Ge * Gf - Ga * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf) },
                                { 2 * Rb * (Ga * Gb * Gc * Gd + Ga * Gb * Gc * Ge + Ga * Gb * Gc * Gf + Ga * Gb * Gd * Ge) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Rb * (-Ga * Gb * Gc * Gd - Ga * Gb * Gc * Ge - Ga * Gb * Gc * Gf - Ga * Gb * Gd * Ge - Ga * Gb * Gd * Gf - Gb * Gc * Gd * Gf - Gb * Gc * Ge * Gf - Gb * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf) + 1, 2 * Rb * (-Ga * Gb * Gc * Gd - Ga * Gb * Gc * Ge - Ga * Gb * Gc * Gf - Gb * Gc * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Rb * (-Ga * Gb * Gd * Ge - Ga * Gb * Gd * Gf - Gb * Gc * Gd * Gf - Gb * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Rb * (Ga * Gb * Gd * Ge - Gb * Gc * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Rb * (-Ga * Gb * Gd * Gf - Gb * Gc * Gd * Gf - Gb * Gc * Ge * Gf - Gb * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf) },
                                { 2 * Rc * (Ga * Gb * Gc * Gd + Ga * Gb * Gc * Ge + Ga * Gb * Gc * Gf + Ga * Gc * Gd * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Rc * (-Ga * Gb * Gc * Gd - Ga * Gb * Gc * Ge - Ga * Gb * Gc * Gf - Gb * Gc * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Rc * (-Ga * Gb * Gc * Gd - Ga * Gb * Gc * Ge - Ga * Gb * Gc * Gf - Ga * Gc * Gd * Ge - Ga * Gc * Gd * Gf - Gb * Gc * Gd * Ge - Gb * Gc * Ge * Gf - Gc * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf) + 1, 2 * Rc * (Ga * Gc * Gd * Ge + Ga * Gc * Gd * Gf + Gb * Gc * Gd * Ge + Gc * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Rc * (-Ga * Gc * Gd * Ge - Gb * Gc * Gd * Ge - Gb * Gc * Ge * Gf - Gc * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Rc * (Ga * Gc * Gd * Gf - Gb * Gc * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf) },
                                { 2 * Rd * (Ga * Gb * Gd * Ge - Ga * Gc * Gd * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Rd * (-Ga * Gb * Gd * Ge - Ga * Gb * Gd * Gf - Gb * Gc * Gd * Gf - Gb * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Rd * (Ga * Gc * Gd * Ge + Ga * Gc * Gd * Gf + Gb * Gc * Gd * Ge + Gc * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Rd * (-Ga * Gb * Gd * Ge - Ga * Gb * Gd * Gf - Ga * Gc * Gd * Ge - Ga * Gc * Gd * Gf - Gb * Gc * Gd * Ge - Gb * Gc * Gd * Gf - Gb * Gd * Ge * Gf - Gc * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf) + 1, 2 * Rd * (Ga * Gb * Gd * Ge + Ga * Gc * Gd * Ge + Gb * Gc * Gd * Ge + Gc * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Rd * (-Ga * Gb * Gd * Gf - Ga * Gc * Gd * Gf - Gb * Gc * Gd * Gf - Gb * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf) },
                                { 2 * Re * (-Ga * Gb * Gd * Ge - Ga * Gb * Ge * Gf - Ga * Gc * Ge * Gf - Ga * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Re * (Ga * Gb * Gd * Ge - Gb * Gc * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Re * (-Ga * Gc * Gd * Ge - Gb * Gc * Gd * Ge - Gb * Gc * Ge * Gf - Gc * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Re * (Ga * Gb * Gd * Ge + Ga * Gc * Gd * Ge + Gb * Gc * Gd * Ge + Gc * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Re * (-Ga * Gb * Gd * Ge - Ga * Gb * Ge * Gf - Ga * Gc * Gd * Ge - Ga * Gc * Ge * Gf - Ga * Gd * Ge * Gf - Gb * Gc * Gd * Ge - Gb * Gc * Ge * Gf - Gc * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf) + 1, 2 * Re * (-Ga * Gb * Ge * Gf - Ga * Gc * Ge * Gf - Ga * Gd * Ge * Gf - Gb * Gc * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf) },
                                { 2 * Rf * (-Ga * Gb * Ge * Gf - Ga * Gc * Gd * Gf - Ga * Gc * Ge * Gf - Ga * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Rf * (-Ga * Gb * Gd * Gf - Gb * Gc * Gd * Gf - Gb * Gc * Ge * Gf - Gb * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Rf * (Ga * Gc * Gd * Gf - Gb * Gc * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Rf * (-Ga * Gb * Gd * Gf - Ga * Gc * Gd * Gf - Gb * Gc * Gd * Gf - Gb * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Rf * (-Ga * Gb * Ge * Gf - Ga * Gc * Ge * Gf - Ga * Gd * Ge * Gf - Gb * Gc * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf), 2 * Rf * (-Ga * Gb * Gd * Gf - Ga * Gb * Ge * Gf - Ga * Gc * Gd * Gf - Ga * Gc * Ge * Gf - Ga * Gd * Ge * Gf - Gb * Gc * Gd * Gf - Gb * Gc * Ge * Gf - Gb * Gd * Ge * Gf) / (Ga * Gb * Gd + Ga * Gb * Ge + Ga * Gb * Gf + Ga * Gc * Gd + Ga * Gc * Ge + Ga * Gc * Gf + Ga * Gd * Ge + Ga * Gd * Gf + Gb * Gc * Gd + Gb * Gc * Ge + Gb * Gc * Gf + Gb * Gd * Ge + Gb * Ge * Gf + Gc * Gd * Gf + Gc * Ge * Gf + Gd * Ge * Gf) + 1 } });
        }
    };

    using RType = chowdsp::wdft::BassmanRootRtypeAdaptor<float, ImpedanceCalc, decltype (Vin_Res3m), decltype (S3), decltype (S2), decltype (Cap2), decltype (Res4), decltype (Cap3)>;
    RType R { Vin_Res3m, S3, S2, Cap2, Res4, Cap3 };
};

int main()
{
    std::cout << "Bassman Tone Stack test\n";

    static constexpr float fs = 48000.0f;

    Reference_WDF ref {};
    ref.setParams (0.5f, 0.5f, 0.5f);
    ref.prepare (fs);

    Impedances impedances {};
    Params params {
        .Vin_Res3m_res_value = ref.Vin_Res3m.wdf.R,
        .Res2_Res3p_value = ref.Res2.wdf.R + ref.Res3p.wdf.R,
        .Res1p_Res1m_Cap1_res_value = ref.Res1p.wdf.R + ref.Res1m.wdf.R,
    };
    calc_impedances (impedances, fs, params);
    State state {};

    float max_error = 0.0f;
    for (int i = 0; i < 100; ++i)
    {
        const auto test_output = process (state, impedances, 1.0f);
        const auto ref_output = ref.process (1.0f);
        const auto error = std::abs (test_output - ref_output);
        max_error = std::max (error, max_error);
    }
    std::cout << "Max Error: " << max_error << '\n';

    if (max_error > 1.0e-3f)
    {
        std::cout << "Error is too large... failing test!\n";
        return 1;
    }

#if RUN_BENCH
    static constexpr int N = 10'000'000;

    auto* data_in = (float*) malloc (N * sizeof (float));
    auto* data_out = (float*) malloc (N * sizeof (float));

    std::random_device rd {};
    std::default_random_engine gen { rd() };
    std::uniform_real_distribution<float> dist { -1.0f, 1.0f };
    for (int n = 0; n < N; ++n)
        data_in[n] = dist (gen);

    double ref_time, test_time;
    {
        auto start = std::chrono::steady_clock::now();

        for (int n = 0; n < N; ++n)
            data_out[n] = ref.process (data_in[n]);

        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::duration<double, std::milli>> (end - start);
        std::cout << data_out[N-1] << '\n';
        std::cout << "chowdsp_wdf: " << duration.count() << " milliseconds" << std::endl;
        ref_time = duration.count();
    }

    {
        auto start = std::chrono::steady_clock::now();

        for (int n = 0; n < N; ++n)
            data_out[n] = process (state, impedances, data_in[n]);

        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::duration<double, std::milli>> (end - start);
        std::cout << data_out[N-1] << '\n';
        std::cout << "wdf_compiler: " << duration.count() << " milliseconds" << std::endl;
        test_time = duration.count();
    }
    std::cout << "wdf_compiler is " << ref_time / test_time << "x faster\n";

    free (data_in);
    free (data_out);
#endif

    return 0;
}
