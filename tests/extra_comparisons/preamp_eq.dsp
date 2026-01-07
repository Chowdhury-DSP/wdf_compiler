import("stdfaust.lib");

preamp_eq(V) = wd.buildtree(connection_tree)
with{

    //declare nodes
    v_in(i) = wd.u_voltage(i, V);
    lfc_cap(i) = wd.capacitor(i, 4.7e-9);
    r_series(i) = wd.resistor(i, 10.0e3);
    hfl_res(i) = wd.resistor(i, 80.0e3);
    hfl_cap(i) = wd.capacitor(i, 2.7e-9);
    hfc_cap(i) = wd.capacitor_Vout(i, 3.3e-6);
    r_shunt(i) = wd.resistor(i, 4.674e3);
    lfl_res(i) = wd.resistor(i, 15.0e3);
    lfl_cap(i) = wd.capacitor(i, 0.15e-6);

    //declare tree
    connection_tree = v_in :
        (wd.series : (
            lfc_cap,
            (wd.series : (
                (wd.parallel : (
                    r_series,
                    (wd.series : (hfl_res, hfl_cap))
                )),
                (wd.parallel : (
                    hfc_cap,
                    (wd.series : (
                        r_shunt,
                        (wd.parallel : (hfl_res, hfl_cap))
                    ))
                ))
            ))
        ));
};

process = preamp_eq;

// faust --language cpp preamp_eq.dsp > preamp_eq_faust.h
