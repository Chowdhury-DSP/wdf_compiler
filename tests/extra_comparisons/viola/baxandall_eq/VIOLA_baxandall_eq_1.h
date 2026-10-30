//
// VIOLA_baxandall_eq_1.h
//
// Code generation for function 'VIOLA_baxandall_eq_1'
//

#ifndef VIOLA_BAXANDALL_EQ_1_H
#define VIOLA_BAXANDALL_EQ_1_H

// Include files
#include "rtwtypes.h"
#include "coder_array.h"
#include <cstddef>
#include <cstdlib>

// Type Definitions
class VIOLA_baxandall_eq_1 {
public:
  bool matlabCodegenIsDeleted;
  double f_s;
  double x_1;
  double Volume;
  char Enable[2];
  double n;
  double pos_Vin;
  double pos_R[8];
  double pos_C[5];
  double B[84];
  double P[70];
  double O[2];
  coder::array<double, 2U> Z;
  coder::array<double, 2U> S;
  coder::array<double, 1U> a;
  coder::array<double, 1U> b;
  double Rp;
  double R_tol;
};

// Function Declarations
void binary_expand_op(VIOLA_baxandall_eq_1 &in1, const double in2[84],
                      const double in3[84]);

#endif
// End of code generation (VIOLA_baxandall_eq_1.h)
