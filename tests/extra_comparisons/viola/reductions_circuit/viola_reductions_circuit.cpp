//
// viola_reductions_circuit.cpp
//
// Code generation for function 'viola_reductions_circuit'
//

// Include files
#include "viola_reductions_circuit.h"
#include "VIOLA_reductions_circuit_1.h"
#include "diag.h"
#include "eye.h"
#include "mldivide.h"
#include "strcmp.h"
#include "viola_reductions_circuit_data.h"
#include "viola_reductions_circuit_initialize.h"
#include "coder_array.h"
#include <algorithm>
#include <cstring>

// Variable Definitions
static VIOLA_reductions_circuit_1 plugin;

static bool plugin_not_empty;

// Function Definitions
void viola_reductions_circuit(const double x[64], int n, bool doReset,
                              double y[64])
{
  static const double dv2[160]{-0.0,
                               -0.0,
                               -0.0,
                               -1.0000000000000002,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               1.000000000000001,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               1.0000000000000013,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               -1.0,
                               -1.0,
                               -1.0,
                               1.0000000000000007,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               1.0000000000000004,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               0.9999999999999997,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               1.0000000000000027,
                               -1.0,
                               -0.9999999999999976,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               0.9999999999999989,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               1.0000000000000004,
                               -0.0,
                               -0.0,
                               -0.0,
                               0.9999999999999972,
                               -0.0,
                               -0.0,
                               -0.9999999999999993,
                               -1.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               1.0000000000000004,
                               -0.0,
                               -0.0,
                               -0.0,
                               0.9999999999999989,
                               -0.0,
                               -0.0,
                               -0.0,
                               -0.0,
                               1.0,
                               0.0,
                               0.0,
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
                               0.0,
                               0.0,
                               1.0,
                               0.0,
                               0.0,
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
                               0.0,
                               0.0,
                               1.0,
                               0.0,
                               0.0,
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
                               0.0,
                               0.0,
                               1.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               1.0};
  static const double dv3[100]{1.0E-9,
                               10000.0,
                               5000.0,
                               100000.0,
                               10000.0,
                               1000.0,
                               3000.0,
                               4000.0,
                               1.0E-6,
                               1.0E-8,
                               4.9999999999999996E-6,
                               100000.0,
                               1.0000000000000001E-7,
                               10000.0,
                               9.999999999999999E-6,
                               4.0E-6,
                               10000.0,
                               5000.0,
                               3.0E-6,
                               4.0E-9,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0.0};
  static const signed char iv[11]{2, 3, 4, 5, 6, 7, 8, 12, 14, 17, 18};
  static const signed char iv1[8]{9, 10, 11, 13, 15, 16, 19, 20};
  coder::array<double, 2U> r;
  coder::array<double, 2U> r1;
  double dv4[160];
  double y_tmp[160];
  double dv1[121];
  double in_data[64];
  double dv[11];
  double b[8];
  double d;
  int b_loop_ub;
  int loop_ub;
  if (!isInitialized_viola_reductions_circuit) {
    viola_reductions_circuit_initialize();
  }
  //  Benchmark entry point written by run_viola_codegen.m for VIOLA's
  //  VIOLA_reductions_circuit_1 plug-in.
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
    for (int i{0}; i < 11; i++) {
      plugin.pos_R[i] = iv[i];
    }
    for (int i{0}; i < 8; i++) {
      plugin.pos_C[i] = iv1[i];
    }
    plugin.R_tol = 1.0E-6;
    plugin.n = 20.0;
    std::copy(&dv2[0], &dv2[160], &plugin.B[0]);
    plugin.Rp = 10000.0;
    std::copy(&dv3[0], &dv3[100], &plugin.P[0]);
    plugin.O[0] = 12.0;
    plugin.O[1] = 1.0;
    plugin.Z.set_size(20, 20);
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
    for (int i{0}; i < 11; i++) {
      dv[i] = plugin.P[static_cast<int>(plugin.pos_R[i]) - 1];
    }
    coder::diag(dv, dv1);
    for (int i{0}; i < 11; i++) {
      for (int i1{0}; i1 < 11; i1++) {
        plugin.Z[(static_cast<int>(plugin.pos_R[i1]) +
                  plugin.Z.size(0) * (static_cast<int>(plugin.pos_R[i]) - 1)) -
                 1] = dv1[i1 + 11 * i];
      }
    }
    for (int i{0}; i < 8; i++) {
      b[i] = 1.0 /
             (2.0 * plugin.P[static_cast<int>(plugin.pos_C[i]) - 1] * 48000.0);
    }
    coder::b_diag(b, in_data);
    for (int i{0}; i < 8; i++) {
      for (int i1{0}; i1 < 8; i1++) {
        plugin.Z[(static_cast<int>(plugin.pos_C[i1]) +
                  plugin.Z.size(0) * (static_cast<int>(plugin.pos_C[i]) - 1)) -
                 1] = in_data[i1 + (i << 3)];
      }
    }
    plugin.Z[plugin.Z.size(0) * 17 + 17] =
        plugin.x_1 * plugin.Rp + plugin.R_tol;
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
    for (int i{0}; i < 8; i++) {
      for (int i1{0}; i1 < 20; i1++) {
        y_tmp[i1 + 20 * i] = plugin.B[i + (i1 << 3)];
      }
    }
    coder::eye(plugin.n, plugin.S);
    std::copy(&plugin.B[0], &plugin.B[160], &dv4[0]);
    loop_ub = plugin.Z.size(1);
    r.set_size(8, plugin.Z.size(1));
    for (int i{0}; i < loop_ub; i++) {
      std::memset(&r[i * 8], 0, 8U * sizeof(double));
      for (int i1{0}; i1 < 20; i1++) {
        d = plugin.Z[i1 + plugin.Z.size(0) * i];
        for (int ii{0}; ii < 8; ii++) {
          b_loop_ub = ii + 8 * i;
          r[b_loop_ub] = r[b_loop_ub] + plugin.B[ii + (i1 << 3)] * d;
        }
      }
    }
    std::memset(&in_data[0], 0, sizeof(double) << 6);
    for (int i{0}; i < 8; i++) {
      for (int i1{0}; i1 < 20; i1++) {
        d = y_tmp[i1 + 20 * i];
        for (int ii{0}; ii < 8; ii++) {
          loop_ub = ii + (i << 3);
          in_data[loop_ub] += r[ii + (i1 << 3)] * d;
        }
      }
    }
    coder::mldivide(in_data, dv4);
    if ((plugin.S.size(0) == plugin.Z.size(0)) && (plugin.S.size(1) == 20)) {
      b_loop_ub = plugin.Z.size(0);
      r1.set_size(plugin.Z.size(0), 8);
      for (int ii{0}; ii < 8; ii++) {
        for (int i{0}; i < b_loop_ub; i++) {
          r1[i + r1.size(0) * ii] = 0.0;
        }
        for (int i{0}; i < 20; i++) {
          d = y_tmp[i + 20 * ii];
          for (int i1{0}; i1 < b_loop_ub; i1++) {
            loop_ub = i1 + r1.size(0) * ii;
            r1[loop_ub] =
                r1[loop_ub] + 2.0 * plugin.Z[i1 + plugin.Z.size(0) * i] * d;
          }
        }
      }
      plugin.S.set_size(plugin.Z.size(0), 20);
      for (int i{0}; i < b_loop_ub; i++) {
        for (int i1{0}; i1 < 20; i1++) {
          d = 0.0;
          for (int ii{0}; ii < 8; ii++) {
            d += r1[i + r1.size(0) * ii] * dv4[ii + (i1 << 3)];
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
    for (int i{0}; i < 11; i++) {
      dv[i] = plugin.P[static_cast<int>(plugin.pos_R[i]) - 1];
    }
    coder::diag(dv, dv1);
    for (int i{0}; i < 11; i++) {
      for (int i1{0}; i1 < 11; i1++) {
        plugin.Z[(static_cast<int>(plugin.pos_R[i1]) +
                  plugin.Z.size(0) * (static_cast<int>(plugin.pos_R[i]) - 1)) -
                 1] = dv1[i1 + 11 * i];
      }
    }
    for (int i{0}; i < 8; i++) {
      b[i] = 1.0 /
             (2.0 * plugin.P[static_cast<int>(plugin.pos_C[i]) - 1] * 48000.0);
    }
    coder::b_diag(b, in_data);
    for (int i{0}; i < 8; i++) {
      for (int i1{0}; i1 < 8; i1++) {
        plugin.Z[(static_cast<int>(plugin.pos_C[i1]) +
                  plugin.Z.size(0) * (static_cast<int>(plugin.pos_C[i]) - 1)) -
                 1] = in_data[i1 + (i << 3)];
      }
    }
    plugin.Z[plugin.Z.size(0) * 17 + 17] =
        plugin.x_1 * plugin.Rp + plugin.R_tol;
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
    for (int i{0}; i < 8; i++) {
      for (int i1{0}; i1 < 20; i1++) {
        y_tmp[i1 + 20 * i] = plugin.B[i + (i1 << 3)];
      }
    }
    coder::eye(plugin.n, plugin.S);
    std::copy(&plugin.B[0], &plugin.B[160], &dv4[0]);
    loop_ub = plugin.Z.size(1);
    r.set_size(8, plugin.Z.size(1));
    for (int i{0}; i < loop_ub; i++) {
      std::memset(&r[i * 8], 0, 8U * sizeof(double));
      for (int i1{0}; i1 < 20; i1++) {
        d = plugin.Z[i1 + plugin.Z.size(0) * i];
        for (int ii{0}; ii < 8; ii++) {
          b_loop_ub = ii + 8 * i;
          r[b_loop_ub] = r[b_loop_ub] + plugin.B[ii + (i1 << 3)] * d;
        }
      }
    }
    std::memset(&in_data[0], 0, sizeof(double) << 6);
    for (int i{0}; i < 8; i++) {
      for (int i1{0}; i1 < 20; i1++) {
        d = y_tmp[i1 + 20 * i];
        for (int ii{0}; ii < 8; ii++) {
          loop_ub = ii + (i << 3);
          in_data[loop_ub] += r[ii + (i1 << 3)] * d;
        }
      }
    }
    coder::mldivide(in_data, dv4);
    if ((plugin.S.size(0) == plugin.Z.size(0)) && (plugin.S.size(1) == 20)) {
      b_loop_ub = plugin.Z.size(0);
      r1.set_size(plugin.Z.size(0), 8);
      for (int ii{0}; ii < 8; ii++) {
        for (int i{0}; i < b_loop_ub; i++) {
          r1[i + r1.size(0) * ii] = 0.0;
        }
        for (int i{0}; i < 20; i++) {
          d = y_tmp[i + 20 * ii];
          for (int i1{0}; i1 < b_loop_ub; i1++) {
            loop_ub = i1 + r1.size(0) * ii;
            r1[loop_ub] =
                r1[loop_ub] + 2.0 * plugin.Z[i1 + plugin.Z.size(0) * i] * d;
          }
        }
      }
      plugin.S.set_size(plugin.Z.size(0), 20);
      for (int i{0}; i < b_loop_ub; i++) {
        for (int i1{0}; i1 < 20; i1++) {
          d = 0.0;
          for (int ii{0}; ii < 8; ii++) {
            d += r1[i + r1.size(0) * ii] * dv4[ii + (i1 << 3)];
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
        for (int i{0}; i < 11; i++) {
          plugin.b[static_cast<int>(plugin.pos_R[i]) - 1] = 0.0;
        }
        for (int i{0}; i < 8; i++) {
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

void viola_reductions_circuit_delete()
{
  if (!plugin.matlabCodegenIsDeleted) {
    plugin.matlabCodegenIsDeleted = true;
  }
}

void viola_reductions_circuit_emx_free()
{
}

void viola_reductions_circuit_emx_init()
{
}

void viola_reductions_circuit_init()
{
  plugin_not_empty = false;
}

void viola_reductions_circuit_new()
{
  plugin.matlabCodegenIsDeleted = true;
}

// End of code generation (viola_reductions_circuit.cpp)
