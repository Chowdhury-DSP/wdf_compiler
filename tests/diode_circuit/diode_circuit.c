#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "diode_circuit_c.h"

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
    printf ("Diode Circuit test\n");

    size_t total_count;
    float* data = get_data (&total_count);
    size_t count = total_count / 2;
    float* input = data;
    float* ref_output = data + count;

    const float fs = 48000.0f;

#if NETLIST
    struct Params params = {
        .C1_value = 1.0e-9,
        .V1_res_value = 1.0e+02,
        .DP_params = {
            .Is = 1.0e-9,
        },
    };
#else
    struct Params params = {
        .C1_value = 1.0e-9,
        .Vin_res_value = 1.0e+02,
        .DP_params = {
            .Is = 1.0e-9,
        },
    };
#endif
    struct Impedances impedances;
    calc_impedances (&impedances, fs, params);
    struct State state = {};

    float max_error = 0.0f;
    for (size_t n = 0; n < count; ++n)
    {
        float test_output = process (&state, &impedances, input[n]);
        float error = fabsf (test_output - ref_output[n]);
        max_error = fmaxf (error, max_error);
    }
    printf ("Max error: %e\n", max_error);
    free (data);

    if (max_error > 1.0e-4f)
    {
        printf("Error is too large... failing test!\n");
        return 1;
    }

    return 0;
};
