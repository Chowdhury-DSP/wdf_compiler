#pragma once

#include <math.h>

struct HC_Params
{
    float A;
};

struct HC_Vars
{
    float A;
};

static inline void update_vars (struct HC_Vars* vars,
                                const struct HC_Params* params,
                                float /*child_R*/,
                                float /*child_G*/)
{
    vars->A = params->A;
}

static inline float root_compute (const struct HC_Vars* vars, float a)
{
    float b;
    if (fabsf (a) < vars->A * 0.5f)
        b = a;
    else
        b = -a + (a >= 0.0f ? vars->A : -vars->A);
    return b;
}
