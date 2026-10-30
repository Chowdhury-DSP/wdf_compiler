//
// mldivide.cpp
//
// Code generation for function 'mldivide'
//

// Include files
#include "mldivide.h"
#include <cmath>
#include <cstring>

// Function Definitions
namespace coder {
void mldivide(const double A[4], const double B[8], double Y[8])
{
  double a21;
  double a22;
  double a22_tmp;
  double d;
  double d1;
  int r1;
  int r2;
  if (std::abs(A[1]) > std::abs(A[0])) {
    r1 = 1;
    r2 = 0;
  } else {
    r1 = 0;
    r2 = 1;
  }
  a21 = A[r2] / A[r1];
  a22_tmp = A[r1 + 2];
  a22 = A[r2 + 2] - a21 * a22_tmp;
  d = (B[r2] - B[r1] * a21) / a22;
  Y[1] = d;
  Y[0] = (B[r1] - d * a22_tmp) / A[r1];
  d1 = B[r1 + 2];
  d = (B[r2 + 2] - d1 * a21) / a22;
  Y[3] = d;
  Y[2] = (d1 - d * a22_tmp) / A[r1];
  d1 = B[r1 + 4];
  d = (B[r2 + 4] - d1 * a21) / a22;
  Y[5] = d;
  Y[4] = (d1 - d * a22_tmp) / A[r1];
  d1 = B[r1 + 6];
  d = (B[r2 + 6] - d1 * a21) / a22;
  Y[7] = d;
  Y[6] = (d1 - d * a22_tmp) / A[r1];
}

} // namespace coder

// End of code generation (mldivide.cpp)
