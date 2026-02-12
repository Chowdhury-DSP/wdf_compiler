#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "reductions_circuit_c.h"

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
    printf ("Reductions Circuit test\n");

    size_t count;
    float* ref_output = get_data (&count);

    const float fs = 48000.0f;

    struct Params params = {
        .Rv1_value = 1.0e+04f,
        .Rv2_value = 5.0e+03f,
        .Rl_value = 1.0e+05f,
        .R7_value = 1.0e+04f,
        .C7_value = 1.0e-05f,
        .R8_value = 1.0e+05f,
        .C8_value = 9.999999e-08f,
        .R1_value = 1.0e+03f,
        .R2_value = 5.0e3f,
        .R3_value = 4.0e+03f,
        .R4_value = 10.0e3f,
        .R5_value = 3.0e+03f,
        .C1_value = 1.0e-06f,
        .C2_value = 5.0e-6f,
        .C3_value = 4.0e-09f,
        .C4_value = 10.0e-9f,
        .C5_value = 3.0e-06f,
        .R6_value = 1.0e+04f,
        .C6_value = 4.0e-06f,
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
