#import "Basic";
#import "File";
#import "Math";
#load "rc_bandpass_var.jai";

main :: () {
    print("RC Bandpass test\n");
    context.print_style.default_format_float.mode = .SCIENTIFIC;

    fs : float32 : 48000.0;

    params := Params.{
        C1_value = 1.0e-6,
        R1_value = 1.0e3,
        R2_value = 1.0e3,
        C2_value = 1.0e-6,
    };
    impedances: Impedances;
    calc_impedances(*impedances, fs, params);
    state: State;

    data_str, _ := read_entire_file("data.bin");
    ref_output : [] float32 = .{data=cast(*float32)data_str.data,
                                count=data_str.count/4};

    max_error : float32 = 0.0;
    for 0..99
    {
        test_output := process(*state, *impedances, ifx it == 0 then 1.0 else 0.0);
        error := abs (test_output - ref_output[it]);
        max_error = max (error, max_error);
    }

    params.R2_value = 100.0;
    params.C2_value = 1.0e-9;
    calc_impedances_lpf2 (*impedances, fs, params);
    for 0..99
    {
        test_output := process(*state, *impedances, ifx it == 0 then 1.0 else 0.0);
        error := abs (test_output - ref_output[it + 100]);
        max_error = max (error, max_error);
    }

    params.R1_value = 10.0e3;
    params.C1_value = 1.0e-4;
    calc_impedances_lpf1 (*impedances, fs, params);
    for 0..99
    {
        test_output := process(*state, *impedances, ifx it == 0 then 1.0 else 0.0);
        error := abs (test_output - ref_output[it + 200]);
        max_error = max (error, max_error);
    }

    print("Max error: %\n", max_error);

    if (max_error > 1.0e-4)
    {
        print("Error is too large... failing test!\n");
        exit(1);
    }
}
