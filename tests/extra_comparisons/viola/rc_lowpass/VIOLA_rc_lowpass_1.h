//
// VIOLA_rc_lowpass_1.h
//
// Code generation for function 'VIOLA_rc_lowpass_1'
//

#ifndef VIOLA_RC_LOWPASS_1_H
#define VIOLA_RC_LOWPASS_1_H

// Include files
#include "rtwtypes.h"
#include "coder_array.h"
#include <cstddef>
#include <cstdlib>

// Type Definitions
class VIOLA_rc_lowpass_1 {
public:
  bool matlabCodegenIsDeleted;
  double f_s;
  double x_1;
  double Volume;
  char Enable[2];
  double n;
  double pos_Vin;
  double pos_R;
  double pos_C;
  double B[3];
  double P[15];
  double O[2];
  coder::array<double, 2U> Z;
  coder::array<double, 2U> S;
  coder::array<double, 1U> a;
  coder::array<double, 1U> b;
  double Rp;
  double R_tol;
};

// Function Declarations
void binary_expand_op(VIOLA_rc_lowpass_1 &in1, double in2);

#endif
// End of code generation (VIOLA_rc_lowpass_1.h)
