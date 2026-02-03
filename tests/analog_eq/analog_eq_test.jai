#import "Basic";
#import "File";
#import "Math";
#load "analog_eq.jai";

get_parameters :: (treble_boost: float, treble_boost_q: float, treble_boost_freq: float, treble_cut: float, treble_cut_freq: float, low_boost: float, low_cut: float, bass_freq: float) -> Params {
    // see sim/pultec_bass_freq_curve_fit.py
    P_low_cut := 1.0 + Math.pow (low_cut, 1.443) * 99.999e3;
    P_low_boost := 1.0 + Math.pow (low_boost, 2.587) * 9.999e3;
    bass_boost_cap_val := 1.0 / (2.0 * Math.PI * 1.13062281e+04 * (bass_freq - 1.30692349e+01)) + 1.68909808e-07;
    bass_cut_cap_val := 1.0 / (2.0 * Math.PI * 2.27142549e+05 * (bass_freq - 1.24565293e+01)) + 7.10814416e-09;

    treble_cut_skew := Math.pow (treble_cut, 0.395);
    P_treble_cut_plus := 1.0e3 * (1.0 - treble_cut_skew);
    P_treble_cut_minus := 1.0e3 * treble_cut_skew;
    treble_cut_cap_val := 1.0 / (2.0 * Math.PI * 169.3 * treble_cut_freq);

    treble_boost_skew := Math.pow (treble_boost, 0.621);
    P_treble_boost_plus := 10.0e3 * (1.0 - treble_boost_skew);
    P_treble_boost_minus := 10.0e3 * treble_boost_skew;
    R_treble_boost_bw := 1.0 + (1.0 - treble_boost_q) * 2499.0;

    // see sim/pultec_treble_boost_freq_curve_fit.py
    A_r := Math.sqrt (1.0e7);
    g_val := 9.75043069e-05 + treble_boost_freq * (-2.05889893e-08 + treble_boost_freq * (1.73050404e-12 + treble_boost_freq * -4.96665892e-17));
    treble_boost_cap_val := g_val / A_r;
    treble_boost_ind_val := A_r * A_r * treble_boost_cap_val;

    return .{
        P_treble_boost_plus_value = P_treble_boost_plus,
        P_treble_boost_minus_value = P_treble_boost_minus,
        R_treble_boost_bw_value = R_treble_boost_bw,
        C_treble_boost_value = cast(float32) treble_boost_cap_val,
        L_treble_boost_params = .{
            value = cast(float32) treble_boost_ind_val,
        },
        P_low_cut_value = P_low_cut,
        C_low_cut_value = cast(float32) bass_cut_cap_val,
        P_treble_cut_plus_value = P_treble_cut_plus,
        P_treble_cut_minus_value = P_treble_cut_minus,
        C_treble_cut_value = treble_cut_cap_val,
        P_low_boost_value = P_low_boost,
        C_low_boost_value = cast(float32) bass_boost_cap_val,
    };
}

main :: () {
    print("Analog EQ test\n");
    context.print_style.default_format_float.mode = .SCIENTIFIC;

    fs : float32 : 48000.0;
    treble_boost : float32 = 0.5;
    treble_boost_q : float32 = 0.5;
    treble_boost_freq : float32 = 5000.0;
    treble_cut : float32 = 0.25;
    treble_cut_freq : float32 = 10000.0;
    low_boost : float32 = 0.75;
    low_cut : float32 = 0.3;
    bass_freq : float32 = 100.0;

    params := get_parameters(treble_boost, treble_boost_q, treble_boost_freq, treble_cut, treble_cut_freq, low_boost, low_cut, bass_freq);
    impedances: Impedances;
    calc_impedances(*impedances, fs, params);
    state := State.{};

    data_str, _ := read_entire_file("data.bin");
    ref_output : [] float32 = .{data=cast(*float32)data_str.data,
                                count=data_str.count/size_of(float32)};

    max_error : float32 = 0.0;
    for 0..(ref_output.count-1)
    {
        test_output := process(*state, *impedances, 1.0);
        error := abs (test_output - ref_output[it]);
        max_error = max (error, max_error);
    }
    print("Max error: %\n", max_error);

    if (max_error > 1.0e-4)
    {
        print("Error is too large... failing test!\n");
        exit(1);
    }
}
