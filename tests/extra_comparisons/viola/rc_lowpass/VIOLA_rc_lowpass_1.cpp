//
// VIOLA_rc_lowpass_1.cpp
//
// Code generation for function 'VIOLA_rc_lowpass_1'
//

// Include files
#include "VIOLA_rc_lowpass_1.h"
#include "coder_array.h"
#include <cstring>

// Function Definitions
void binary_expand_op(VIOLA_rc_lowpass_1 &in1, double in2)
{
  coder::array<double, 2U> c_in1;
  coder::array<double, 2U> r1;
  coder::array<double, 1U> r;
  double b_in1[3];
  int loop_ub;
  int stride_0_0;
  int stride_0_1;
  int stride_1_0;
  loop_ub = in1.Z.size(0);
  r.set_size(loop_ub);
  if (loop_ub - 1 >= 0) {
    std::memset(&r[0], 0, static_cast<unsigned int>(loop_ub) * sizeof(double));
  }
  for (int i{0}; i < 3; i++) {
    double d;
    d = in1.B[i];
    for (int i1{0}; i1 < loop_ub; i1++) {
      r[i1] = r[i1] + 2.0 * in1.Z[i1 + in1.Z.size(0) * i] * d;
    }
    b_in1[i] = in1.B[i] / in2;
  }
  r1.set_size(loop_ub, 3);
  for (int i{0}; i < 3; i++) {
    for (int i1{0}; i1 < loop_ub; i1++) {
      r1[i1 + r1.size(0) * i] = r[i1] * b_in1[i];
    }
  }
  if (r1.size(0) == 1) {
    loop_ub = in1.S.size(0);
  } else {
    loop_ub = r1.size(0);
  }
  c_in1.set_size(loop_ub, 3);
  stride_0_0 = (in1.S.size(0) != 1);
  stride_0_1 = (in1.S.size(1) != 1);
  stride_1_0 = (r1.size(0) != 1);
  for (int i{0}; i < 3; i++) {
    for (int i1{0}; i1 < loop_ub; i1++) {
      c_in1[i1 + c_in1.size(0) * i] =
          in1.S[i1 * stride_0_0 + in1.S.size(0) * (i * stride_0_1)] -
          r1[i1 * stride_1_0 + r1.size(0) * i];
    }
  }
  in1.S.set_size(loop_ub, 3);
  loop_ub = c_in1.size(0) * 3;
  for (int i{0}; i < loop_ub; i++) {
    in1.S[i] = c_in1[i];
  }
}

// End of code generation (VIOLA_rc_lowpass_1.cpp)
