#pragma once

#include <cmath>

struct HC_Params
{
    float A = 1.0f;
};

struct HC_Vars
{
    float A = 1.0f;
};

static inline void update_vars (HC_Vars& vars,
                                const HC_Params& params,
                                [[maybe_unused]] float child_R,
                                [[maybe_unused]] float child_G)
{
    vars.A = params.A;
}

static inline float root_compute (const HC_Vars& vars, float a)
{
    float b;
    if (std::abs (a) < vars.A * 0.5f)
        b = a;
    else
        b = -a + (a >= 0.0f ? vars.A : -vars.A);
    return b;
}
