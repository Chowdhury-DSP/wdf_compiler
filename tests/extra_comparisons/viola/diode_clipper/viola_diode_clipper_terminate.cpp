//
// viola_diode_clipper_terminate.cpp
//
// Code generation for function 'viola_diode_clipper_terminate'
//

// Include files
#include "viola_diode_clipper_terminate.h"
#include "viola_diode_clipper.h"
#include "viola_diode_clipper_data.h"
#include <cstring>

// Function Definitions
void viola_diode_clipper_terminate()
{
  viola_diode_clipper_delete();
  viola_diode_clipper_emx_free();
  isInitialized_viola_diode_clipper = false;
}

// End of code generation (viola_diode_clipper_terminate.cpp)
