#pragma once

#include <wdf_lib_rtype_helpers.h>

static constexpr int num_ports = 7;
static constexpr int num_ports_padded = wdf_lib::pad_to_multiple (num_ports, 4);
static constexpr int up_port = 0;

struct SK_LPF_R_Params
{
};

struct SK_LPF_R_Vars
{
    alignas (16) float S[num_ports * num_ports_padded] {};
};

// Note: sk_lpf.wdf's Custom(...) declares 6 children (R2, C2, C1, Rg, Rf,
// Rload), so this only takes 6 R/G pairs -- there is no input pair for the
// adapt port itself (up_port), since that impedance is what's being solved
// for here, not a known input.
static inline float SK_LPF_update_vars (SK_LPF_R_Vars* vars,
                                 const SK_LPF_R_Params* /*params*/,
                                 float R2,
                                 float G2,
                                 float R3,
                                 float G3,
                                 float R4,
                                 float G4,
                                 float R5,
                                 float G5,
                                 float R6,
                                 float G6,
                                 float /*R7*/,
                                 float /*G7*/)
{
    const auto R1 = (R3*(R2 + R4)*R5)/((R2 + R3)*R5 - R4*R6);

    const float S_transpose[num_ports][num_ports] {
        {0,(R1*(-(R3*R5) + R4*(R5 + R6)))/(R3*(R2 + R4)*R5),-(R1/R3),-1 - (R1*R6)/(R3*R5),(R1*R6)/(R3*R5),R1/R3,0},
        {-(R2/(R2 + R4)),(R1*R3*R5 - R2*R3*R5 - R1*R4*R6)/(R2*R3*R5 + R3*R4*R5),(R1*R2)/(R2*R3 + R3*R4),(R2*(-(R3*R5) + R1*R6))/(R3*(R2 + R4)*R5),-((R1*R2*R6)/(R2*R3*R5 + R3*R4*R5)),-((R1*R2)/(R2*R3 + R3*R4)),0},
        {(-(R2*R5) + R4*R6)/((R2 + R4)*R5),(R1*R5 + R4*(R5 + R6))/((R2 + R4)*R5),-(R1/(R2 + R4)),-((R1*R6 + R2*(R5 + R6))/((R2 + R4)*R5)),((R1 + R2 + R4)*R6)/((R2 + R4)*R5),1 + R1/(R2 + R4),0},
        {-(R4/(R2 + R4)),-(((R1 + R3)*R4)/(R3*(R2 + R4))),(R1*R4)/(R2*R3 + R3*R4),(R2*R3*R5 + R1*R4*R6)/(R2*R3*R5 + R3*R4*R5),-((R1*R4*R6)/(R2*R3*R5 + R3*R4*R5)),-((R1*R4)/(R2*R3 + R3*R4)),0},
        {-(R4/(R2 + R4)),-(((R1 + R3)*R4)/(R3*(R2 + R4))),(R1*R4)/(R2*R3 + R3*R4),(R2*R3 + R1*(R2 + R3))/(R3*(R2 + R4)),-1 - (R1*R4*R6)/(R2*R3*R5 + R3*R4*R5),-((R1*R4)/(R2*R3 + R3*R4)),0},
        {(R4*R6)/(R2*R5 + R4*R5),((R1 + R3)*R4*R6)/(R3*(R2 + R4)*R5),-((R1*R4*R6)/(R2*R3*R5 + R3*R4*R5)),-(((R2*R3 + R1*(R2 + R3))*R6)/(R3*(R2 + R4)*R5)),((R1*(R2 + R3) + R3*(R2 + R4))*R6)/(R3*(R2 + R4)*R5),1 + (R1*R4*R6)/(R2*R3*R5 + R3*R4*R5),0},
        {-((R4*(R5 + R6))/((R2 + R4)*R5)),-(((R1 + R3)*R4*(R5 + R6))/(R3*(R2 + R4)*R5)),(R1*R4*(R5 + R6))/(R3*(R2 + R4)*R5),((R2*R3 + R1*(R2 + R3))*(R5 + R6))/(R3*(R2 + R4)*R5),-(((R3*(R2 + R4) + R1*(R2 + R3 + R4))*R6)/(R3*(R2 + R4)*R5)),-((2*R2*R3*R5 + R1*R4*R5 + 2*R3*R4*R5 + R1*R4*R6)/(R2*R3*R5 + R3*R4*R5)),-1},
    };

    for (int c = 0; c < num_ports; ++c)
    {
        for (int r = 0; r < num_ports; ++r)
            vars->S[r * num_ports_padded + c] = S_transpose[c][r];
    }

    return R1;
}

static inline float SK_LPF_reflected (const SK_LPF_R_Vars* vars, const float* a_in)
{
    // S[up_port][up_port] == 0, so this doesn't need a fresh a[up_port].
    alignas (16) float a[num_ports] {};
    for (int i = 0, j = 0; i < num_ports; ++i)
    {
        if (i != up_port)
        {
            a[i] = a_in[j];
            j++;
        }
    }

    return wdf_lib::single_output_matmul<num_ports, num_ports_padded> (vars->S, a, up_port);
}

static inline void SK_LPF_incident (const SK_LPF_R_Vars* vars, float a_up, const float* a_in, float* b_out)
{
    alignas (16) float a[num_ports];
    alignas (16) float b[num_ports_padded];

    for (int i = 0, j = 0; i < num_ports; ++i)
    {
        if (i == up_port)
        {
            a[i] = a_up;
        }
        else
        {
            a[i] = a_in[j];
            j++;
        }
    }

    wdf_lib::aligned_matmul<num_ports, num_ports_padded> (vars->S, a, b);

    for (int i = 0, j = 0; i < num_ports; ++i)
    {
        if (i != up_port)
        {
            b_out[j] = b[i];
            j++;
        }
    }
}
