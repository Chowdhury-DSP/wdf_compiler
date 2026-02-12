#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "rc_lowpass_double_c.h"

static double* get_data(size_t* count)
{
    FILE* file = fopen ("data.bin", "rb");
    fseek (file, 0, SEEK_END);
    long file_size = ftell (file);
    rewind (file);

    char* buffer = malloc (file_size);
    size_t bytes_read = fread (buffer, 1, file_size, file);
    assert (bytes_read == file_size);

    *count = bytes_read / sizeof (double);
    return (double*) buffer;
}

int main()
{
    printf ("RC Lowpass test (double-precision)\n");

    size_t count;
    double* ref_output = get_data (&count);

    const float fs = 48000.0;

    struct Params params = {
        .R1_value = 1.0e+03,
        .C1_value = 1.0e-06,
    };
    struct Impedances impedances;
    calc_impedances (&impedances, fs, params);
    struct State state = {};

    double max_error = 0.0;
    for (size_t n = 0; n < count; ++n)
    {
        double test_output = process (&state, &impedances, 1.0);
        double error = fabs (test_output - ref_output[n]);
        max_error = fmax (error, max_error);
    }
    printf ("Max error: %e\n", max_error);
    free (ref_output);

    if (max_error > 1.0e-4)
    {
        printf("Error is too large... failing test!\n");
        return 1;
    }

    return 0;
};
