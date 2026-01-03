#include <xsimd/xsimd.hpp>
#include "../chowdsp_wdf.h"
#include <iostream>
#include <fstream>
#include <random>

#include "../performance_counters/event_counter.h"

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
    Reference_WDF()
    {
        R.impedanceCalculator = [] (auto& R) {
            const auto Ra = R.getPortImpedance (0);
            const auto Rb = R.getPortImpedance (1);
            const auto Rc = R.getPortImpedance (2);
            const auto Rd = R.getPortImpedance (3);
            const auto Re = R.getPortImpedance (4);

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
        };
    }

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

        return Rl.voltage();
    }

    static constexpr auto Pt = 100.0e3f;
    static constexpr auto Pb = 100.0e3f;

    // Port A
    chowdsp::wdf::ResistorCapacitorSeries<float> S4 { 1.0f, 6.4e-9f };

    // Port B
    chowdsp::wdf::ResistorCapacitorSeries<float> S5 { 1.0f, 64.0e-9f };
    chowdsp::wdf::Resistor<float> Rl { 1.0e6f };
    chowdsp::wdf::WDFParallel<float> P1 { &Rl, &S5 };

    // Port C
    chowdsp::wdf::Resistor<float> Resc { 10.0e3f };

    // Port D
    chowdsp::wdf::ResistorCapacitorParallel<float> P3 { 1.0f, 220.0e-9f };
    chowdsp::wdf::Resistor<float> Resb { 1.0e3f };
    chowdsp::wdf::WDFSeries<float> S3 { &Resb, &P3 };

    // Port E
    chowdsp::wdf::ResistorCapacitorParallel<float> P2 { 1.0f, 22.0e-9f };
    chowdsp::wdf::Resistor<float> Resa { 10.0e3f };
    chowdsp::wdf::WDFSeries<float> S2 { &Resa, &P2 };

    chowdsp::wdf::RtypeAdaptor<float> R { { &S4, &P1, &Resc, &S3, &S2 }, 5 };

    // Port F
    chowdsp::wdf::Capacitor<float> Ca { 1.0e-6f };
    chowdsp::wdf::WDFSeries<float> S1 { &R, &Ca };
    chowdsp::wdf::IdealVoltageSource<float> Vin { &S1 };
};


int main()
{
    std::cout << "Baxandall EQ test (chowdsp_wdf polymorphic)\n";

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
