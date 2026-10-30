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
void mldivide(const double A[64], double B[160])
{
  double b_A[64];
  double smax;
  int jA;
  int jBcol;
  int mmj;
  int temp_tmp;
  signed char ipiv[8];
  std::copy(&A[0], &A[64], &b_A[0]);
  for (int k{0}; k < 8; k++) {
    ipiv[k] = static_cast<signed char>(k + 1);
  }
  for (int j{0}; j < 7; j++) {
    int b;
    signed char i;
    mmj = 6 - j;
    b = j * 9;
    jA = 9 - j;
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
        jBcol += j;
        ipiv[j] = static_cast<signed char>(jBcol + 1);
        for (int k{0}; k < 8; k++) {
          jA = k << 3;
          temp_tmp = j + jA;
          smax = b_A[temp_tmp];
          jA += jBcol;
          b_A[temp_tmp] = b_A[jA];
          b_A[jA] = smax;
        }
      }
      jA = (b - j) + 8;
      for (int k{b + 2}; k <= jA; k++) {
        b_A[k - 1] /= b_A[b];
      }
    }
    jA = b;
    for (int k{0}; k <= mmj; k++) {
      smax = b_A[(b + (k << 3)) + 8];
      if (smax != 0.0) {
        jBcol = (jA - j) + 16;
        for (int ijA{jA + 10}; ijA <= jBcol; ijA++) {
          b_A[ijA - 1] += b_A[((b + ijA) - jA) - 9] * -smax;
        }
      }
      jA += 8;
    }
    i = ipiv[j];
    if (i != j + 1) {
      for (int k{0}; k < 20; k++) {
        jA = k << 3;
        jBcol = j + jA;
        smax = B[jBcol];
        jA = (i + jA) - 1;
        B[jBcol] = B[jA];
        B[jA] = smax;
      }
    }
  }
  for (int k{0}; k < 20; k++) {
    jBcol = k << 3;
    for (int ijA{0}; ijA < 8; ijA++) {
      jA = ijA << 3;
      temp_tmp = ijA + jBcol;
      if (B[temp_tmp] != 0.0) {
        for (int j{ijA + 2}; j < 9; j++) {
          mmj = (j + jBcol) - 1;
          B[mmj] -= B[temp_tmp] * b_A[(j + jA) - 1];
        }
      }
    }
  }
  for (int k{0}; k < 20; k++) {
    jA = k << 3;
    for (int ijA{7}; ijA >= 0; ijA--) {
      jBcol = ijA << 3;
      temp_tmp = ijA + jA;
      smax = B[temp_tmp];
      if (smax != 0.0) {
        B[temp_tmp] = smax / b_A[ijA + jBcol];
        for (int j{0}; j < ijA; j++) {
          mmj = j + jA;
          B[mmj] -= B[temp_tmp] * b_A[j + jBcol];
        }
      }
    }
  }
}

} // namespace coder

// End of code generation (mldivide.cpp)
