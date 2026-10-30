//
// viola_preamp_eq_terminate.cpp
//
// Code generation for function 'viola_preamp_eq_terminate'
//

// Include files
#include "viola_preamp_eq_terminate.h"
#include "viola_preamp_eq.h"
#include "viola_preamp_eq_data.h"
#include <cstring>

// Function Definitions
void viola_preamp_eq_terminate()
{
  viola_preamp_eq_delete();
  viola_preamp_eq_emx_free();
  isInitialized_viola_preamp_eq = false;
}

// End of code generation (viola_preamp_eq_terminate.cpp)
