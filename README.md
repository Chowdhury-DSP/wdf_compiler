# WDF Compiler

WDF Compiler is a code generation system for developing circuit simulations using Wave Digital Filters (WDFs).

## Usage

WDF Compiler is intended to be used as follows:
1. Write your circuit description
2. Use `wdf_compiler` to generate your circuit simulation code
3. Compile the generated code into your own executable

### Circuit Description

`wdf_compiler` uses its own circuit description language. To write a circuit description, you can declare each WDF element in your circuit, followed by that element's "children". For example, here is a circuit description for a simple RC lowpass filter:
```
circuit:
IVs(Vin);
    Series(S1);
        R(R1, 1k);
        C(C1, 1u);

inputs:
Vin

outputs:
V:C1
```
For more examples of the circuit description language, see the `tests/` directory.

### Compilation Stage

Let's say that you've written your circuit description in a file called `my_circuit.wdf`. Simulation code can be generated for the circuit using the command:
```bash
wdf_compiler my_circuit.wdf my_circuit.h
```
By default, `wdf_compiler` will generate C++ code, using `float` as the base data type. `wdf_compiler` also supports generating code in JAI:
```bash
wdf_compiler -lang jai my_circuit.wdf my_circuit.jai
```

`wdf_compiler` also supports generating code using arbitrary data types:
```bash
wdf_compiler -dtype double my_circuit.wdf my_circuit.h
```

Run `wdf_compiler -help` to see additional options.

## Development

### Building

To build the `wdf_compiler` for yourself, you must have the JAI compiler installed. `wdf_compiler` is currently built with JAI beta version 0.2.023, but most recent versions should work.

To build the compiler, run:
```bash
jai compiler/wdf_compiler.jai
```

### Tests

To run the `wdf_compiler` tests, run:
```bash
./tests/test_runner.sh
```

### Performance Benchmarks

To run the `wdf_compiler` performance benchmarks, run:
```bash
./tests/test_runner.sh clang bench
```

## License

WDF Compiler is open-source software published under the MIT.
