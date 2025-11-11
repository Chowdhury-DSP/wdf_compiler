#import "Basic";
#import "File";
#import "Math";
#load "rc_lowpass_var.jai";

main :: () {
    print("RC Lowpass test\n");
    context.print_style.default_format_float.mode = .SCIENTIFIC;

    fs : float32 : 48000.0;
    R1_V : float32 : 0.5;
    R1_R : float32 : 15.0e3;
    Vminus_R : float32 : 25.0e3;
    C1 : float32 : 4.0e-6;

    impedances: Impedances;
    calc_impedances(*impedances,
                    fs,
                    Params.{
                        Vminus_res_value=Vminus_R,
                        R1_v_value=R1_V,
                        R1_res_value=R1_R,
                        C1_value=C1,
                    });
    state: State;

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
