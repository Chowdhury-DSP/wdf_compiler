#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "channel_hpf_c.h"

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
    printf ("Channel HPF test\n");

    size_t count;
    float* ref_output = get_data (&count);

    const float fs = 48000.0f;

    struct Params params = {
        .Sw1_value = 1.0f,
        .R8_value = 1.0e+04,
        .Lh_value = 3.0e+00,
        .Ll_value = 7.0e+00,
        .C17_Vin_cap_value = 4.7e-07,
        .C18_value = 4.7e-07,
        .R9_value = 5.1e+03,
    };
    struct Impedances impedances;
    calc_impedances (&impedances, fs, params);
    struct State state = {};

    float max_error = 0.0f;
    for (size_t n = 0; n < count / 2; ++n)
    {
        float test_output = process (&state, &impedances, n == 0 ? 1.0f : 0.0f);
        float error = fabsf (test_output - ref_output[n]);
        max_error = fmaxf (error, max_error);
    }

    params.Sw1_value = 0.0f;
    calc_impedances (&impedances, fs, params);
    for (size_t n = 0; n < count / 2; ++n)
    {
        float test_output = process (&state, &impedances, n == 0 ? 1.0f : 0.0f);
        float error = fabsf (test_output - ref_output[n + count / 2]);
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
