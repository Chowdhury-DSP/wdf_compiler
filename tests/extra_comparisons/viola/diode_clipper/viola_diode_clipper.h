//
// viola_diode_clipper.h
//
// Code generation for function 'viola_diode_clipper'
//

#ifndef VIOLA_DIODE_CLIPPER_H
#define VIOLA_DIODE_CLIPPER_H

// Include files
#include "rtwtypes.h"
#include <cstddef>
#include <cstdlib>

// Function Declarations
extern void viola_diode_clipper(const double x[64], int n, bool doReset,
                                double y[64]);

void viola_diode_clipper_delete();

void viola_diode_clipper_emx_free();

void viola_diode_clipper_emx_init();

void viola_diode_clipper_init();

void viola_diode_clipper_new();

#endif
// End of code generation (viola_diode_clipper.h)
