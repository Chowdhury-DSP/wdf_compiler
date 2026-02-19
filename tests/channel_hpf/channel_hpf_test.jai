#import "Basic";
#import "File";
#import "Math";
#load "channel_hpf.jai";

main :: () {
    print("Channel HPF test\n");
    context.print_style.default_format_float.mode = .SCIENTIFIC;

    fs : float32 : 48000.0;

    params := Params.{
        Sw1_closed = 1.0,
    };
    impedances: Impedances;
    calc_impedances(*impedances, fs, params);
    state: State;

    data_str, _ := read_entire_file("data.bin");
    ref_output : [] float32 = .{data=cast(*float32)data_str.data,
                                count=data_str.count/size_of(float32)};

    max_error : float32 = 0.0;
    for 0..99
    {
        test_output := process(*state, *impedances, ifx it == 0 then 1.0 else 0.0);
        error := abs (test_output - ref_output[it]);
        max_error = max (error, max_error);
    }

    params.Sw1_closed = 0.0;
    calc_impedances (*impedances, fs, params);
    for 0..99
    {
        test_output := process(*state, *impedances, ifx it == 0 then 1.0 else 0.0);
        error := abs (test_output - ref_output[it + 100]);
        max_error = max (error, max_error);
    }

    print("Max error: %\n", max_error);

    if (max_error > 1.0e-4)
    {
        print("Error is too large... failing test!\n");
        exit(1);
    }
}
