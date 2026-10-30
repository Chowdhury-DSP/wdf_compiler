//
// viola_rc_lowpass.cpp
//
// Code generation for function 'viola_rc_lowpass'
//

// Include files
#include "viola_rc_lowpass.h"
#include "VIOLA_rc_lowpass_1.h"
#include "eye.h"
#include "strcmp.h"
#include "viola_rc_lowpass_data.h"
#include "viola_rc_lowpass_initialize.h"
#include "coder_array.h"
#include <algorithm>
#include <cstring>

// Variable Definitions
static VIOLA_rc_lowpass_1 plugin;

static bool plugin_not_empty;

// Function Definitions
void viola_rc_lowpass(const double x[64], int n, bool doReset, double y[64])
{
  static const double dv[15]{1.0E-9, 1.0E-6, 1000.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                             0.0,    0.0,    0.0,    0.0, 0.0, 0.0, 0.0};
  coder::array<double, 2U> r;
  coder::array<double, 1U> r1;
  double in_data[64];
  double dv1[3];
  double A;
  int i;
  int m;
  if (!isInitialized_viola_rc_lowpass) {
    viola_rc_lowpass_initialize();
  }
  //  Benchmark entry point written by run_viola_codegen.m for VIOLA's
  //  VIOLA_rc_lowpass_1 plug-in.
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
    plugin.pos_R = 3.0;
    plugin.pos_C = 2.0;
    plugin.R_tol = 1.0E-6;
    plugin.n = 3.0;
    plugin.B[0] = -0.9999999999999998;
    plugin.B[1] = 1.0;
    plugin.B[2] = 1.0;
    plugin.Rp = 2000.0;
    std::copy(&dv[0], &dv[15], &plugin.P[0]);
    plugin.O[0] = 2.0;
    plugin.O[1] = 1.0;
    plugin.Z.set_size(3, 3);
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
    plugin.Z[(static_cast<int>(plugin.pos_R) +
              plugin.Z.size(0) * (static_cast<int>(plugin.pos_R) - 1)) -
             1] = plugin.P[static_cast<int>(plugin.pos_R) - 1];
    plugin.Z[(static_cast<int>(plugin.pos_C) +
              plugin.Z.size(0) * (static_cast<int>(plugin.pos_C) - 1)) -
             1] =
        1.0 / (2.0 * plugin.P[static_cast<int>(plugin.pos_C) - 1] * 48000.0);
    plugin.Z[plugin.Z.size(0) * 2 + 2] = plugin.x_1 * plugin.Rp + plugin.R_tol;
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
    m = plugin.Z.size(1);
    r.set_size(1, plugin.Z.size(1));
    for (int ii{0}; ii < m; ii++) {
      i = plugin.Z.size(0) * ii;
      r[ii] = (plugin.B[0] * plugin.Z[i] + plugin.B[1] * plugin.Z[i + 1]) +
              plugin.B[2] * plugin.Z[i + 2];
    }
    A = (r[0] * plugin.B[0] + r[1] * plugin.B[1]) + r[2] * plugin.B[2];
    coder::eye(plugin.n, plugin.S);
    if ((plugin.S.size(0) == plugin.Z.size(0)) && (plugin.S.size(1) == 3)) {
      m = plugin.Z.size(0);
      r1.set_size(plugin.Z.size(0));
      if (m - 1 >= 0) {
        std::memset(&r1[0], 0, static_cast<unsigned int>(m) * sizeof(double));
      }
      for (int ii{0}; ii < 3; ii++) {
        for (int i2{0}; i2 < m; i2++) {
          r1[i2] = r1[i2] +
                   2.0 * plugin.Z[i2 + plugin.Z.size(0) * ii] * plugin.B[ii];
        }
        dv1[ii] = plugin.B[ii] / A;
      }
      plugin.S.set_size(plugin.Z.size(0), 3);
      for (int ii{0}; ii < 3; ii++) {
        for (int i2{0}; i2 < m; i2++) {
          plugin.S[i2 + plugin.S.size(0) * ii] =
              plugin.S[i2 + plugin.S.size(0) * ii] - r1[i2] * dv1[ii];
        }
      }
    } else {
      binary_expand_op(plugin, A);
    }
  }
  std::memset(&y[0], 0, 64U * sizeof(double));
  if (doReset) {
    plugin.f_s = 48000.0;
    plugin.Z[(static_cast<int>(plugin.pos_Vin) +
              plugin.Z.size(0) * (static_cast<int>(plugin.pos_Vin) - 1)) -
             1] = plugin.P[static_cast<int>(plugin.pos_Vin) - 1];
    plugin.Z[(static_cast<int>(plugin.pos_R) +
              plugin.Z.size(0) * (static_cast<int>(plugin.pos_R) - 1)) -
             1] = plugin.P[static_cast<int>(plugin.pos_R) - 1];
    plugin.Z[(static_cast<int>(plugin.pos_C) +
              plugin.Z.size(0) * (static_cast<int>(plugin.pos_C) - 1)) -
             1] =
        1.0 / (2.0 * plugin.P[static_cast<int>(plugin.pos_C) - 1] * 48000.0);
    plugin.Z[plugin.Z.size(0) * 2 + 2] = plugin.x_1 * plugin.Rp + plugin.R_tol;
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
    m = plugin.Z.size(1);
    r.set_size(1, plugin.Z.size(1));
    for (int ii{0}; ii < m; ii++) {
      i = plugin.Z.size(0) * ii;
      r[ii] = (plugin.B[0] * plugin.Z[i] + plugin.B[1] * plugin.Z[i + 1]) +
              plugin.B[2] * plugin.Z[i + 2];
    }
    A = (r[0] * plugin.B[0] + r[1] * plugin.B[1]) + r[2] * plugin.B[2];
    coder::eye(plugin.n, plugin.S);
    if ((plugin.S.size(0) == plugin.Z.size(0)) && (plugin.S.size(1) == 3)) {
      m = plugin.Z.size(0);
      r1.set_size(plugin.Z.size(0));
      if (m - 1 >= 0) {
        std::memset(&r1[0], 0, static_cast<unsigned int>(m) * sizeof(double));
      }
      for (int ii{0}; ii < 3; ii++) {
        for (int i2{0}; i2 < m; i2++) {
          r1[i2] = r1[i2] +
                   2.0 * plugin.Z[i2 + plugin.Z.size(0) * ii] * plugin.B[ii];
        }
        dv1[ii] = plugin.B[ii] / A;
      }
      plugin.S.set_size(plugin.Z.size(0), 3);
      for (int ii{0}; ii < 3; ii++) {
        for (int i2{0}; i2 < m; i2++) {
          plugin.S[i2 + plugin.S.size(0) * ii] =
              plugin.S[i2 + plugin.S.size(0) * ii] - r1[i2] * dv1[ii];
        }
      }
    } else {
      binary_expand_op(plugin, A);
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
        int i1;
        plugin.b[static_cast<int>(plugin.pos_Vin) - 1] = x[ii];
        plugin.b[static_cast<int>(plugin.pos_R) - 1] = 0.0;
        plugin.b[static_cast<int>(plugin.pos_C) - 1] =
            plugin.a[static_cast<int>(plugin.pos_C) - 1];
        i = plugin.S.size(0);
        i1 = plugin.S.size(1);
        plugin.a.set_size(plugin.S.size(0));
        if (i - 1 >= 0) {
          std::memset(&plugin.a[0], 0,
                      static_cast<unsigned int>(i) * sizeof(double));
        }
        for (int i2{0}; i2 < i1; i2++) {
          for (int i3{0}; i3 < i; i3++) {
            plugin.a[i3] = plugin.a[i3] +
                           plugin.S[i3 + plugin.S.size(0) * i2] * plugin.b[i2];
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

void viola_rc_lowpass_delete()
{
  if (!plugin.matlabCodegenIsDeleted) {
    plugin.matlabCodegenIsDeleted = true;
  }
}

void viola_rc_lowpass_emx_free()
{
}

void viola_rc_lowpass_emx_init()
{
}

void viola_rc_lowpass_init()
{
  plugin_not_empty = false;
}

void viola_rc_lowpass_new()
{
  plugin.matlabCodegenIsDeleted = true;
}

// End of code generation (viola_rc_lowpass.cpp)
