//
// viola_preamp_eq.h
//
// Code generation for function 'viola_preamp_eq'
//

#ifndef VIOLA_PREAMP_EQ_H
#define VIOLA_PREAMP_EQ_H

// Include files
#include "rtwtypes.h"
#include <cstddef>
#include <cstdlib>

// Function Declarations
extern void viola_preamp_eq(const double x[64], int n, bool doReset,
                            double y[64]);

void viola_preamp_eq_delete();

void viola_preamp_eq_emx_free();

void viola_preamp_eq_emx_init();

void viola_preamp_eq_init();

void viola_preamp_eq_new();

#endif
// End of code generation (viola_preamp_eq.h)
