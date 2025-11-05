#import "Basic";
#import "File";
#import "Math";
#load "rl_lowpass_var.jai";

main :: () {
    print("RL Lowpass test\n");
    context.print_style.default_format_float.mode = .SCIENTIFIC;

    fs : float32 : 48000.0;
    Iplus_I : float32 : 1.0;
    Ivar_I : float32 : -0.99;
    R1 : float32 : 59.0e3;

    impedances: Impedances;
    calc_impedances(*impedances,
                    fs,
                    Params.{
                        Iplus_i_value=Iplus_I,
                        Ivar_i_value=Ivar_I,
                        R1_value=R1,
                    });
    state: State;

    data_str, _ := read_entire_file("data.bin");
    ref_output : [] float32 = .{data=cast(*float32)data_str.data,
                                count=data_str.count/4};

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

