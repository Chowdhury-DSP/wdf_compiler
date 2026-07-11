#pragma once

#include <wdf_lib_rtype_helpers.h>

namespace rat_drive_rtype
{
static constexpr int num_ports = 4;
static constexpr int num_ports_padded = wdf_lib::pad_to_multiple (num_ports, 4);
static constexpr int up_port = 3;

struct R_Params
{
};

struct R_Vars
{
    alignas (16) float S[num_ports * num_ports_padded] {};
};

static inline float update_vars (R_Vars* vars,
                                 const R_Params* /*params*/,
                                 float Ra,
                                 float Ga,
                                 float Rb,
                                 float Gb,
                                 float Rc,
                                 float Gc)
{
    static constexpr float Ag = 100.0f; // op-amp gain
    static constexpr float Ri = 10.0e6f; // op-amp input impedance
    static constexpr float Ro = 1.0e-1f; // op-amp output impedance

    const auto Rd = -(((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro) / (Ra * Rb + Ra * Rc + Rb * Rc + Rb * Ri + Ag * Rb * Ri + Rc * Ri - (Ra + Rb + Ri) * Ro));

    const float S_transpose[num_ports][num_ports] {
        { (Ra * Rd * (Rb + Rc - Ro) + Rc * Ri * Ro + Rb * (Rc + Ri) * Ro) / ((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro), (Ra * (-(Rc * Rd) + (Rc + Rd) * Ro)) / ((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro), (Ra * Rb * (Rd - Ro)) / ((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro), -((Ra * Rb) / (Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri))) },
        { (-(Rb * Rd * (Rc + Ag * Ri)) + Rb * (Rc + Rd) * Ro) / ((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro), -((Rc * Rd * (Ra + Ri) + (Ra * (Rb - Rd) - Rd * Ri + Rb * (Rc + Ri)) * Ro) / ((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro)), (Rb * (Ra + Ri) * (Rd - Ro)) / ((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro), -((Rb * (Ra + Ri)) / (Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri))) },
        { -((Rc * (Ag * Rd * Ri + Rb * (-Rd + Ro))) / ((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro)), (Rc * Rd * (Ra + Ri + Ag * Ri) - Rc * (Ra + Ri) * Ro) / ((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro), (Rc * Rd * (Ra + Rb + Ri) + Rb * (Ra + Ri) * Ro) / ((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro), -((Rc * (Ra + Rb + Ri)) / (Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri))) },
        { (Rd * (Ag * (Rb + Rc) * Ri - Rb * Ro)) / ((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro), -((Rd * (Ag * Rc * Ri + (Ra + Ri) * Ro)) / ((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro)), -((Rd + Ro) / Ro), 0 }
    };

    for (int c = 0; c < num_ports; ++c)
    {
        for (int r = 0; r < num_ports; ++r)
            vars->S[r * num_ports + c] = S_transpose[c][r];
    }

    return Rd;
}

static inline float reflected (const R_Vars* vars, const float* a_in)
{
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

static inline void incident (const R_Vars* vars, float a_up, const float* a_in, float* b_out)
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
}
