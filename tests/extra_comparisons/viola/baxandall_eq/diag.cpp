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
void b_diag(const double v[5], double d[25])
{
  std::memset(&d[0], 0, 25U * sizeof(double));
  for (int j{0}; j < 5; j++) {
    d[j + 5 * j] = v[j];
  }
}

void diag(const double v[8], double d[64])
{
  std::memset(&d[0], 0, 64U * sizeof(double));
  for (int j{0}; j < 8; j++) {
    d[j + (j << 3)] = v[j];
  }
}

} // namespace coder

// End of code generation (diag.cpp)
