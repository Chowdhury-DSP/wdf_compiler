# Ciaramella reference implementations

The Ciaramella sources (`*.crm`) and external bindings (`externals.h`) for the RC lowpass, pre-amp EQ, diode clipper and Baxandall EQ are copied from the [Zampogna](https://github.com/paolomarrone/Zampogna) repository, `examples/wdf_comp/crm/`, at commit `abacd7e2947a1ad4d2667c3c87094bcbc05dba0b`. They are distributed under the ISC license (see `LICENSE`). `reductions_circuit/reductions_circuit.crm` was written manually, using the same element and adaptor definitions.

The `externals.h` files depend on headers from this repository: `lib/wdf_lib_omega.h` for the diode clipper, and `tests/baxandall_eq/custom_baxandall_rtype.h` for the Baxandall EQ.
