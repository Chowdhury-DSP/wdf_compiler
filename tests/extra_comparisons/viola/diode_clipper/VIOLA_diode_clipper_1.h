//
// VIOLA_diode_clipper_1.h
//
// Code generation for function 'VIOLA_diode_clipper_1'
//

#ifndef VIOLA_DIODE_CLIPPER_1_H
#define VIOLA_DIODE_CLIPPER_1_H

// Include files
#include "rtwtypes.h"
#include "coder_array.h"
#include <cstddef>
#include <cstdlib>

// Type Definitions
class VIOLA_diode_clipper_1 {
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
  double pos_Dap;
  double B[8];
  double P[20];
  double O[2];
  coder::array<double, 2U> Z;
  coder::array<double, 2U> S;
  coder::array<double, 1U> a;
  coder::array<double, 1U> b;
  coder::array<double, 2U> Ap;
  double alpha;
  double beta;
  double pos;
  double order[4];
  double Rp;
  double R_tol;
};

// Function Declarations
void binary_expand_op(VIOLA_diode_clipper_1 &in1, const double in2[8]);

#endif
// End of code generation (VIOLA_diode_clipper_1.h)
