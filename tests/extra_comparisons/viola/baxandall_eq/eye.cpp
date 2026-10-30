//
// eye.cpp
//
// Code generation for function 'eye'
//

// Include files
#include "eye.h"
#include "coder_array.h"
#include <cstring>

// Function Definitions
namespace coder {
void eye(double varargin_1, array<double, 2U> &b_I)
{
  double t;
  int loop_ub;
  int m;
  if (varargin_1 < 0.0) {
    t = 0.0;
  } else {
    t = varargin_1;
  }
  m = static_cast<int>(t);
  b_I.set_size(static_cast<int>(t), static_cast<int>(t));
  loop_ub = static_cast<int>(t) * static_cast<int>(t);
  for (int k{0}; k < loop_ub; k++) {
    b_I[k] = 0.0;
  }
  if (static_cast<int>(t) > 0) {
    for (int k{0}; k < m; k++) {
      b_I[k + b_I.size(0) * k] = 1.0;
    }
  }
}

} // namespace coder

// End of code generation (eye.cpp)
