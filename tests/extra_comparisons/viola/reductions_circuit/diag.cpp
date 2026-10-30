//
// diag.cpp
//
// Code generation for function 'diag'
//

// Include files
#include "diag.h"
#include <cstring>

// Function Definitions
namespace coder {
void b_diag(const double v[8], double d[64])
{
  std::memset(&d[0], 0, 64U * sizeof(double));
  for (int j{0}; j < 8; j++) {
    d[j + (j << 3)] = v[j];
  }
}

void diag(const double v[11], double d[121])
{
  std::memset(&d[0], 0, 121U * sizeof(double));
  for (int j{0}; j < 11; j++) {
    d[j + 11 * j] = v[j];
  }
}

} // namespace coder

// End of code generation (diag.cpp)
