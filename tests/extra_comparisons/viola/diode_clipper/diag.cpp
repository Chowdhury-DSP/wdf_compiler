//
// diag.cpp
//
// Code generation for function 'diag'
//

// Include files
#include "diag.h"
#include "coder_array.h"
#include <cstring>

// Function Definitions
namespace coder {
void diag(const array<double, 2U> &v, array<double, 1U> &d)
{
  if ((v.size(0) == 1) && (v.size(1) == 1)) {
    d.set_size(1);
    d[0] = v[0];
  } else {
    int u0;
    int u1;
    u0 = v.size(0);
    u1 = v.size(1);
    if (u0 <= u1) {
      u1 = u0;
    }
    if (v.size(1) <= 0) {
      u1 = 0;
    }
    d.set_size(u1);
    for (int k{0}; k < u1; k++) {
      d[k] = v[k + v.size(0) * k];
    }
  }
}

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
