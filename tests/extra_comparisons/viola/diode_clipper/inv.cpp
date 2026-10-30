//
// inv.cpp
//
// Code generation for function 'inv'
//

// Include files
#include "inv.h"
#include "coder_array.h"
#include <cmath>
#include <cstring>

// Function Definitions
namespace coder {
void inv(const array<double, 2U> &x, array<double, 2U> &y)
{
  array<double, 2U> b_x;
  array<int, 2U> ipiv;
  array<int, 2U> p;
  if ((x.size(0) == 0) || (x.size(1) == 0)) {
    int yk;
    y.set_size(x.size(0), x.size(1));
    yk = x.size(0) * x.size(1);
    for (int k{0}; k < yk; k++) {
      y[k] = x[k];
    }
  } else {
    double temp;
    int i;
    int jA;
    int ldap1;
    int n;
    int temp_tmp;
    int yk;
    n = x.size(0);
    y.set_size(x.size(0), x.size(1));
    yk = x.size(0) * x.size(1);
    for (int k{0}; k < yk; k++) {
      y[k] = 0.0;
    }
    b_x.set_size(x.size(0), x.size(1));
    for (int k{0}; k < yk; k++) {
      b_x[k] = x[k];
    }
    ipiv.set_size(1, x.size(0));
    ipiv[0] = 1;
    yk = 1;
    for (int k{2}; k <= n; k++) {
      yk++;
      ipiv[k - 1] = yk;
    }
    ldap1 = x.size(0) + 1;
    for (int j{0}; j <= n - 2; j++) {
      int b;
      int mmj_tmp;
      mmj_tmp = n - j;
      yk = mmj_tmp - 1;
      b = j * ldap1;
      if (mmj_tmp - 1 < 0) {
        jA = -1;
      } else {
        jA = 0;
        if (mmj_tmp - 1 > 0) {
          temp = std::abs(b_x[b]);
          for (int k{2}; k <= yk + 1; k++) {
            double s;
            s = std::abs(b_x[(b + k) - 1]);
            if (s > temp) {
              jA = k - 1;
              temp = s;
            }
          }
        }
      }
      if (b_x[b + jA] != 0.0) {
        if (jA != 0) {
          jA += j;
          ipiv[j] = jA + 1;
          for (int k{0}; k < n; k++) {
            yk = k * n;
            temp_tmp = j + yk;
            temp = b_x[temp_tmp];
            yk += jA;
            b_x[temp_tmp] = b_x[yk];
            b_x[yk] = temp;
          }
        }
        yk = b + mmj_tmp;
        for (int k{b + 2}; k <= yk; k++) {
          b_x[k - 1] = b_x[k - 1] / b_x[b];
        }
      }
      yk = b + n;
      jA = b + ldap1;
      for (int k{0}; k <= mmj_tmp - 2; k++) {
        temp = b_x[yk + k * n];
        if (temp != 0.0) {
          i = (mmj_tmp + jA) - 1;
          for (int ijA{jA + 1}; ijA <= i; ijA++) {
            b_x[ijA - 1] = b_x[ijA - 1] + b_x[(b + ijA) - jA] * -temp;
          }
        }
        jA += n;
      }
    }
    p.set_size(1, x.size(0));
    p[0] = 1;
    yk = 1;
    for (int k{2}; k <= n; k++) {
      yk++;
      p[k - 1] = yk;
    }
    for (int k{0}; k < n; k++) {
      yk = ipiv[k];
      if (yk > k + 1) {
        jA = p[yk - 1];
        p[yk - 1] = p[k];
        p[k] = jA;
      }
    }
    for (int j{0}; j < n; j++) {
      yk = p[j];
      yk = y.size(0) * (yk - 1);
      y[j + yk] = 1.0;
      for (int k{j + 1}; k <= n; k++) {
        jA = (k + yk) - 1;
        if (y[jA] != 0.0) {
          for (int ijA{k + 1}; ijA <= n; ijA++) {
            temp_tmp = (ijA + yk) - 1;
            y[temp_tmp] =
                y[temp_tmp] - y[jA] * b_x[(ijA + b_x.size(0) * (k - 1)) - 1];
          }
        }
      }
    }
    for (int k{0}; k < n; k++) {
      yk = n * k - 1;
      for (int ijA{n}; ijA >= 1; ijA--) {
        jA = n * (ijA - 1) - 1;
        temp_tmp = ijA + yk;
        temp = y[temp_tmp];
        if (temp != 0.0) {
          y[temp_tmp] = temp / b_x[ijA + jA];
          for (int j{0}; j <= ijA - 2; j++) {
            i = (j + yk) + 1;
            y[i] = y[i] - y[temp_tmp] * b_x[(j + jA) + 1];
          }
        }
      }
    }
  }
}

} // namespace coder

// End of code generation (inv.cpp)
