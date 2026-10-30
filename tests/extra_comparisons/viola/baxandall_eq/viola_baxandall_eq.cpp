//
// viola_baxandall_eq.cpp
//
// Code generation for function 'viola_baxandall_eq'
//

// Include files
#include "viola_baxandall_eq.h"
#include "VIOLA_baxandall_eq_1.h"
#include "diag.h"
#include "eye.h"
#include "mldivide.h"
#include "strcmp.h"
#include "viola_baxandall_eq_data.h"
#include "viola_baxandall_eq_initialize.h"
#include "coder_array.h"
#include <algorithm>
#include <cstring>

// Variable Definitions
static VIOLA_baxandall_eq_1 plugin;

static bool plugin_not_empty;

// Function Definitions
void viola_baxandall_eq(const double x[64], int n, bool doReset, double y[64])
{
  static const double dv1[84]{-0.9999999999999987,
                              -0.0,
                              -0.0,
                              -0.0,
                              -0.0,
                              -0.0,
                              -0.0,
                              0.9999999999999998,
                              -0.0,
                              -0.0,
                              -0.0,
                              -0.0,
                              1.0,
                              -0.9999999999999996,
                              -0.0,
                              -0.0,
                              -0.0,
                              -0.0,
                              -0.0,
                              -0.0,
                              -0.0,
                              1.0000000000000007,
                              -0.0,
                              -0.0,
                              -0.0,
                              -1.0,
                              0.9999999999999998,
                              0.9999999999999998,
                              -0.0,
                              -0.0,
                              1.0000000000000002,
                              -0.0,
                              -1.0000000000000007,
                              -1.0000000000000007,
                              -1.0,
                              -0.0,
                              1.0000000000000007,
                              -0.0,
                              -1.0000000000000007,
                              -1.000000000000001,
                              -0.0,
                              -0.0,
                              1.0,
                              -0.9999999999999992,
                              -0.0,
                              -0.0,
                              -0.0,
                              -1.0,
                              1.0,
                              0.0,
                              0.0,
                              0.0,
                              0.0,
                              0.0,
                              0.0,
                              1.0,
                              0.0,
                              0.0,
                              0.0,
                              0.0,
                              0.0,
                              0.0,
                              1.0,
                              0.0,
                              0.0,
                              0.0,
                              0.0,
                              0.0,
                              0.0,
                              1.0,
                              0.0,
                              0.0,
                              0.0,
                              0.0,
                              0.0,
                              0.0,
                              1.0,
                              0.0,
                              0.0,
                              0.0,
                              0.0,
                              0.0,
                              0.0,
                              1.0};
  static const double dv2[70]{1.0E-9,  6.400000000000001E-9,
                              10000.0, 980.3921568627451,
                              10000.0, 2.2E-7,
                              1000.0,  50000.0,
                              1.0E-6,  8333.333333333334,
                              1.0E+6,  6.4E-8,
                              50000.0, 2.2000000000000002E-8,
                              0.0,     0.0,
                              0.0,     0.0,
                              0.0,     0.0,
                              0.0,     0.0,
                              0.0,     0.0,
                              0.0,     0.0,
                              0.0,     0.0,
                              0.0,     0.0,
                              0.0,     0.0,
                              0.0,     0.0,
                              0.0,     0.0,
                              0.0,     0.0,
                              0.0,     0.0,
                              0.0,     0.0,
                              0.0,     0.0,
                              0.0,     0.0,
                              0.0,     0.0,
                              0.0,     0.0,
                              0.0,     0.0,
                              0.0,     0.0,
                              0.0,     0.0,
                              0.0,     0.0,
                              0.0,     0.0,
                              0.0,     0.0,
                              0.0,     0.0,
                              0.0,     0.0,
                              0.0,     0.0,
                              0.0,     0.0};
  static const signed char iv[8]{3, 4, 5, 7, 8, 10, 11, 13};
  static const signed char iv1[5]{2, 6, 9, 12, 14};
  coder::array<double, 2U> r;
  coder::array<double, 2U> r1;
  double dv4[84];
  double y_tmp[84];
  double in_data[64];
  double dv5[36];
  double dv3[25];
  double dv[8];
  double b[5];
  double d;
  int b_loop_ub;
  int loop_ub;
  if (!isInitialized_viola_baxandall_eq) {
    viola_baxandall_eq_initialize();
  }
  //  Benchmark entry point written by run_viola_codegen.m for VIOLA's
  //  VIOLA_baxandall_eq_1 plug-in.
  //    doReset = true:  re-initialise the plug-in exactly like its reset()
  //    (sample rate
  //                     fixed at 48000 Hz) and clear its state; x and n are
  //                     ignored.
  //    doReset = false: y(1:n) = process(plugin, x(1:n)), i.e. one host block
  //    of n samples.
  if (!plugin_not_empty) {
    plugin.x_1 = 0.5;
    plugin.Volume = 1.0;
    plugin.Enable[0] = 'O';
    plugin.Enable[1] = 'N';
    plugin.pos_Vin = 1.0;
    for (int i{0}; i < 8; i++) {
      plugin.pos_R[i] = iv[i];
    }
    for (int i{0}; i < 5; i++) {
      plugin.pos_C[i] = iv1[i];
    }
    plugin.R_tol = 1.0E-6;
    plugin.n = 14.0;
    std::copy(&dv1[0], &dv1[84], &plugin.B[0]);
    plugin.Rp = 100000.0;
    std::copy(&dv2[0], &dv2[70], &plugin.P[0]);
    plugin.O[0] = 11.0;
    plugin.O[1] = 1.0;
    plugin.Z.set_size(14, 14);
    loop_ub = static_cast<int>(plugin.n) * static_cast<int>(plugin.n);
    if (loop_ub - 1 >= 0) {
      std::memset(&plugin.Z[0], 0,
                  static_cast<unsigned int>(loop_ub) * sizeof(double));
    }
    plugin.matlabCodegenIsDeleted = false;
    plugin_not_empty = true;
    plugin.f_s = 48000.0;
    plugin.Z[(static_cast<int>(plugin.pos_Vin) +
              plugin.Z.size(0) * (static_cast<int>(plugin.pos_Vin) - 1)) -
             1] = plugin.P[static_cast<int>(plugin.pos_Vin) - 1];
    for (int i{0}; i < 8; i++) {
      dv[i] = plugin.P[static_cast<int>(plugin.pos_R[i]) - 1];
    }
    coder::diag(dv, in_data);
    for (int i{0}; i < 8; i++) {
      for (int i1{0}; i1 < 8; i1++) {
        plugin.Z[(static_cast<int>(plugin.pos_R[i1]) +
                  plugin.Z.size(0) * (static_cast<int>(plugin.pos_R[i]) - 1)) -
                 1] = in_data[i1 + (i << 3)];
      }
    }
    for (int i{0}; i < 5; i++) {
      b[i] = 1.0 /
             (2.0 * plugin.P[static_cast<int>(plugin.pos_C[i]) - 1] * 48000.0);
    }
    coder::b_diag(b, dv3);
    for (int i{0}; i < 5; i++) {
      for (int i1{0}; i1 < 5; i1++) {
        plugin.Z[(static_cast<int>(plugin.pos_C[i1]) +
                  plugin.Z.size(0) * (static_cast<int>(plugin.pos_C[i]) - 1)) -
                 1] = dv3[i1 + 5 * i];
      }
    }
    plugin.Z[plugin.Z.size(0) * 7 + 7] = plugin.x_1 * plugin.Rp + plugin.R_tol;
    plugin.Z[plugin.Z.size(0) * 12 + 12] =
        (1.0 - plugin.x_1) * plugin.Rp + plugin.R_tol;
    plugin.a.set_size(static_cast<int>(plugin.n));
    loop_ub = static_cast<int>(plugin.n);
    if (loop_ub - 1 >= 0) {
      std::memset(&plugin.a[0], 0,
                  static_cast<unsigned int>(loop_ub) * sizeof(double));
    }
    plugin.b.set_size(static_cast<int>(plugin.n));
    loop_ub = static_cast<int>(plugin.n);
    if (loop_ub - 1 >= 0) {
      std::memset(&plugin.b[0], 0,
                  static_cast<unsigned int>(loop_ub) * sizeof(double));
    }
    for (int i{0}; i < 6; i++) {
      for (int i1{0}; i1 < 14; i1++) {
        y_tmp[i1 + 14 * i] = plugin.B[i + 6 * i1];
      }
    }
    coder::eye(plugin.n, plugin.S);
    std::copy(&plugin.B[0], &plugin.B[84], &dv4[0]);
    b_loop_ub = plugin.Z.size(1);
    r.set_size(6, plugin.Z.size(1));
    for (int ii{0}; ii < b_loop_ub; ii++) {
      for (int i{0}; i < 6; i++) {
        r[i + 6 * ii] = 0.0;
      }
      for (int i{0}; i < 14; i++) {
        d = plugin.Z[i + plugin.Z.size(0) * ii];
        for (int i1{0}; i1 < 6; i1++) {
          loop_ub = i1 + 6 * ii;
          r[loop_ub] = r[loop_ub] + plugin.B[i1 + 6 * i] * d;
        }
      }
    }
    std::memset(&dv5[0], 0, 36U * sizeof(double));
    for (int i{0}; i < 6; i++) {
      for (int i1{0}; i1 < 14; i1++) {
        d = y_tmp[i1 + 14 * i];
        for (int ii{0}; ii < 6; ii++) {
          loop_ub = ii + 6 * i;
          dv5[loop_ub] += r[ii + 6 * i1] * d;
        }
      }
    }
    coder::mldivide(dv5, dv4);
    if ((plugin.S.size(0) == plugin.Z.size(0)) && (plugin.S.size(1) == 14)) {
      b_loop_ub = plugin.Z.size(0);
      r1.set_size(plugin.Z.size(0), 6);
      for (int ii{0}; ii < 6; ii++) {
        for (int i{0}; i < b_loop_ub; i++) {
          r1[i + r1.size(0) * ii] = 0.0;
        }
        for (int i{0}; i < 14; i++) {
          d = y_tmp[i + 14 * ii];
          for (int i1{0}; i1 < b_loop_ub; i1++) {
            loop_ub = i1 + r1.size(0) * ii;
            r1[loop_ub] =
                r1[loop_ub] + 2.0 * plugin.Z[i1 + plugin.Z.size(0) * i] * d;
          }
        }
      }
      plugin.S.set_size(plugin.Z.size(0), 14);
      for (int i{0}; i < b_loop_ub; i++) {
        for (int i1{0}; i1 < 14; i1++) {
          d = 0.0;
          for (int ii{0}; ii < 6; ii++) {
            d += r1[i + r1.size(0) * ii] * dv4[ii + 6 * i1];
          }
          plugin.S[i + plugin.S.size(0) * i1] =
              plugin.S[i + plugin.S.size(0) * i1] - d;
        }
      }
    } else {
      binary_expand_op(plugin, y_tmp, dv4);
    }
  }
  std::memset(&y[0], 0, 64U * sizeof(double));
  if (doReset) {
    plugin.f_s = 48000.0;
    plugin.Z[(static_cast<int>(plugin.pos_Vin) +
              plugin.Z.size(0) * (static_cast<int>(plugin.pos_Vin) - 1)) -
             1] = plugin.P[static_cast<int>(plugin.pos_Vin) - 1];
    for (int i{0}; i < 8; i++) {
      dv[i] = plugin.P[static_cast<int>(plugin.pos_R[i]) - 1];
    }
    coder::diag(dv, in_data);
    for (int i{0}; i < 8; i++) {
      for (int i1{0}; i1 < 8; i1++) {
        plugin.Z[(static_cast<int>(plugin.pos_R[i1]) +
                  plugin.Z.size(0) * (static_cast<int>(plugin.pos_R[i]) - 1)) -
                 1] = in_data[i1 + (i << 3)];
      }
    }
    for (int i{0}; i < 5; i++) {
      b[i] = 1.0 /
             (2.0 * plugin.P[static_cast<int>(plugin.pos_C[i]) - 1] * 48000.0);
    }
    coder::b_diag(b, dv3);
    for (int i{0}; i < 5; i++) {
      for (int i1{0}; i1 < 5; i1++) {
        plugin.Z[(static_cast<int>(plugin.pos_C[i1]) +
                  plugin.Z.size(0) * (static_cast<int>(plugin.pos_C[i]) - 1)) -
                 1] = dv3[i1 + 5 * i];
      }
    }
    plugin.Z[plugin.Z.size(0) * 7 + 7] = plugin.x_1 * plugin.Rp + plugin.R_tol;
    plugin.Z[plugin.Z.size(0) * 12 + 12] =
        (1.0 - plugin.x_1) * plugin.Rp + plugin.R_tol;
    plugin.a.set_size(static_cast<int>(plugin.n));
    loop_ub = static_cast<int>(plugin.n);
    if (loop_ub - 1 >= 0) {
      std::memset(&plugin.a[0], 0,
                  static_cast<unsigned int>(loop_ub) * sizeof(double));
    }
    plugin.b.set_size(static_cast<int>(plugin.n));
    loop_ub = static_cast<int>(plugin.n);
    if (loop_ub - 1 >= 0) {
      std::memset(&plugin.b[0], 0,
                  static_cast<unsigned int>(loop_ub) * sizeof(double));
    }
    for (int i{0}; i < 6; i++) {
      for (int i1{0}; i1 < 14; i1++) {
        y_tmp[i1 + 14 * i] = plugin.B[i + 6 * i1];
      }
    }
    coder::eye(plugin.n, plugin.S);
    std::copy(&plugin.B[0], &plugin.B[84], &dv4[0]);
    b_loop_ub = plugin.Z.size(1);
    r.set_size(6, plugin.Z.size(1));
    for (int ii{0}; ii < b_loop_ub; ii++) {
      for (int i{0}; i < 6; i++) {
        r[i + 6 * ii] = 0.0;
      }
      for (int i{0}; i < 14; i++) {
        d = plugin.Z[i + plugin.Z.size(0) * ii];
        for (int i1{0}; i1 < 6; i1++) {
          loop_ub = i1 + 6 * ii;
          r[loop_ub] = r[loop_ub] + plugin.B[i1 + 6 * i] * d;
        }
      }
    }
    std::memset(&dv5[0], 0, 36U * sizeof(double));
    for (int i{0}; i < 6; i++) {
      for (int i1{0}; i1 < 14; i1++) {
        d = y_tmp[i1 + 14 * i];
        for (int ii{0}; ii < 6; ii++) {
          loop_ub = ii + 6 * i;
          dv5[loop_ub] += r[ii + 6 * i1] * d;
        }
      }
    }
    coder::mldivide(dv5, dv4);
    if ((plugin.S.size(0) == plugin.Z.size(0)) && (plugin.S.size(1) == 14)) {
      b_loop_ub = plugin.Z.size(0);
      r1.set_size(plugin.Z.size(0), 6);
      for (int ii{0}; ii < 6; ii++) {
        for (int i{0}; i < b_loop_ub; i++) {
          r1[i + r1.size(0) * ii] = 0.0;
        }
        for (int i{0}; i < 14; i++) {
          d = y_tmp[i + 14 * ii];
          for (int i1{0}; i1 < b_loop_ub; i1++) {
            loop_ub = i1 + r1.size(0) * ii;
            r1[loop_ub] =
                r1[loop_ub] + 2.0 * plugin.Z[i1 + plugin.Z.size(0) * i] * d;
          }
        }
      }
      plugin.S.set_size(plugin.Z.size(0), 14);
      for (int i{0}; i < b_loop_ub; i++) {
        for (int i1{0}; i1 < 14; i1++) {
          d = 0.0;
          for (int ii{0}; ii < 6; ii++) {
            d += r1[i + r1.size(0) * ii] * dv4[ii + 6 * i1];
          }
          plugin.S[i + plugin.S.size(0) * i1] =
              plugin.S[i + plugin.S.size(0) * i1] - d;
        }
      }
    } else {
      binary_expand_op(plugin, y_tmp, dv4);
    }
  } else {
    int m;
    if (n >= 0) {
      m = n;
    } else {
      m = 0;
    }
    if (m > 64) {
      m = 64;
    }
    if (m - 1 >= 0) {
      std::copy(&x[0], &x[m], &in_data[0]);
    }
    if (coder::internal::b_strcmp(plugin.Enable)) {
      for (int ii{0}; ii < m; ii++) {
        plugin.b[static_cast<int>(plugin.pos_Vin) - 1] = x[ii];
        for (int i{0}; i < 8; i++) {
          plugin.b[static_cast<int>(plugin.pos_R[i]) - 1] = 0.0;
        }
        for (int i{0}; i < 5; i++) {
          loop_ub = static_cast<int>(plugin.pos_C[i]) - 1;
          plugin.b[loop_ub] = plugin.a[loop_ub];
        }
        loop_ub = plugin.S.size(0);
        b_loop_ub = plugin.S.size(1);
        plugin.a.set_size(plugin.S.size(0));
        if (loop_ub - 1 >= 0) {
          std::memset(&plugin.a[0], 0,
                      static_cast<unsigned int>(loop_ub) * sizeof(double));
        }
        for (int i{0}; i < b_loop_ub; i++) {
          for (int i1{0}; i1 < loop_ub; i1++) {
            plugin.a[i1] = plugin.a[i1] +
                           plugin.S[i1 + plugin.S.size(0) * i] * plugin.b[i];
          }
        }
        in_data[ii] = plugin.Volume *
                      ((plugin.a[static_cast<int>(plugin.O[0]) - 1] +
                        plugin.b[static_cast<int>(plugin.O[0]) - 1]) *
                       plugin.O[1]) /
                      2.0;
      }
    }
    if (m - 1 >= 0) {
      std::copy(&in_data[0], &in_data[m], &y[0]);
    }
  }
}

void viola_baxandall_eq_delete()
{
  if (!plugin.matlabCodegenIsDeleted) {
    plugin.matlabCodegenIsDeleted = true;
  }
}

void viola_baxandall_eq_emx_free()
{
}

void viola_baxandall_eq_emx_init()
{
}

void viola_baxandall_eq_init()
{
  plugin_not_empty = false;
}

void viola_baxandall_eq_new()
{
  plugin.matlabCodegenIsDeleted = true;
}

// End of code generation (viola_baxandall_eq.cpp)
