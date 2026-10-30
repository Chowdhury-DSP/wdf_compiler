//
// VIOLA_preamp_eq_1.cpp
//
// Code generation for function 'VIOLA_preamp_eq_1'
//

// Include files
#include "VIOLA_preamp_eq_1.h"
#include "coder_array.h"
#include <cstring>

// Function Definitions
void binary_expand_op(VIOLA_preamp_eq_1 &in1, const double in2[36],
                      const double in3[36])
{
  coder::array<double, 2U> b_in1;
  coder::array<double, 2U> r;
  coder::array<double, 2U> r1;
  double d;
  int loop_ub;
  int stride_0_0;
  int stride_0_1;
  int stride_1_0;
  stride_0_0 = in1.Z.size(0);
  r.set_size(stride_0_0, 4);
  for (int i{0}; i < 4; i++) {
    for (int i1{0}; i1 < stride_0_0; i1++) {
      r[i1 + r.size(0) * i] = 0.0;
    }
    for (int i1{0}; i1 < 9; i1++) {
      d = in2[i1 + 9 * i];
      for (int i2{0}; i2 < stride_0_0; i2++) {
        loop_ub = i2 + r.size(0) * i;
        r[loop_ub] = r[loop_ub] + 2.0 * in1.Z[i2 + in1.Z.size(0) * i1] * d;
      }
    }
  }
  r1.set_size(stride_0_0, 9);
  for (int i{0}; i < 9; i++) {
    for (int i1{0}; i1 < stride_0_0; i1++) {
      r1[i1 + r1.size(0) * i] = 0.0;
    }
    for (int i1{0}; i1 < 4; i1++) {
      d = in3[i1 + (i << 2)];
      for (int i2{0}; i2 < stride_0_0; i2++) {
        loop_ub = i2 + r1.size(0) * i;
        r1[loop_ub] = r1[loop_ub] + r[i2 + r.size(0) * i1] * d;
      }
    }
  }
  if (r1.size(0) == 1) {
    loop_ub = in1.S.size(0);
  } else {
    loop_ub = r1.size(0);
  }
  b_in1.set_size(loop_ub, 9);
  stride_0_0 = (in1.S.size(0) != 1);
  stride_0_1 = (in1.S.size(1) != 1);
  stride_1_0 = (r1.size(0) != 1);
  for (int i1{0}; i1 < 9; i1++) {
    for (int i2{0}; i2 < loop_ub; i2++) {
      b_in1[i2 + b_in1.size(0) * i1] =
          in1.S[i2 * stride_0_0 + in1.S.size(0) * (i1 * stride_0_1)] -
          r1[i2 * stride_1_0 + r1.size(0) * i1];
    }
  }
  in1.S.set_size(loop_ub, 9);
  loop_ub = b_in1.size(0) * 9;
  for (int i1{0}; i1 < loop_ub; i1++) {
    in1.S[i1] = b_in1[i1];
  }
}

// End of code generation (VIOLA_preamp_eq_1.cpp)
