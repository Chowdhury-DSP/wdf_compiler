#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "reductions_circuit2_c.h"

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
    printf ("Reductions Circuit 2 test\n");

    size_t count;
    float* ref_output = get_data (&count);

    const float fs = 48000.0f;

    struct Params params = {
        .Vb_v_value = 1.5e+00f,
        .Rp_value = 4.0e+03f,
        .S4_v_value = 1.2e+00f,
        .Vcc_cap_value = 1.0e-6f,
        .C1_value = 1.0e-08f,
        .Cp2_value = 7.0e-09f,
        .Vin_res_value = 5.0e+03f,
        .R1_value = 1.0e+04f,
        .R2_value = 11.0e3f,
        .Cp_value = 4.0e-06f,
        .Rp2_value = 7.0e+03f,
        .Rl_v_value = -1.1e+00f,
        .Rl_res_value = 1.0e+05f,
        .Rl_cap_value = 1.0e-05f,
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
