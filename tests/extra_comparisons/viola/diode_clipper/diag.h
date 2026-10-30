//
// diag.h
//
// Code generation for function 'diag'
//

#ifndef DIAG_H
#define DIAG_H

// Include files
#include "rtwtypes.h"
#include "coder_array.h"
#include <cstddef>
#include <cstdlib>

// Function Declarations
namespace coder {
void diag(const array<double, 2U> &v, array<double, 1U> &d);

void diag(const double v[4], double d[16]);

} // namespace coder

#endif
// End of code generation (diag.h)
