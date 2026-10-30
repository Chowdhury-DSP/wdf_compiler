//
// viola_rc_lowpass.h
//
// Code generation for function 'viola_rc_lowpass'
//

#ifndef VIOLA_RC_LOWPASS_H
#define VIOLA_RC_LOWPASS_H

// Include files
#include "rtwtypes.h"
#include <cstddef>
#include <cstdlib>

// Function Declarations
extern void viola_rc_lowpass(const double x[64], int n, bool doReset,
                             double y[64]);

void viola_rc_lowpass_delete();

void viola_rc_lowpass_emx_free();

void viola_rc_lowpass_emx_init();

void viola_rc_lowpass_init();

void viola_rc_lowpass_new();

#endif
// End of code generation (viola_rc_lowpass.h)
