#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "rc_bandpass_var_c.h"

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
    printf ("RC Bandpass test (variable)\n");

    size_t count;
    float* ref_output = get_data (&count);

    const float fs = 48000.0f;

    struct Params params = {
        .C1_value = 1.0e-6f,
        .R1_value = 1.0e3f,
        .R2_value = 1.0e3f,
        .C2_value = 1.0e-6f,
    };
    struct Impedances impedances;
    calc_impedances (&impedances, fs, params);
    struct State state = {};

    float max_error = 0.0f;
    for (size_t n = 0; n < count / 3; ++n)
    {
        float test_output = process (&state, &impedances, n == 0 ? 1.0f : 0.0f);
        float error = fabsf (test_output - ref_output[n]);
        max_error = fmaxf (error, max_error);
    }

    params.R2_value = 100.0f;
    params.C2_value = 1.0e-9f;
    calc_impedances_lpf2 (&impedances, fs, params);
    for (size_t n = 0; n < count / 3; ++n)
    {
        float test_output = process (&state, &impedances, n == 0 ? 1.0f : 0.0f);
        float error = fabsf (test_output - ref_output[n + count / 3]);
        max_error = fmaxf (error, max_error);
    }

    params.R1_value = 10.0e3f;
    params.C1_value = 1.0e-4f;
    calc_impedances_lpf1 (&impedances, fs, params);
    for (size_t n = 0; n < count / 3; ++n)
    {
        float test_output = process (&state, &impedances, n == 0 ? 1.0f : 0.0f);
        float error = fabsf (test_output - ref_output[n + 2 * count / 3]);
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
