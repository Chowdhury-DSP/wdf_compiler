#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "preamp_eq_c.h"

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
    printf ("Pre-amp EQ test\n");

    size_t count;
    float* ref_output = get_data (&count);

    const float fs = 48000.0f;

    struct Params params = {
        .lfc_cap_value = 4.7e-09f,
        .r_series_value = 1.0e+04f,
        .hfl_res_value = 8.0e+04f,
        .hfl_cap_value = 2.7e-09f,
        .hfc_cap_value = 3.3e-06f,
        .r_shunt_value = 4.674e+03f,
        .lfl_res_value = 1.5e+04f,
        .lfl_cap_value = 1.5e-07f,
    };
    struct Impedances impedances;
    calc_impedances (&impedances, fs, params);
    struct State state = {};

    float max_error = 0.0f;
    for (size_t n = 0; n < count; ++n)
    {
        float test_output = process (&state, &impedances, 1.0f);
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
