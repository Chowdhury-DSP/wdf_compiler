//
// viola_preamp_eq.cpp
//
// Code generation for function 'viola_preamp_eq'
//

// Include files
#include "viola_preamp_eq.h"
#include "VIOLA_preamp_eq_1.h"
#include "diag.h"
#include "eye.h"
#include "mldivide.h"
#include "strcmp.h"
#include "viola_preamp_eq_data.h"
#include "viola_preamp_eq_initialize.h"
#include "coder_array.h"
#include <algorithm>
#include <cstring>

// Variable Definitions
static VIOLA_preamp_eq_1 plugin;

static bool plugin_not_empty;

// Function Definitions
void viola_preamp_eq(const double x[64], int n, bool doReset, double y[64])
{
  static const double dv2[45]{4.700000000000001E-9,
                              10000.0,
                              4674.0,
                              2.7E-9,
                              15000.0,
                              1.0E-9,
                              80000.0,
                              3.2999999999999997E-6,
                              1.5E-7,
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
  static const double dv1[36]{-0.9999999999999998,
                              -0.0,
                              -0.0,
                              -0.0,
                              -1.0000000000000004,
                              -1.0000000000000002,
                              -0.0,
                              -0.0,
                              -1.0000000000000004,
                              -0.0,
                              -1.0000000000000004,
                              -0.0,
                              -0.0,
                              0.9999999999999997,
                              -0.0,
                              -0.0,
                              -1.0,
                              -0.0,
                              -1.0,
                              -0.9999999999999997,
                              1.0,
                              0.0,
                              0.0,
                              0.0,
                              0.0,
                              1.0,
                              0.0,
                              0.0,
                              0.0,
                              0.0,
                              1.0,
                              0.0,
                              0.0,
                              0.0,
                              0.0,
                              1.0};
  coder::array<double, 2U> r;
  coder::array<double, 2U> r1;
  double in_data[64];
  double dv3[36];
  double y_tmp[36];
  double dv[16];
  double b[4];
  double d;
  int b_loop_ub;
  int i2;
  int i3;
  int loop_ub;
  int m;
  if (!isInitialized_viola_preamp_eq) {
    viola_preamp_eq_initialize();
  }
  //  Benchmark entry point written by run_viola_codegen.m for VIOLA's
  //  VIOLA_preamp_eq_1 plug-in.
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
    plugin.pos_Vin = 6.0;
    plugin.pos_R[0] = 2.0;
    plugin.pos_C[0] = 1.0;
    plugin.pos_R[1] = 3.0;
    plugin.pos_C[1] = 4.0;
    plugin.pos_R[2] = 5.0;
    plugin.pos_C[2] = 8.0;
    plugin.pos_R[3] = 7.0;
    plugin.pos_C[3] = 9.0;
    plugin.R_tol = 1.0E-6;
    plugin.n = 9.0;
    std::copy(&dv1[0], &dv1[36], &plugin.B[0]);
    plugin.Rp = 20000.0;
    std::copy(&dv2[0], &dv2[45], &plugin.P[0]);
    plugin.O[0] = 8.0;
    plugin.O[1] = 1.0;
    plugin.Z.set_size(9, 9);
    m = static_cast<int>(plugin.n) * static_cast<int>(plugin.n);
    if (m - 1 >= 0) {
      std::memset(&plugin.Z[0], 0,
                  static_cast<unsigned int>(m) * sizeof(double));
    }
    plugin.matlabCodegenIsDeleted = false;
    plugin_not_empty = true;
    plugin.f_s = 48000.0;
    plugin.Z[(static_cast<int>(plugin.pos_Vin) +
              plugin.Z.size(0) * (static_cast<int>(plugin.pos_Vin) - 1)) -
             1] = plugin.P[static_cast<int>(plugin.pos_Vin) - 1];
    b[0] = plugin.P[static_cast<int>(plugin.pos_R[0]) - 1];
    b[1] = plugin.P[static_cast<int>(plugin.pos_R[1]) - 1];
    b[2] = plugin.P[static_cast<int>(plugin.pos_R[2]) - 1];
    b[3] = plugin.P[static_cast<int>(plugin.pos_R[3]) - 1];
    coder::diag(b, dv);
    for (int ii{0}; ii < 4; ii++) {
      m = ii << 2;
      plugin.Z[(static_cast<int>(plugin.pos_R[0]) +
                plugin.Z.size(0) * (static_cast<int>(plugin.pos_R[ii]) - 1)) -
               1] = dv[m];
      plugin.Z[(static_cast<int>(plugin.pos_R[1]) +
                plugin.Z.size(0) * (static_cast<int>(plugin.pos_R[ii]) - 1)) -
               1] = dv[m + 1];
      plugin.Z[(static_cast<int>(plugin.pos_R[2]) +
                plugin.Z.size(0) * (static_cast<int>(plugin.pos_R[ii]) - 1)) -
               1] = dv[m + 2];
      plugin.Z[(static_cast<int>(plugin.pos_R[3]) +
                plugin.Z.size(0) * (static_cast<int>(plugin.pos_R[ii]) - 1)) -
               1] = dv[m + 3];
      b[ii] = 1.0 / (2.0 * plugin.P[static_cast<int>(plugin.pos_C[ii]) - 1] *
                     48000.0);
    }
    coder::diag(b, dv);
    for (int ii{0}; ii < 4; ii++) {
      m = ii << 2;
      plugin.Z[(static_cast<int>(plugin.pos_C[0]) +
                plugin.Z.size(0) * (static_cast<int>(plugin.pos_C[ii]) - 1)) -
               1] = dv[m];
      plugin.Z[(static_cast<int>(plugin.pos_C[1]) +
                plugin.Z.size(0) * (static_cast<int>(plugin.pos_C[ii]) - 1)) -
               1] = dv[m + 1];
      plugin.Z[(static_cast<int>(plugin.pos_C[2]) +
                plugin.Z.size(0) * (static_cast<int>(plugin.pos_C[ii]) - 1)) -
               1] = dv[m + 2];
      plugin.Z[(static_cast<int>(plugin.pos_C[3]) +
                plugin.Z.size(0) * (static_cast<int>(plugin.pos_C[ii]) - 1)) -
               1] = dv[m + 3];
    }
    plugin.Z[plugin.Z.size(0) + 1] = plugin.x_1 * plugin.Rp + plugin.R_tol;
    plugin.a.set_size(static_cast<int>(plugin.n));
    m = static_cast<int>(plugin.n);
    if (m - 1 >= 0) {
      std::memset(&plugin.a[0], 0,
                  static_cast<unsigned int>(m) * sizeof(double));
    }
    plugin.b.set_size(static_cast<int>(plugin.n));
    m = static_cast<int>(plugin.n);
    if (m - 1 >= 0) {
      std::memset(&plugin.b[0], 0,
                  static_cast<unsigned int>(m) * sizeof(double));
    }
    for (int ii{0}; ii < 9; ii++) {
      m = ii << 2;
      y_tmp[ii] = plugin.B[m];
      y_tmp[ii + 9] = plugin.B[m + 1];
      y_tmp[ii + 18] = plugin.B[m + 2];
      y_tmp[ii + 27] = plugin.B[m + 3];
    }
    coder::eye(plugin.n, plugin.S);
    std::copy(&plugin.B[0], &plugin.B[36], &dv3[0]);
    loop_ub = plugin.Z.size(1);
    r.set_size(4, plugin.Z.size(1));
    for (int i{0}; i < loop_ub; i++) {
      r[4 * i] = 0.0;
      m = 4 * i + 1;
      r[m] = 0.0;
      b_loop_ub = 4 * i + 2;
      r[b_loop_ub] = 0.0;
      i2 = 4 * i + 3;
      r[i2] = 0.0;
      for (int i1{0}; i1 < 9; i1++) {
        d = plugin.Z[i1 + plugin.Z.size(0) * i];
        i3 = i1 << 2;
        r[4 * i] = r[4 * i] + plugin.B[i3] * d;
        r[m] = r[m] + plugin.B[i3 + 1] * d;
        r[b_loop_ub] = r[b_loop_ub] + plugin.B[i3 + 2] * d;
        r[i2] = r[i2] + plugin.B[i3 + 3] * d;
      }
    }
    std::memset(&dv[0], 0, sizeof(double) << 4);
    for (int ii{0}; ii < 4; ii++) {
      m = ii << 2;
      for (int i{0}; i < 9; i++) {
        d = y_tmp[i + 9 * ii];
        loop_ub = i << 2;
        dv[m] += r[loop_ub] * d;
        dv[m + 1] += r[loop_ub + 1] * d;
        dv[m + 2] += r[loop_ub + 2] * d;
        dv[m + 3] += r[loop_ub + 3] * d;
      }
    }
    coder::mldivide(dv, dv3);
    if ((plugin.S.size(0) == plugin.Z.size(0)) && (plugin.S.size(1) == 9)) {
      loop_ub = plugin.Z.size(0);
      r1.set_size(plugin.Z.size(0), 4);
      for (int i1{0}; i1 < 4; i1++) {
        for (int ii{0}; ii < loop_ub; ii++) {
          r1[ii + r1.size(0) * i1] = 0.0;
        }
        for (int ii{0}; ii < 9; ii++) {
          d = y_tmp[ii + 9 * i1];
          for (int i{0}; i < loop_ub; i++) {
            m = i + r1.size(0) * i1;
            r1[m] = r1[m] + 2.0 * plugin.Z[i + plugin.Z.size(0) * ii] * d;
          }
        }
      }
      plugin.S.set_size(plugin.Z.size(0), 9);
      for (int ii{0}; ii < loop_ub; ii++) {
        for (int i{0}; i < 9; i++) {
          m = i << 2;
          plugin.S[ii + plugin.S.size(0) * i] =
              plugin.S[ii + plugin.S.size(0) * i] -
              (((r1[ii] * dv3[m] + r1[ii + r1.size(0)] * dv3[m + 1]) +
                r1[ii + r1.size(0) * 2] * dv3[m + 2]) +
               r1[ii + r1.size(0) * 3] * dv3[m + 3]);
        }
      }
    } else {
      binary_expand_op(plugin, y_tmp, dv3);
    }
  }
  std::memset(&y[0], 0, 64U * sizeof(double));
  if (doReset) {
    plugin.f_s = 48000.0;
    plugin.Z[(static_cast<int>(plugin.pos_Vin) +
              plugin.Z.size(0) * (static_cast<int>(plugin.pos_Vin) - 1)) -
             1] = plugin.P[static_cast<int>(plugin.pos_Vin) - 1];
    b[0] = plugin.P[static_cast<int>(plugin.pos_R[0]) - 1];
    b[1] = plugin.P[static_cast<int>(plugin.pos_R[1]) - 1];
    b[2] = plugin.P[static_cast<int>(plugin.pos_R[2]) - 1];
    b[3] = plugin.P[static_cast<int>(plugin.pos_R[3]) - 1];
    coder::diag(b, dv);
    for (int ii{0}; ii < 4; ii++) {
      m = ii << 2;
      plugin.Z[(static_cast<int>(plugin.pos_R[0]) +
                plugin.Z.size(0) * (static_cast<int>(plugin.pos_R[ii]) - 1)) -
               1] = dv[m];
      plugin.Z[(static_cast<int>(plugin.pos_R[1]) +
                plugin.Z.size(0) * (static_cast<int>(plugin.pos_R[ii]) - 1)) -
               1] = dv[m + 1];
      plugin.Z[(static_cast<int>(plugin.pos_R[2]) +
                plugin.Z.size(0) * (static_cast<int>(plugin.pos_R[ii]) - 1)) -
               1] = dv[m + 2];
      plugin.Z[(static_cast<int>(plugin.pos_R[3]) +
                plugin.Z.size(0) * (static_cast<int>(plugin.pos_R[ii]) - 1)) -
               1] = dv[m + 3];
      b[ii] = 1.0 / (2.0 * plugin.P[static_cast<int>(plugin.pos_C[ii]) - 1] *
                     48000.0);
    }
    coder::diag(b, dv);
    for (int ii{0}; ii < 4; ii++) {
      m = ii << 2;
      plugin.Z[(static_cast<int>(plugin.pos_C[0]) +
                plugin.Z.size(0) * (static_cast<int>(plugin.pos_C[ii]) - 1)) -
               1] = dv[m];
      plugin.Z[(static_cast<int>(plugin.pos_C[1]) +
                plugin.Z.size(0) * (static_cast<int>(plugin.pos_C[ii]) - 1)) -
               1] = dv[m + 1];
      plugin.Z[(static_cast<int>(plugin.pos_C[2]) +
                plugin.Z.size(0) * (static_cast<int>(plugin.pos_C[ii]) - 1)) -
               1] = dv[m + 2];
      plugin.Z[(static_cast<int>(plugin.pos_C[3]) +
                plugin.Z.size(0) * (static_cast<int>(plugin.pos_C[ii]) - 1)) -
               1] = dv[m + 3];
    }
    plugin.Z[plugin.Z.size(0) + 1] = plugin.x_1 * plugin.Rp + plugin.R_tol;
    plugin.a.set_size(static_cast<int>(plugin.n));
    m = static_cast<int>(plugin.n);
    if (m - 1 >= 0) {
      std::memset(&plugin.a[0], 0,
                  static_cast<unsigned int>(m) * sizeof(double));
    }
    plugin.b.set_size(static_cast<int>(plugin.n));
    m = static_cast<int>(plugin.n);
    if (m - 1 >= 0) {
      std::memset(&plugin.b[0], 0,
                  static_cast<unsigned int>(m) * sizeof(double));
    }
    for (int ii{0}; ii < 9; ii++) {
      m = ii << 2;
      y_tmp[ii] = plugin.B[m];
      y_tmp[ii + 9] = plugin.B[m + 1];
      y_tmp[ii + 18] = plugin.B[m + 2];
      y_tmp[ii + 27] = plugin.B[m + 3];
    }
    coder::eye(plugin.n, plugin.S);
    std::copy(&plugin.B[0], &plugin.B[36], &dv3[0]);
    m = plugin.Z.size(1);
    r.set_size(4, plugin.Z.size(1));
    for (int ii{0}; ii < m; ii++) {
      r[4 * ii] = 0.0;
      loop_ub = 4 * ii + 1;
      r[loop_ub] = 0.0;
      b_loop_ub = 4 * ii + 2;
      r[b_loop_ub] = 0.0;
      i2 = 4 * ii + 3;
      r[i2] = 0.0;
      for (int i{0}; i < 9; i++) {
        d = plugin.Z[i + plugin.Z.size(0) * ii];
        i3 = i << 2;
        r[4 * ii] = r[4 * ii] + plugin.B[i3] * d;
        r[loop_ub] = r[loop_ub] + plugin.B[i3 + 1] * d;
        r[b_loop_ub] = r[b_loop_ub] + plugin.B[i3 + 2] * d;
        r[i2] = r[i2] + plugin.B[i3 + 3] * d;
      }
    }
    std::memset(&dv[0], 0, sizeof(double) << 4);
    for (int ii{0}; ii < 4; ii++) {
      m = ii << 2;
      for (int i{0}; i < 9; i++) {
        d = y_tmp[i + 9 * ii];
        loop_ub = i << 2;
        dv[m] += r[loop_ub] * d;
        dv[m + 1] += r[loop_ub + 1] * d;
        dv[m + 2] += r[loop_ub + 2] * d;
        dv[m + 3] += r[loop_ub + 3] * d;
      }
    }
    coder::mldivide(dv, dv3);
    if ((plugin.S.size(0) == plugin.Z.size(0)) && (plugin.S.size(1) == 9)) {
      b_loop_ub = plugin.Z.size(0);
      r1.set_size(plugin.Z.size(0), 4);
      for (int i1{0}; i1 < 4; i1++) {
        for (int ii{0}; ii < b_loop_ub; ii++) {
          r1[ii + r1.size(0) * i1] = 0.0;
        }
        for (int ii{0}; ii < 9; ii++) {
          d = y_tmp[ii + 9 * i1];
          for (int i{0}; i < b_loop_ub; i++) {
            m = i + r1.size(0) * i1;
            r1[m] = r1[m] + 2.0 * plugin.Z[i + plugin.Z.size(0) * ii] * d;
          }
        }
      }
      plugin.S.set_size(plugin.Z.size(0), 9);
      for (int ii{0}; ii < b_loop_ub; ii++) {
        for (int i{0}; i < 9; i++) {
          m = i << 2;
          plugin.S[ii + plugin.S.size(0) * i] =
              plugin.S[ii + plugin.S.size(0) * i] -
              (((r1[ii] * dv3[m] + r1[ii + r1.size(0)] * dv3[m + 1]) +
                r1[ii + r1.size(0) * 2] * dv3[m + 2]) +
               r1[ii + r1.size(0) * 3] * dv3[m + 3]);
        }
      }
    } else {
      binary_expand_op(plugin, y_tmp, dv3);
    }
  } else {
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
        plugin.b[static_cast<int>(plugin.pos_R[0]) - 1] = 0.0;
        plugin.b[static_cast<int>(plugin.pos_R[1]) - 1] = 0.0;
        plugin.b[static_cast<int>(plugin.pos_R[2]) - 1] = 0.0;
        plugin.b[static_cast<int>(plugin.pos_R[3]) - 1] = 0.0;
        plugin.b[static_cast<int>(plugin.pos_C[0]) - 1] =
            plugin.a[static_cast<int>(plugin.pos_C[0]) - 1];
        plugin.b[static_cast<int>(plugin.pos_C[1]) - 1] =
            plugin.a[static_cast<int>(plugin.pos_C[1]) - 1];
        plugin.b[static_cast<int>(plugin.pos_C[2]) - 1] =
            plugin.a[static_cast<int>(plugin.pos_C[2]) - 1];
        plugin.b[static_cast<int>(plugin.pos_C[3]) - 1] =
            plugin.a[static_cast<int>(plugin.pos_C[3]) - 1];
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

void viola_preamp_eq_delete()
{
  if (!plugin.matlabCodegenIsDeleted) {
    plugin.matlabCodegenIsDeleted = true;
  }
}

void viola_preamp_eq_emx_free()
{
}

void viola_preamp_eq_emx_init()
{
}

void viola_preamp_eq_init()
{
  plugin_not_empty = false;
}

void viola_preamp_eq_new()
{
  plugin.matlabCodegenIsDeleted = true;
}

// End of code generation (viola_preamp_eq.cpp)
