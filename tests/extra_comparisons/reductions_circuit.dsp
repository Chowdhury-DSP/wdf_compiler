import("stdfaust.lib");

reductions_circuit(Vin) = wd.buildtree(connection_tree)
with{

    //declare nodes
    vin(i) = wd.u_voltage(i, Vin);
    rv1(i) = wd.resistor(i, 10.0e3);
    rv2(i) = wd.resistor(i, 5.0e3);
    rl(i) = wd.resistor_Vout(i, 100.0e3);
    r1(i) = wd.resistor(i, 1.0e3);
    r2(i) = wd.resistor(i, 5.0e3);
    r3(i) = wd.resistor(i, 4.0e3);
    r4(i) = wd.resistor(i, 10.0e3);
    r5(i) = wd.resistor(i, 3.0e3);
    r6(i) = wd.resistor(i, 10.0e3);
    r7(i) = wd.resistor(i, 10.0e3);
    r8(i) = wd.resistor(i, 100.0e3);
    c1(i) = wd.capacitor(i, 1.0e-6);
    c2(i) = wd.capacitor(i, 5.0e-6);
    c3(i) = wd.capacitor(i, 4.0e-9);
    c4(i) = wd.capacitor(i, 10.0e-9);
    c5(i) = wd.capacitor(i, 3.0e-6);
    c6(i) = wd.capacitor(i, 4.0e-6);
    c7(i) = wd.capacitor(i, 10.0e-6);
    c8(i) = wd.capacitor(i, 100.0e-9);

    //declare tree
    resistor_combos = wd.parallel : (
        (wd.series : ((wd.series : (r1, r2)), r3)),
        (wd.parallel : (r4, r5))
    );
    capacitor_combos = wd.parallel : (
        (wd.series : ((wd.series : (c1, c2)), c3)),
        (wd.parallel : (c4, c5))
    );
    rc_series = wd.series : (
        (wd.series : ((wd.series : (resistor_combos, capacitor_combos)), r6)),
        c6
    );
    rc_parallel = wd.parallel : (
        (wd.parallel : ((wd.parallel : (r7, c7)), r8)),
        c8
    );

    connection_tree = vin :
        wd.series : (
            rv1,
            (wd.series : (
                rv2,
                (wd.series : (
                    rl,
                    (wd.series : (rc_series, rc_parallel))
                ))
            ))
        );
};

process = reductions_circuit;

// faust -t 0 --language cpp reductions_circuit.dsp > reductions_circuit_faust.h
// Note that "-t 0" disables Faust's compile timeout. With Faust 2.85.5, this took about 23 minutes and 3.5 GB of memory.
