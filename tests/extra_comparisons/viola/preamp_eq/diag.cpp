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
void diag(const double v[4], double d[16])
{
  std::memset(&d[0], 0, 16U * sizeof(double));
  d[0] = v[0];
  d[5] = v[1];
  d[10] = v[2];
  d[15] = v[3];
}

} // namespace coder

// End of code generation (diag.cpp)
