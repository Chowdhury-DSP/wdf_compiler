#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "bassman_tone_stack_c.h"

static float* get_data(size_t* count)
{
    FILE* file = fopen ("data.bin", "rb");
    fseek (file, 0, SEEK_END);
    long file_size = ftell (file);
    rewind (file);

    char* buffer = malloc (file_size);
    size_t bytes_read = fread (buffer, 1, file_size, file);
    assert (bytes_read == file_size);

    *count = bytes_read / sizeof (float);
    return (float*) buffer;
}

int main()
{
    printf ("Bassman Tone Stack test\n");

    size_t count;
    float* ref_output = get_data (&count);

    const float fs = 48000.0f;

    struct Params params = {
        .Vin_Res3m_res_value = 12.5e3f,
        .Res2_Res3p_value = 512.5e3f,
        .Res1p_Res1m_value = 250.0e3f,
        .Cap1_value = 2.5e-10f,
        .Cap2_value = 2.0e-08f,
        .Res4_value = 5.6e+04f,
        .Cap3_value = 2.0e-08f,
    };
    struct Impedances impedances;
    calc_impedances (&impedances, fs, params);
    struct State state = {};

    float max_error = 0.0f;
    for (size_t n = 0; n < count; ++n)
    {
        float v = 1.0f;
        float v_Res1p_Res1m_Cap1, v_Vin_Res3m;
        process (&state, &impedances, v, &v_Res1p_Res1m_Cap1, &v_Vin_Res3m);
        float test_output = v_Res1p_Res1m_Cap1 + (v_Vin_Res3m - v);
        float error = fabsf (test_output - ref_output[n]);
        max_error = fmaxf (error, max_error);
    }
    printf ("Max error: %e\n", max_error);
    free (ref_output);

    if (max_error > 1.0e-4f)
    {
        printf("Error is too large... failing test!\n");
        return 1;
    }

    return 0;
};
