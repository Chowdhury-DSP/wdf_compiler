#include "sk_lpf.h"

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
            const auto R7 = impedances[5];

            static constexpr float Ri = 100.0e3f;
            static constexpr float Ro = 10.0f;
            static constexpr float Ag = 1.0e6f;

            // This scattering matrix matches custom_sk_lpf_rtype.h's update_vars.
            const auto R1 = (R3*R7*(R2*R4*R5 + R2*R4*R6 + R2*R5*R6 + R4*R5*R6 - (R2 + R4)*((-1 + Ag)*R5 - R6)*Ri) - (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro)/(R7*(R4*R5*R6 + R4*(R5 + R6 + Ag*R6)*Ri + R2*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri - Ag*Ri)) + R3*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri - Ag*Ri))) - (R4*R5*R6 + (R4 + R5)*R6*R7 + R4*(R5 + R6)*Ri + (R4 + R5 + R6)*R7*Ri + R2*(R5*(R6 + R7) + R4*(R5 + R6 + R7) + (R5 + R6 + R7)*Ri) + R3*(R5*(R6 + R7) + R4*(R5 + R6 + R7) + (R5 + R6 + R7)*Ri))*Ro);

            R.setSMatrixData({ { 0,(R1*R7*(Ag*R4*(R5 + R6)*Ri + R3*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri - Ag*Ri))) - R1*((R4 + R5)*R6*R7 + (R5 + R6)*R7*Ri + R3*(R5*(R6 + R7) + R4*(R5 + R6 + R7) + (R5 + R6 + R7)*Ri))*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),(R1*R7*(R2*R4*R5 + R2*R4*R6 + R2*R5*R6 + R4*R5*R6 - (R2 + R4)*((-1 + Ag)*R5 - R6)*Ri) - R1*(R4*R5*R6 + R4*(R5 + R6 + R7)*Ri + R2*(R5*(R6 + R7) + R4*(R5 + R6 + R7) + (R5 + R6 + R7)*Ri))*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),(R1*R7*(R3*R6*Ri - Ag*R2*(R5 + R6)*Ri + R3*R5*(R6 + Ri - Ag*Ri)) - R1*(R3*R5*R6 + R5*(R2 + R3 + R6)*R7 + (R5 + R6)*R7*Ri + R3*(R5 + R6 + R7)*Ri)*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),(R1*R6*R7*(R3*R4 + Ag*(R2 + R4)*Ri) - R1*(R3*R4*R6 + R4*(R2 + R3 + R6)*R7 + (R2 + R4)*R7*Ri)*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),(R1*(Ag*(R2 + R4)*R5*R7*Ri + R3*R4*R5*(R7 - Ro) + R7*(R4*Ri + R2*(R4 + R5 + Ri))*Ro))/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),-((R1*(R4*R5*(R3 + R6) + R4*(R5 + R6)*Ri + R2*(R5*R6 + R4*(R5 + R6) + (R5 + R6)*Ri))*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro)) },
                                { (R2*R3*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri - Ag*Ri)) - R2*((R4 + R5)*R6*R7 + (R5 + R6)*R7*Ri + R3*(R5*(R6 + R7) + R4*(R5 + R6 + R7) + (R5 + R6 + R7)*Ri))*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),(R2*R3*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri - Ag*Ri)) - R1*R7*(R4*R5*R6 + R4*(R5 + R6 + Ag*R6)*Ri + R3*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri - Ag*Ri))) + (R1*(R4*R5*R6 + (R4 + R5)*R6*R7 + R4*(R5 + R6)*Ri + (R4 + R5 + R6)*R7*Ri + R3*(R5*(R6 + R7) + R4*(R5 + R6 + R7) + (R5 + R6 + R7)*Ri)) - R2*(R7*(R5*R6 + R4*(R5 + R6) + (R5 + R6)*Ri) + R3*(R5*(R6 + R7) + R4*(R5 + R6 + R7) + (R5 + R6 + R7)*Ri)))*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),(-(R1*R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri - Ag*Ri))) + R2*(R4*R5*R7 + R1*(R5*(R6 + R7) + R4*(R5 + R6 + R7) + (R5 + R6 + R7)*Ri))*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),(R2*R7*((R1 + R3)*R5*R6 + R3*(R5 - Ag*R5 + R6)*Ri + R1*(R5 + R6 + Ag*R6)*Ri) - R2*((R1 + R3)*R5*R6 + R5*(R3 + R6)*R7 + (R1 + R3)*(R5 + R6)*Ri + (R1 + R3 + R5 + R6)*R7*Ri)*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),(R2*R6*R7*((R1 + R3)*R4 - Ag*R1*Ri) - R2*((R1 + R3)*R4*R6 + R4*(R3 + R6)*R7 - R1*R7*Ri)*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),(R2*R5*R7*(-((R1 + R3)*R4) + Ag*R1*Ri) + R2*(R4*R5*(R3 + R7) + R1*R4*(R5 + R7) + R1*R7*(R5 + Ri))*Ro)/(R3*R7*(R2*R4*R5 + R2*R4*R6 + R2*R5*R6 + R4*R5*R6 - (R2 + R4)*((-1 + Ag)*R5 - R6)*Ri) - (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),(R2*(-(R3*R4*R5) + R1*(R4 + R5)*R6 + R1*(R5 + R6)*Ri)*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro) },
                                { (-(R3*R7*(R4*R5*R6 + R4*(R5 + R6 + Ag*R6)*Ri + R2*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri - Ag*Ri)))) + R3*(R4*R5*R6 + R4*(R5 + R6 + R7)*Ri + R2*(R5*(R6 + R7) + R4*(R5 + R6 + R7) + (R5 + R6 + R7)*Ri))*Ro)/(R3*R7*(R2*R4*R5 + R2*R4*R6 + R2*R5*R6 + R4*R5*R6 - (R2 + R4)*((-1 + Ag)*R5 - R6)*Ri) - (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),(R3*R7*(Ag*R4*(R5 + R6)*Ri - R1*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri - Ag*Ri))) + R3*(R4*R5*R7 + R1*(R5*(R6 + R7) + R4*(R5 + R6 + R7) + (R5 + R6 + R7)*Ri))*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),(R1*R3*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri - Ag*Ri)) + (R7*(R2*R4*R5 + R2*R4*R6 + R2*R5*R6 + R4*R5*R6 + (R2 + R4)*(R5 + R6)*Ri) - R1*R3*(R5*(R6 + R7) + R4*(R5 + R6 + R7) + (R5 + R6 + R7)*Ri))*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),(R2*R3*R7*(Ag*(R5 + R6)*Ri + R5*Ro) + R1*R3*((1 + Ag)*R6*R7*Ri + R5*(R6 + Ri)*(R7 - Ro) - (R6 + R7)*Ri*Ro))/(R3*R7*(R2*R4*R5 + R2*R4*R6 + R2*R5*R6 + R4*R5*R6 - (R2 + R4)*((-1 + Ag)*R5 - R6)*Ri) - (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),-((R3*(-(Ag*(R2 + R4)*R6*R7*Ri) + R1*R4*R6*(R7 - Ro) + R2*R4*R7*Ro + (R2 + R4)*R7*Ri*Ro + R1*R7*Ri*(-(Ag*R6) + Ro)))/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro)),(R3*R5*R7*(-(R1*R4) + Ag*(R1 + R2 + R4)*Ri) + R3*(R1*R4*(R5 + R7) + R1*R7*(R5 + Ri) + R4*R7*(R5 + Ri) + R2*R7*(R4 + R5 + Ri))*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),-((R3*(R2*R4*R5 + R1*R4*R6 + R2*R4*R6 + R1*R5*R6 + R2*R5*R6 + R4*R5*R6 + (R1 + R2 + R4)*(R5 + R6)*Ri)*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro)) },
                                { (R3*R4*R7*(R6*Ri + R5*(R6 + Ri - Ag*Ri)) - R4*(R3*R5*R6 + R5*(R2 + R3 + R6)*R7 + (R5 + R6)*R7*Ri + R3*(R5 + R6 + R7)*Ri)*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),((R1 + R3)*R4*R7*(R6*Ri + R5*(R6 + Ri - Ag*Ri)) - R4*((R1 + R3)*R5*R6 + R5*(R3 + R6)*R7 + (R1 + R3)*(R5 + R6)*Ri + (R1 + R3 + R5 + R6)*R7*Ri)*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),(-(R1*R4*R7*(R6*Ri + R5*(R6 + Ri - Ag*Ri))) + R4*(R1*R5*R6 - R2*R5*R7 + R1*(R5 + R6 + R7)*Ri)*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),(R7*(R1*R4*(R5*R6 + R2*(R5 + R6) + R3*(R5 + R6) + (R5 + R6 + Ag*R6)*Ri) - R2*R3*(R6*Ri + R5*(R6 + Ri - Ag*Ri))) - R1*R4*(R6*(R5 + R7) + R2*(R5 + R6 + R7) + R3*(R5 + R6 + R7) + (R5 + R6 + R7)*Ri)*Ro + R2*(R3*R5*(R6 + R7) + R6*R7*Ri + R3*(R5 + R6 + R7)*Ri + R5*R7*(R6 + Ri))*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),(-(R4*R6*R7*(R2*R3 + R1*(R2 + R3 + Ag*Ri))) + R4*(R2*R3*R6 + R2*(R3 + R6)*R7 + R1*(R2*(R6 + R7) + R3*(R6 + R7) + R7*(R6 + Ri)))*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),(R2*R4*R5*(R3*R7 - (R3 + R7)*Ro) + R1*R4*(R5*R7*(R2 + R3 + Ag*Ri) - (R2 + R3)*R5*Ro + R7*Ri*Ro))/(R3*R7*(R2*R4*R5 + R2*R4*R6 + R2*R5*R6 + R4*R5*R6 - (R2 + R4)*((-1 + Ag)*R5 - R6)*Ri) - (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),(R4*(R2*R3*R5 + R1*R5*(R2 + R3 + R6) + R1*(R5 + R6)*Ri)*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro) },
                                { (R3*R4*R5*R7*(R6 - Ag*Ri) - R5*(R3*R4*R6 + R4*(R2 + R3 + R6)*R7 + (R2 + R4)*R7*Ri)*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),(-((R1 + R3)*R4*R5*R7*(R6 - Ag*Ri)) + R5*((R1 + R3)*R4*R6 + R4*(R3 + R6)*R7 - R1*R7*Ri)*Ro)/(R3*R7*(R2*R4*R5 + R2*R4*R6 + R2*R5*R6 + R4*R5*R6 - (R2 + R4)*((-1 + Ag)*R5 - R6)*Ri) - (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),-((R5*(R1*R7*Ri*Ro + R4*R7*Ri*Ro + R2*R7*(R4 + Ri)*Ro - R1*R4*(Ag*R7*Ri + R6*(-R7 + Ro))))/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro)),((R2*R3 + R1*(R2 + R3))*R5*R7*(R6 - Ag*Ri) - R5*(R2*R3*R6 + R2*(R3 + R6)*R7 + R1*(R2*(R6 + R7) + R3*(R6 + R7) + R7*(R6 + Ri)))*Ro)/(R3*R7*(R2*R4*R5 + R2*R4*R6 + R2*R5*R6 + R4*R5*R6 - (R2 + R4)*((-1 + Ag)*R5 - R6)*Ri) - (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),(R7*(R3*R4*R5*(R6 + Ri - Ag*Ri) - R1*R6*(R3*R4 + (R3 + R4 + Ag*R4)*Ri) + R2*(-(R1*R6*(R4 + Ri)) + R3*R5*(R4 + R6 + Ri - Ag*Ri))) + (R1*R4*(R6*R7 + R3*(R6 + R7)) + R1*((R3 + R4)*R6 + (R3 + R4 + R6)*R7)*Ri - R4*R5*(R7*(R6 + Ri) + R3*(R6 + R7 + Ri)) - R2*(-(R1*(R6 + R7)*(R4 + Ri)) + R5*R7*(R4 + R6 + Ri) + R3*R5*(R4 + R6 + R7 + Ri)))*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),(R5*(R4*Ri + R2*(R4 + Ri))*(-(R3*R7) + (R3 + R7)*Ro) + R1*R5*(-((R2 + R3)*R4*R7) - (R2 + R3 + R4 + Ag*R4)*R7*Ri + (R2 + R3)*R4*Ro + (R2 + R3 + R4 + R7)*Ri*Ro))/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),(R5*(R1*R4*(R2 + R3 + R6) + R3*R4*Ri + R1*(R2 + R3 + R4)*Ri + R2*R3*(R4 + Ri))*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro) },
                                { -((R6*(R3*R4*(Ag*R7*Ri + R5*(R7 - Ro)) + R7*(R4*Ri + R2*(R4 + R5 + Ri))*Ro))/(R3*R7*(R2*R4*R5 + R2*R4*R6 + R2*R5*R6 + R4*R5*R6 - (R2 + R4)*((-1 + Ag)*R5 - R6)*Ri) - (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro)),(-((R1 + R3)*R4*R6*R7*(R5 + Ag*Ri)) + R6*(R4*R5*(R3 + R7) + R1*R4*(R5 + R7) + R1*R7*(R5 + Ri))*Ro)/(R3*R7*(R2*R4*R5 + R2*R4*R6 + R2*R5*R6 + R4*R5*R6 - (R2 + R4)*((-1 + Ag)*R5 - R6)*Ri) - (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),(-(R1*R4*R6*R7*(R5 + Ag*Ri)) + R6*(R1*R4*(R5 + R7) + R1*R7*(R5 + Ri) + R4*R7*(R5 + Ri) + R2*R7*(R4 + R5 + Ri))*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),((R2*R3 + R1*(R2 + R3))*R6*R7*(R5 + Ag*Ri) - R6*(R1*(R2 + R3)*R5 + R2*R5*(R3 + R7) - R1*R7*Ri)*Ro)/(R3*R7*(R2*R4*R5 + R2*R4*R6 + R2*R5*R6 + R4*R5*R6 - (R2 + R4)*((-1 + Ag)*R5 - R6)*Ri) - (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),(R6*R7*(-((R2*R3 + R1*(R2 + R3))*R4) + ((-1 + Ag)*R1*(R2 + R3) - R1*R4 + (-1 + Ag)*R3*(R2 + R4))*Ri) + R6*(R1*(R2 + R3)*R4 + R1*(R2 + R3 + R4 + R7)*Ri + (R3 + R7)*(R4*Ri + R2*(R4 + Ri)))*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),(R7*((-1 + Ag)*R3*R4*R5*Ri + R1*R2*R6*(R4 + R5 + Ri) + R1*R3*R6*(R4 + R5 + Ri) - R2*R3*R5*(R4 + Ri - Ag*Ri) + R1*R4*R6*(R5 + Ri + Ag*Ri)) + (R4*(R3*R5*R7 + R5*R7*Ri + R3*(R5 + R7)*Ri) - R1*R6*(R7*(R5 + Ri) + R3*(R4 + R5 + Ri) + R4*(R5 + R7 + Ri)) + R2*(R5*R7*(R4 + Ri) - R1*R6*(R4 + R5 + Ri) + R3*(R5*R7 + R4*(R5 + R7) + (R5 + R7)*Ri)))*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),-((R6*(R1*(R2 + R3)*(R4 + R5) + R1*(R2 + R3 + R4)*Ri + R3*R4*(R5 + Ri) + R2*R3*(R4 + R5 + Ri))*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro)) },
                                { -((Ag*R3*R4*(R5 + R6)*R7*Ri + R7*(R2*R4*R5 + R3*R4*R5 + R2*R4*R6 + R2*R5*R6 + R4*R5*R6 + (R2 + R4)*(R5 + R6)*Ri)*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro)),(-(Ag*(R1 + R3)*R4*(R5 + R6)*R7*Ri) + R7*(-(R3*R4*R5) + R1*(R4 + R5)*R6 + R1*(R5 + R6)*Ri)*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),(Ag*R1*R4*(R5 + R6)*R7*Ri - R7*(R2*R4*R5 + R1*R4*R6 + R2*R4*R6 + R1*R5*R6 + R2*R5*R6 + R4*R5*R6 + (R1 + R2 + R4)*(R5 + R6)*Ri)*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),(Ag*(R2*R3 + R1*(R2 + R3))*(R5 + R6)*R7*Ri + R7*(R2*R3*R5 + R1*R5*(R2 + R3 + R6) + R1*(R5 + R6)*Ri)*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),(-(Ag*(R3*(R2 + R4) + R1*(R2 + R3 + R4))*R6*R7*Ri) + R7*(R2*R3*R4 + R1*R4*(R2 + R3 + R6) + R3*(R2 + R4)*Ri + R1*(R2 + R3 + R4)*Ri)*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro),-((Ag*(R3*(R2 + R4) + R1*(R2 + R3 + R4))*R5*R7*Ri + R7*(R1*(R2 + R3)*(R4 + R5) + R1*(R2 + R3 + R4)*Ri + R3*R4*(R5 + Ri) + R2*R3*(R4 + R5 + Ri))*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro)),(R3*R7*(R2*R4*R5 + R2*R4*R6 + R2*R5*R6 + R4*R5*R6 - (R2 + R4)*((-1 + Ag)*R5 - R6)*Ri) + (-((R4*R5*(R3 + R6) + R2*(R4*R5 + R3*(R4 + R5) + (R4 + R5)*R6))*R7) - (R2 + R4)*(R3 + R5 + R6)*R7*Ri + R1*(R2*R4*R5 + R3*R4*R5 + R2*R4*R6 + R3*R4*R6 + R2*R5*R6 + R3*R5*R6 + R4*R5*R6 + (R2 + R3 + R4)*(R5 + R6)*Ri))*Ro)/(-(R3*(R2*R4*R5 + R4*R5*R6 + R2*(R4 + R5)*R6)*R7) + R3*(R2 + R4)*((-1 + Ag)*R5 - R6)*R7*Ri + (R4*R5*R6*R7 + R3*R4*R5*(R6 + R7) + R4*(R5 + R6)*R7*Ri + R3*R4*(R5 + R6 + R7)*Ri + R2*R7*(R4*(R5 + R6) + R6*Ri + R5*(R6 + Ri)) + R2*R3*(R4*(R5 + R6 + R7) + (R6 + R7)*Ri + R5*(R6 + R7 + Ri)))*Ro) } });

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
    std::cout << "Sallen-Key LPF test\n";

    static constexpr float fs = 48000.0f;

    Reference_WDF ref {};
    ref.prepare (fs);

    Impedances impedances {};
    Params params {};
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
