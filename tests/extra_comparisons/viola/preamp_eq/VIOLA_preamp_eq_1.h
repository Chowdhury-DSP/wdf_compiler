//
// VIOLA_preamp_eq_1.h
//
// Code generation for function 'VIOLA_preamp_eq_1'
//

#ifndef VIOLA_PREAMP_EQ_1_H
#define VIOLA_PREAMP_EQ_1_H

// Include files
#include "rtwtypes.h"
#include "coder_array.h"
#include <cstddef>
#include <cstdlib>

// Type Definitions
class VIOLA_preamp_eq_1 {
public:
  bool matlabCodegenIsDeleted;
  double f_s;
  double x_1;
  double Volume;
  char Enable[2];
  double n;
  double pos_Vin;
  double pos_R[4];
  double pos_C[4];
  double B[36];
  double P[45];
  double O[2];
  coder::array<double, 2U> Z;
  coder::array<double, 2U> S;
  coder::array<double, 1U> a;
  coder::array<double, 1U> b;
  double Rp;
  double R_tol;
};

// Function Declarations
void binary_expand_op(VIOLA_preamp_eq_1 &in1, const double in2[36],
                      const double in3[36]);

#endif
// End of code generation (VIOLA_preamp_eq_1.h)
