//
// viola_reductions_circuit_terminate.cpp
//
// Code generation for function 'viola_reductions_circuit_terminate'
//

// Include files
#include "viola_reductions_circuit_terminate.h"
#include "viola_reductions_circuit.h"
#include "viola_reductions_circuit_data.h"
#include <cstring>

// Function Definitions
void viola_reductions_circuit_terminate()
{
  viola_reductions_circuit_delete();
  viola_reductions_circuit_emx_free();
  isInitialized_viola_reductions_circuit = false;
}

// End of code generation (viola_reductions_circuit_terminate.cpp)
