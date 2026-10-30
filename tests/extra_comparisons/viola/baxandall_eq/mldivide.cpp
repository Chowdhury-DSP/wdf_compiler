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
void mldivide(const double A[36], double B[84])
{
  double b_A[36];
  double smax;
  int jA;
  int jBcol;
  int kAcol;
  int mmj;
  signed char ipiv[6];
  std::copy(&A[0], &A[36], &b_A[0]);
  for (int k{0}; k < 6; k++) {
    ipiv[k] = static_cast<signed char>(k + 1);
  }
  for (int j{0}; j < 5; j++) {
    int b;
    signed char i;
    mmj = 4 - j;
    b = j * 7;
    jA = 7 - j;
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
        for (int k{0}; k < 6; k++) {
          jBcol = j + k * 6;
          smax = b_A[jBcol];
          kAcol = jA + k * 6;
          b_A[jBcol] = b_A[kAcol];
          b_A[kAcol] = smax;
        }
      }
      jA = (b - j) + 6;
      for (int k{b + 2}; k <= jA; k++) {
        b_A[k - 1] /= b_A[b];
      }
    }
    jA = b;
    for (int k{0}; k <= mmj; k++) {
      smax = b_A[(b + k * 6) + 6];
      if (smax != 0.0) {
        jBcol = (jA - j) + 12;
        for (int ijA{jA + 8}; ijA <= jBcol; ijA++) {
          b_A[ijA - 1] += b_A[((b + ijA) - jA) - 7] * -smax;
        }
      }
      jA += 6;
    }
    i = ipiv[j];
    if (i != j + 1) {
      for (int k{0}; k < 14; k++) {
        jA = j + 6 * k;
        smax = B[jA];
        jBcol = (i + 6 * k) - 1;
        B[jA] = B[jBcol];
        B[jBcol] = smax;
      }
    }
  }
  for (int k{0}; k < 14; k++) {
    jBcol = 6 * k;
    for (int ijA{0}; ijA < 6; ijA++) {
      kAcol = 6 * ijA;
      jA = ijA + jBcol;
      if (B[jA] != 0.0) {
        for (int j{ijA + 2}; j < 7; j++) {
          mmj = (j + jBcol) - 1;
          B[mmj] -= B[jA] * b_A[(j + kAcol) - 1];
        }
      }
    }
  }
  for (int k{0}; k < 14; k++) {
    jA = 6 * k;
    for (int ijA{5}; ijA >= 0; ijA--) {
      jBcol = 6 * ijA;
      kAcol = ijA + jA;
      smax = B[kAcol];
      if (smax != 0.0) {
        B[kAcol] = smax / b_A[ijA + jBcol];
        for (int j{0}; j < ijA; j++) {
          mmj = j + jA;
          B[mmj] -= B[kAcol] * b_A[j + jBcol];
        }
      }
    }
  }
}

} // namespace coder

// End of code generation (mldivide.cpp)
