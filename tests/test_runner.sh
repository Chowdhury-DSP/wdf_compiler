#!/bin/bash

set -e
shopt -s expand_aliases
source ~/.bashrc

if [[ "$OSTYPE" == "darwin"* ]]; then
    sudo_pass=$(more ~/spass)
fi
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)

if [[ -n "$1" ]]; then
    cpp_compiler="$1"
else
    cpp_compiler="clang"
fi

if [ ! -d "${SCRIPT_DIR}/xsimd" ]; then
    git clone https://github.com/xtensor-stack/xsimd "${SCRIPT_DIR}/xsimd"
fi
if [[ "$*" = *bench* || "$*" = *lang-perf* ]]; then
    echo "Running bench tests..."
    cpp_bench_flags="-DRUN_BENCH=1 -O3 -I${SCRIPT_DIR}/xsimd/include"
    echo "Extra flags: $cpp_bench_flags"
fi
if [[ "$*" = *lang-perf* ]]; then
    echo "Running JAI perf tests..."
    jai_compile_bench_flags="-release"
    jai_run_bench_flags="run_bench"
    c_bench_flags="-DRUN_BENCH=1 -O3"
    rust_bench_flags="-O --cfg RUN_BENCH"
fi

if [[ "$OSTYPE" == "win32" || "$OSTYPE" == "msys" ]]; then
   wdf_compiler="${SCRIPT_DIR}/../compiler/wdf_compiler.exe"
   libcpp_flag=""
else
   wdf_compiler="${SCRIPT_DIR}/../compiler/wdf_compiler"
   libcpp_flag="-lstdc++"
fi

cpp_test () {
   test="$1"
   echo "Running CPP Test: $test"
   cd "${SCRIPT_DIR}/${test}"
   rm -f data.bin

   wdf_compiler_flags=""
   cpp_compiler_flags=${libcpp_flag}
   if [[ "$test" == *"double"* ]]; then
       wdf_compiler_flags="-dtype double"
   elif [[ "$test" == *"simd"* ]]; then
       wdf_compiler_flags="-dtype WDF_Float"
       cpp_compiler_flags="${cpp_compiler_flags} -I${SCRIPT_DIR}/xsimd/include"
   fi

   $wdf_compiler "${test}.wdf" "${test}.h" ${wdf_compiler_flags}
   $cpp_compiler "${test}.cpp" ${cpp_bench_flags} -I../../lib --std=c++20 ${cpp_compiler_flags} -o "${test}.exe"
   if [[ "$OSTYPE" == "darwin"* ]]; then
      echo ${sudo_pass} | sudo -S "./${test}.exe"
   else
      "./${test}.exe"
   fi
}

c_test () {
   test="$1"
   echo "Running C Test: $test"
   cd "${SCRIPT_DIR}/${test}"

   wdf_compiler_flags="-lang c"
   if [[ "$test" == *"double"* ]]; then
       wdf_compiler_flags="${wdf_compiler_flags} -dtype double"
   fi

   $wdf_compiler "${test}.wdf" "${test}_c.h" ${wdf_compiler_flags}
   clang ${test}.c ${c_bench_flags} -D_CRT_SECURE_NO_WARNINGS -Wno-c2x-extensions -I../../lib -o ${test}_c.exe
   "./${test}_c.exe"
}

jai_test () {
   test="$1"
   echo "Running JAI Test: $test"
   cd "${SCRIPT_DIR}/${test}"

   wdf_compiler_flags="-lang jai"
   if [[ "$test" == *"double"* ]]; then
       wdf_compiler_flags="${wdf_compiler_flags} -dtype float64"
   fi

   $wdf_compiler "${test}.wdf" "${test}.jai" ${wdf_compiler_flags}
   jai ${jai_compile_bench_flags} -quiet "${test}_test.jai"
   if [[ "$OSTYPE" == "win32" || "$OSTYPE" == "msys" ]]; then
      "./${test}_test.exe" ${jai_run_bench_flags}
   else
      "./${test}_test" ${jai_run_bench_flags}
   fi
}

rust_test () {
   test="$1"
   echo "Running Rust Test: $test"
   cd "${SCRIPT_DIR}/${test}"

   wdf_compiler_flags="-lang rust"
   if [[ "$test" == *"double"* ]]; then
       wdf_compiler_flags="${wdf_compiler_flags} -dtype f64"
   fi

   $wdf_compiler "${test}.wdf" "${test}.rs" ${wdf_compiler_flags}
   rustc ${rust_bench_flags} ${test}_test.rs -o ${test}_rust.exe
   "./${test}_rust.exe"
}

test () {
   test="$1"
   # The C++ test needs to run first,
   # since it generates the reference data.
   if [[ "$*" = *cpp* ]]; then cpp_test $test; fi
   if [[ "$*" = *jai* ]]; then jai_test $test; fi
   if [[ "$*" = *c_lang* ]]; then c_test $test; fi
   if [[ "$*" = *rust* ]]; then rust_test $test; fi
}

if [[ "$*" = *bench* ]]; then
   # cpp_test rc_lowpass
   cpp_test rc_lowpass_double
   cpp_test rc_lowpass_simd
   # cpp_test rc_bandpass
   # cpp_test preamp_eq
   # cpp_test diode_clipper
   # cpp_test simple_triode
   # cpp_test baxandall_eq
   # cpp_test pulse_shaper
   # cpp_test reductions_circuit
   # cpp_test reductions_circuit2
   # cpp_test analog_eq
elif [[ "$*" = *lang-perf* ]]; then
    # The C++ test needs to run first,
    # since it generates the reference data.
    cpp_test rc_lowpass
    jai_test rc_lowpass
    c_test rc_lowpass
    rust_test rc_lowpass

    cpp_test preamp_eq
    jai_test preamp_eq
    c_test preamp_eq
    rust_test preamp_eq

    cpp_test baxandall_eq
    jai_test baxandall_eq
    c_test baxandall_eq
    rust_test baxandall_eq
else
   test rc_lowpass cpp jai c_lang rust
   test rc_lowpass_double cpp jai c_lang rust
   test rc_lowpass_simd cpp
   test rc_bandpass cpp jai c_lang rust
   test rl_lowpass cpp jai c_lang rust
   test rc_lowpass_var cpp jai c_lang rust
   test rl_lowpass_var cpp jai c_lang rust
   test rc_bandpass_var cpp jai c_lang rust
   test rc_lowpass_2ins cpp jai c_lang rust
   test preamp_eq cpp jai c_lang rust
   test preamp_eq_comb cpp jai c_lang rust
   test hard_clipper cpp jai c_lang rust
   test diode_clipper cpp jai
   test diode_circuit cpp jai
   test simple_triode cpp jai
   test bassman_tone_stack cpp jai c_lang rust
   test baxandall_eq cpp jai c_lang rust
   test pulse_shaper cpp jai
   test reductions_circuit cpp jai c_lang rust
   test reductions_circuit2 cpp jai c_lang rust
   test analog_eq cpp jai rust
   test channel_hpf cpp jai c_lang rust
fi

if [[ "$*" = *bad_configs* ]]; then
    echo "Running bad config tests..."
    cd "${SCRIPT_DIR}/bad_configs"
    python3 run_tests.py
fi
