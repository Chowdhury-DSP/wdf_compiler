//
// viola_diode_clipper.cpp
//
// Code generation for function 'viola_diode_clipper'
//

// Include files
#include "viola_diode_clipper.h"
#include "VIOLA_diode_clipper_1.h"
#include "diag.h"
#include "eye.h"
#include "inv.h"
#include "mldivide.h"
#include "nullAssignment.h"
#include "strcmp.h"
#include "viola_diode_clipper_data.h"
#include "viola_diode_clipper_initialize.h"
#include "coder_array.h"
#include <algorithm>
#include <cmath>
#include <cstring>

// Variable Definitions
static VIOLA_diode_clipper_1 plugin;

static bool plugin_not_empty;

// Function Definitions
void viola_diode_clipper(const double x[64], int n, bool doReset, double y[64])
{
  static const double dv1[20]{
      1.0E-9, 1.0E-6,  1000.0, 1.0E-9, 0.0, 0.0,    0.0, 1.0, 0.0, 0.0,
      0.0,    0.02585, 0.0,    0.0,    0.0, 1.0E-6, 0.0, 0.0, 0.0, 1.0E+12};
  static const double dv[8]{
      -0.9999999999999998, -0.0, 1.0, -1.0, 1.0, 0.0, 0.0, 1.0};
  static const signed char iv[12]{1, -1, 0, 0, -1, 1, 1, 0, -1, 1, 0, -1};
  coder::array<double, 2U> c_tmp_data;
  coder::array<double, 2U> r;
  coder::array<double, 2U> r1;
  coder::array<double, 2U> r2;
  coder::array<double, 2U> r3;
  coder::array<double, 2U> tmp_data;
  coder::array<double, 1U> temp;
  double in_data[64];
  double b_tmp_data[16];
  double dv2[8];
  double b_temp[4];
  double e;
  int tmp_size[2];
  int b_loop_ub;
  int c_loop_ub;
  int loop_ub;
  if (!isInitialized_viola_diode_clipper) {
    viola_diode_clipper_initialize();
  }
  //  Benchmark entry point written by run_viola_codegen.m for VIOLA's
  //  VIOLA_diode_clipper_1 plug-in.
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
    plugin.pos_Dap = 4.0;
    plugin.R_tol = 1.0E-6;
    plugin.n = 4.0;
    std::copy(&dv[0], &dv[8], &plugin.B[0]);
    plugin.Rp = 2000.0;
    std::copy(&dv1[0], &dv1[20], &plugin.P[0]);
    plugin.O[0] = 2.0;
    plugin.O[1] = 1.0;
    plugin.Z.set_size(4, 4);
    loop_ub = static_cast<int>(plugin.n) * static_cast<int>(plugin.n);
    if (loop_ub - 1 >= 0) {
      std::memset(&plugin.Z[0], 0,
                  static_cast<unsigned int>(loop_ub) * sizeof(double));
    }
    plugin.alpha = 3.0;
    plugin.beta = 1.0;
    plugin.pos = 4.0;
    tmp_size[0] = 3;
    tmp_size[1] = 4;
    for (int i{0}; i < 12; i++) {
      b_tmp_data[i] = iv[i];
    }
    coder::internal::nullAssignment(b_tmp_data, tmp_size, 4);
    coder::internal::b_nullAssignment(b_tmp_data, tmp_size, 3);
    plugin.Ap.set_size(tmp_size[0], tmp_size[1]);
    loop_ub = tmp_size[0] * tmp_size[1];
    if (loop_ub - 1 >= 0) {
      std::copy(&b_tmp_data[0], &b_tmp_data[loop_ub], &plugin.Ap[0]);
    }
    plugin.order[0] = 1.0;
    plugin.order[1] = 3.0;
    plugin.order[2] = 2.0;
    plugin.order[3] = 4.0;
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
    coder::diag(plugin.Z, temp);
    b_temp[0] = temp[static_cast<int>(plugin.order[0]) - 1];
    b_temp[1] = temp[static_cast<int>(plugin.order[1]) - 1];
    b_temp[2] = temp[static_cast<int>(plugin.order[2]) - 1];
    b_temp[3] = temp[static_cast<int>(plugin.order[3]) - 1];
    coder::diag(b_temp, b_tmp_data);
    tmp_size[0] = 4;
    tmp_size[1] = 4;
    coder::internal::b_nullAssignment(b_tmp_data, tmp_size,
                                      static_cast<int>(plugin.pos));
    coder::internal::nullAssignment(b_tmp_data, tmp_size,
                                    static_cast<int>(plugin.pos));
    c_tmp_data.set(&b_tmp_data[0], tmp_size[0], tmp_size[1]);
    coder::inv(c_tmp_data, plugin.S);
    c_loop_ub = plugin.Ap.size(0);
    b_loop_ub = plugin.S.size(1);
    r.set_size(plugin.Ap.size(0), plugin.S.size(1));
    for (int i{0}; i < c_loop_ub; i++) {
      for (int i1{0}; i1 < b_loop_ub; i1++) {
        e = 0.0;
        loop_ub = plugin.Ap.size(1);
        for (int ii{0}; ii < loop_ub; ii++) {
          e += plugin.Ap[i + plugin.Ap.size(0) * ii] *
               plugin.S[ii + plugin.S.size(0) * i1];
        }
        r[i + r.size(0) * i1] = e;
      }
    }
    r1.set_size(plugin.Ap.size(0), plugin.Ap.size(0));
    for (int ii{0}; ii < c_loop_ub; ii++) {
      for (int i{0}; i < c_loop_ub; i++) {
        r1[i + r1.size(0) * ii] = 0.0;
      }
      for (int i{0}; i < b_loop_ub; i++) {
        e = plugin.Ap[ii + plugin.Ap.size(0) * i];
        for (int i1{0}; i1 < c_loop_ub; i1++) {
          loop_ub = i1 + r1.size(0) * ii;
          r1[loop_ub] = r1[loop_ub] + r[i1 + r.size(0) * i] * e;
        }
      }
    }
    coder::inv(r1, plugin.S);
    plugin.Z[(static_cast<int>(plugin.pos_Dap) +
              plugin.Z.size(0) * (static_cast<int>(plugin.pos_Dap) - 1)) -
             1] =
        plugin.S[(static_cast<int>(plugin.beta) +
                  plugin.S.size(0) * (static_cast<int>(plugin.beta) - 1)) -
                 1];
    coder::eye(plugin.n, plugin.S);
    loop_ub = plugin.Z.size(1);
    r2.set_size(2, plugin.Z.size(1));
    for (int i{0}; i < loop_ub; i++) {
      r2[2 * i] = 0.0;
      b_loop_ub = 2 * i + 1;
      r2[b_loop_ub] = 0.0;
      for (int i1{0}; i1 < 4; i1++) {
        e = plugin.Z[i1 + plugin.Z.size(0) * i];
        c_loop_ub = i1 << 1;
        r2[2 * i] = r2[2 * i] + plugin.B[c_loop_ub] * e;
        r2[b_loop_ub] = r2[b_loop_ub] + plugin.B[c_loop_ub + 1] * e;
      }
    }
    std::memset(&b_temp[0], 0, sizeof(double) << 2);
    for (int i{0}; i < 2; i++) {
      loop_ub = i << 1;
      for (int i1{0}; i1 < 4; i1++) {
        b_loop_ub = i1 << 1;
        e = plugin.B[i + b_loop_ub];
        b_temp[loop_ub] += r2[b_loop_ub] * e;
        b_temp[loop_ub + 1] += r2[b_loop_ub + 1] * e;
      }
    }
    coder::mldivide(b_temp, plugin.B, dv2);
    if ((plugin.S.size(0) == plugin.Z.size(0)) && (plugin.S.size(1) == 4)) {
      c_loop_ub = plugin.Z.size(0);
      r3.set_size(plugin.Z.size(0), 2);
      for (int ii{0}; ii < 2; ii++) {
        for (int i{0}; i < c_loop_ub; i++) {
          r3[i + r3.size(0) * ii] = 0.0;
        }
        for (int i{0}; i < 4; i++) {
          e = plugin.B[ii + (i << 1)];
          for (int i1{0}; i1 < c_loop_ub; i1++) {
            loop_ub = i1 + r3.size(0) * ii;
            r3[loop_ub] =
                r3[loop_ub] + 2.0 * plugin.Z[i1 + plugin.Z.size(0) * i] * e;
          }
        }
      }
      plugin.S.set_size(plugin.Z.size(0), 4);
      for (int i{0}; i < c_loop_ub; i++) {
        for (int i1{0}; i1 < 4; i1++) {
          loop_ub = i1 << 1;
          plugin.S[i + plugin.S.size(0) * i1] =
              plugin.S[i + plugin.S.size(0) * i1] -
              (r3[i] * dv2[loop_ub] + r3[i + r3.size(0)] * dv2[loop_ub + 1]);
        }
      }
    } else {
      binary_expand_op(plugin, dv2);
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
    coder::diag(plugin.Z, temp);
    b_temp[0] = temp[static_cast<int>(plugin.order[0]) - 1];
    b_temp[1] = temp[static_cast<int>(plugin.order[1]) - 1];
    b_temp[2] = temp[static_cast<int>(plugin.order[2]) - 1];
    b_temp[3] = temp[static_cast<int>(plugin.order[3]) - 1];
    coder::diag(b_temp, b_tmp_data);
    tmp_size[0] = 4;
    tmp_size[1] = 4;
    coder::internal::b_nullAssignment(b_tmp_data, tmp_size,
                                      static_cast<int>(plugin.pos));
    coder::internal::nullAssignment(b_tmp_data, tmp_size,
                                    static_cast<int>(plugin.pos));
    tmp_data.set(&b_tmp_data[0], tmp_size[0], tmp_size[1]);
    coder::inv(tmp_data, plugin.S);
    c_loop_ub = plugin.Ap.size(0);
    b_loop_ub = plugin.S.size(1);
    r.set_size(plugin.Ap.size(0), plugin.S.size(1));
    for (int i{0}; i < c_loop_ub; i++) {
      for (int i1{0}; i1 < b_loop_ub; i1++) {
        e = 0.0;
        loop_ub = plugin.Ap.size(1);
        for (int ii{0}; ii < loop_ub; ii++) {
          e += plugin.Ap[i + plugin.Ap.size(0) * ii] *
               plugin.S[ii + plugin.S.size(0) * i1];
        }
        r[i + r.size(0) * i1] = e;
      }
    }
    r1.set_size(plugin.Ap.size(0), plugin.Ap.size(0));
    for (int ii{0}; ii < c_loop_ub; ii++) {
      for (int i{0}; i < c_loop_ub; i++) {
        r1[i + r1.size(0) * ii] = 0.0;
      }
      for (int i{0}; i < b_loop_ub; i++) {
        e = plugin.Ap[ii + plugin.Ap.size(0) * i];
        for (int i1{0}; i1 < c_loop_ub; i1++) {
          loop_ub = i1 + r1.size(0) * ii;
          r1[loop_ub] = r1[loop_ub] + r[i1 + r.size(0) * i] * e;
        }
      }
    }
    coder::inv(r1, plugin.S);
    plugin.Z[(static_cast<int>(plugin.pos_Dap) +
              plugin.Z.size(0) * (static_cast<int>(plugin.pos_Dap) - 1)) -
             1] =
        plugin.S[(static_cast<int>(plugin.beta) +
                  plugin.S.size(0) * (static_cast<int>(plugin.beta) - 1)) -
                 1];
    coder::eye(plugin.n, plugin.S);
    loop_ub = plugin.Z.size(1);
    r2.set_size(2, plugin.Z.size(1));
    for (int i{0}; i < loop_ub; i++) {
      r2[2 * i] = 0.0;
      b_loop_ub = 2 * i + 1;
      r2[b_loop_ub] = 0.0;
      for (int i1{0}; i1 < 4; i1++) {
        e = plugin.Z[i1 + plugin.Z.size(0) * i];
        c_loop_ub = i1 << 1;
        r2[2 * i] = r2[2 * i] + plugin.B[c_loop_ub] * e;
        r2[b_loop_ub] = r2[b_loop_ub] + plugin.B[c_loop_ub + 1] * e;
      }
    }
    std::memset(&b_temp[0], 0, sizeof(double) << 2);
    for (int i{0}; i < 2; i++) {
      loop_ub = i << 1;
      for (int i1{0}; i1 < 4; i1++) {
        b_loop_ub = i1 << 1;
        e = plugin.B[i + b_loop_ub];
        b_temp[loop_ub] += r2[b_loop_ub] * e;
        b_temp[loop_ub + 1] += r2[b_loop_ub + 1] * e;
      }
    }
    coder::mldivide(b_temp, plugin.B, dv2);
    if ((plugin.S.size(0) == plugin.Z.size(0)) && (plugin.S.size(1) == 4)) {
      b_loop_ub = plugin.Z.size(0);
      r3.set_size(plugin.Z.size(0), 2);
      for (int ii{0}; ii < 2; ii++) {
        for (int i{0}; i < b_loop_ub; i++) {
          r3[i + r3.size(0) * ii] = 0.0;
        }
        for (int i{0}; i < 4; i++) {
          e = plugin.B[ii + (i << 1)];
          for (int i1{0}; i1 < b_loop_ub; i1++) {
            loop_ub = i1 + r3.size(0) * ii;
            r3[loop_ub] =
                r3[loop_ub] + 2.0 * plugin.Z[i1 + plugin.Z.size(0) * i] * e;
          }
        }
      }
      plugin.S.set_size(plugin.Z.size(0), 4);
      for (int i{0}; i < b_loop_ub; i++) {
        for (int i1{0}; i1 < 4; i1++) {
          loop_ub = i1 << 1;
          plugin.S[i + plugin.S.size(0) * i1] =
              plugin.S[i + plugin.S.size(0) * i1] -
              (r3[i] * dv2[loop_ub] + r3[i + r3.size(0)] * dv2[loop_ub + 1]);
        }
      }
    } else {
      binary_expand_op(plugin, dv2);
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
        double a;
        double alpha;
        double b_beta_tmp;
        double beta;
        double beta_tmp;
        double f;
        plugin.b[static_cast<int>(plugin.pos_Vin) - 1] = x[ii];
        plugin.b[static_cast<int>(plugin.pos_R) - 1] = 0.0;
        plugin.b[static_cast<int>(plugin.pos_C) - 1] =
            plugin.a[static_cast<int>(plugin.pos_C) - 1];
        b_loop_ub = plugin.S.size(1);
        tmp_data.set_size(1, plugin.S.size(1));
        for (int i{0}; i < b_loop_ub; i++) {
          tmp_data[i] =
              plugin
                  .S[(static_cast<int>(plugin.pos_Dap) + plugin.S.size(0) * i) -
                     1];
        }
        loop_ub = plugin.b.size(0);
        temp.set_size(plugin.b.size(0));
        if (loop_ub - 1 >= 0) {
          std::copy(&plugin.b[0], &plugin.b[loop_ub], &temp[0]);
        }
        a = 0.0;
        for (int i{0}; i < b_loop_ub; i++) {
          a += tmp_data[i] * temp[i];
        }
        double Z;
        double alpha_tmp;
        double alpha_tmp_tmp;
        double b_alpha_tmp;
        double c_alpha_tmp;
        double mod_a;
        Z = plugin
                .Z[(static_cast<int>(plugin.pos_Dap) +
                    plugin.Z.size(0) * (static_cast<int>(plugin.pos_Dap) - 1)) -
                   1];
        mod_a = std::abs(a);
        alpha_tmp = plugin.P[static_cast<int>(plugin.pos_Dap) + 15];
        alpha_tmp_tmp = plugin.P[static_cast<int>(plugin.pos_Dap) + 11];
        e = alpha_tmp + alpha_tmp_tmp;
        b_alpha_tmp = plugin.P[static_cast<int>(plugin.pos_Dap) - 1];
        c_alpha_tmp = e + Z;
        alpha =
            (2.0 * alpha_tmp * b_alpha_tmp * Z + mod_a * (e - Z)) / c_alpha_tmp;
        e = alpha_tmp_tmp + Z;
        f = plugin.P[static_cast<int>(plugin.pos_Dap) + 3];
        beta_tmp = plugin.P[static_cast<int>(plugin.pos_Dap) + 7];
        b_beta_tmp = 2.0 * f * beta_tmp * Z;
        beta = b_beta_tmp / e;
        f = (std::log(alpha_tmp * b_alpha_tmp * e /
                      (f * beta_tmp * c_alpha_tmp)) +
             mod_a * (Z - alpha_tmp_tmp) / b_beta_tmp) +
            alpha / beta;
        if (f < -3.6) {
          b_beta_tmp = 0.929404810843623 * std::exp(0.986418416898303 * f) -
                       6.79639452545093E-6;
        } else if (f < 0.0) {
          b_beta_tmp =
              (((-0.00431200176082888 * std::exp(3.55275697783407 * f) +
                 0.00792228971991872 * std::pow(f, 3.0)) +
                0.0907827580611564 * (f * f)) +
               0.375633000771424 * f) +
              0.571426071298747;
        } else if (f < 2.5) {
          b_beta_tmp = (((-0.531567672565454 * std::exp(0.26741260001099 * f) -
                          0.00273225759962195 * std::pow(f, 3.0)) +
                         0.0953348873301105 * (f * f)) +
                        0.503184890060992 * f) +
                       1.09876994061414;
        } else if (f < 7.0) {
          b_beta_tmp = (((1.32189881177891 * std::exp(-2.0564214137968 * f) -
                          0.00164391849625201 * std::pow(f, 3.0)) +
                         0.0428488786259462 * (f * f)) +
                        0.481822411934234 * f) +
                       0.418369998866986;
        } else if (f < 30.0) {
          e = std::log(f + 0.217073722991741);
          b_beta_tmp = ((1.00007068672705 * f - 1.00147317311501 * e) +
                        1.08561409526715 * e / f) +
                       0.516677184575301 * e * (e - 2.0) / (f * f);
        } else {
          e = std::log(f);
          b_beta_tmp = ((f - e) + e / f) + 0.5 * e * (e - 2.0) / (f * f);
        }
        e = std::exp(f - b_beta_tmp);
        f = b_beta_tmp - e;
        beta_tmp = f * e / (e + 1.0);
        b_beta_tmp -= f * (1.0 - 0.5 * (beta_tmp * beta_tmp)) / (e + 1.0);
        if (a < 0.0) {
          e = -1.0;
        } else {
          e = (a > 0.0);
        }
        plugin.b[static_cast<int>(plugin.pos_Dap) - 1] =
            e * (alpha - beta * b_beta_tmp);
        loop_ub = plugin.b.size(0);
        temp.set_size(plugin.b.size(0));
        if (loop_ub - 1 >= 0) {
          std::copy(&plugin.b[0], &plugin.b[loop_ub], &temp[0]);
        }
        loop_ub = plugin.S.size(0);
        c_loop_ub = plugin.S.size(1);
        plugin.a.set_size(plugin.S.size(0));
        if (loop_ub - 1 >= 0) {
          std::memset(&plugin.a[0], 0,
                      static_cast<unsigned int>(loop_ub) * sizeof(double));
        }
        for (int i{0}; i < c_loop_ub; i++) {
          for (int i1{0}; i1 < loop_ub; i1++) {
            plugin.a[i1] =
                plugin.a[i1] + plugin.S[i1 + plugin.S.size(0) * i] * temp[i];
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

void viola_diode_clipper_delete()
{
  if (!plugin.matlabCodegenIsDeleted) {
    plugin.matlabCodegenIsDeleted = true;
  }
}

void viola_diode_clipper_emx_free()
{
}

void viola_diode_clipper_emx_init()
{
}

void viola_diode_clipper_init()
{
  plugin_not_empty = false;
}

void viola_diode_clipper_new()
{
  plugin.matlabCodegenIsDeleted = true;
}

// End of code generation (viola_diode_clipper.cpp)
