#pragma once

#include <cmath>

namespace nl_inductor
{
struct L_Params
{
    float value {};
};

struct L_State
{
    float z {};
};

static inline float update_vars (const L_Params& params,
                                 float fs)
{
    return 2.0f * params.value * fs;
}

static inline float reflected (const L_State& state)
{
    return -state.z;
}

static inline void incident (L_State& state, float a)
{
    state.z = a / (std::sqrt (a * a + 1.0f));
}
}
