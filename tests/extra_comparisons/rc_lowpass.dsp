import("stdfaust.lib");

rc_lowpass(Vin) = wd.buildtree(connection_tree)
with{

    //declare nodes
    r1(i) = wd.resistor(i, 10.0e3);
    c1(i) = wd.capacitor_Vout(i, 1.0e-6);
    vin(i) = wd.u_voltage(i, Vin);

    //declare tree
    connection_tree = vin :
        wd.series : (
            r1,
            c1
        );
};

process = rc_lowpass;

// faust --language cpp rc_lowpass.dsp > rc_lowpass_faust.h
