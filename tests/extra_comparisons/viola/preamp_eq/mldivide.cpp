//
// mldivide.cpp
//
// Code generation for function 'mldivide'
//

// Include files
#include "mldivide.h"
#include <algorithm>
#include <cmath>
#include <cstring>

// Function Definitions
namespace coder {
void mldivide(const double A[16], double B[36])
{
  double b_A[16];
  double smax;
  int b;
  int jA;
  int jBcol;
  int mmj;
  signed char ipiv[4];
  std::copy(&A[0], &A[16], &b_A[0]);
  ipiv[0] = 1;
  ipiv[1] = 2;
  ipiv[2] = 3;
  ipiv[3] = 4;
  for (int j{0}; j < 3; j++) {
    signed char i;
    mmj = 2 - j;
    b = j * 5;
    jA = 5 - j;
    jBcol = 0;
    smax = std::abs(b_A[b]);
    for (int k{2}; k < jA; k++) {
      double s;
      s = std::abs(b_A[(b + k) - 1]);
      if (s > smax) {
        jBcol = k - 1;
        smax = s;
      }
    }
    if (b_A[b + jBcol] != 0.0) {
      if (jBcol != 0) {
        jA = j + jBcol;
        ipiv[j] = static_cast<signed char>(jA + 1);
        smax = b_A[j];
        b_A[j] = b_A[jA];
        b_A[jA] = smax;
        smax = b_A[j + 4];
        b_A[j + 4] = b_A[jA + 4];
        b_A[jA + 4] = smax;
        smax = b_A[j + 8];
        b_A[j + 8] = b_A[jA + 8];
        b_A[jA + 8] = smax;
        smax = b_A[j + 12];
        b_A[j + 12] = b_A[jA + 12];
        b_A[jA + 12] = smax;
      }
      jA = (b - j) + 4;
      for (int k{b + 2}; k <= jA; k++) {
        b_A[k - 1] /= b_A[b];
      }
    }
    jA = b;
    for (int k{0}; k <= mmj; k++) {
      smax = b_A[(b + (k << 2)) + 4];
      if (smax != 0.0) {
        jBcol = (jA - j) + 8;
        for (int ijA{jA + 6}; ijA <= jBcol; ijA++) {
          b_A[ijA - 1] += b_A[((b + ijA) - jA) - 5] * -smax;
        }
      }
      jA += 4;
    }
    i = ipiv[j];
    if (i != j + 1) {
      for (int k{0}; k < 9; k++) {
        jA = k << 2;
        jBcol = j + jA;
        smax = B[jBcol];
        jA = (i + jA) - 1;
        B[jBcol] = B[jA];
        B[jA] = smax;
      }
    }
  }
  for (int k{0}; k < 9; k++) {
    jBcol = k << 2;
    for (int ijA{0}; ijA < 4; ijA++) {
      jA = ijA << 2;
      mmj = ijA + jBcol;
      if (B[mmj] != 0.0) {
        for (int j{ijA + 2}; j < 5; j++) {
          b = (j + jBcol) - 1;
          B[b] -= B[mmj] * b_A[(j + jA) - 1];
        }
      }
    }
  }
  for (int k{0}; k < 9; k++) {
    jA = k << 2;
    for (int ijA{3}; ijA >= 0; ijA--) {
      jBcol = ijA << 2;
      mmj = ijA + jA;
      smax = B[mmj];
      if (smax != 0.0) {
        B[mmj] = smax / b_A[ijA + jBcol];
        for (int j{0}; j < ijA; j++) {
          b = j + jA;
          B[b] -= B[mmj] * b_A[j + jBcol];
        }
      }
    }
  }
}

} // namespace coder

// End of code generation (mldivide.cpp)
