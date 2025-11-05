#import "Basic";
#import "File";
#import "Math";
#load "simple_triode.jai";

main :: () {
    print("Simple Triode test\n");
    context.print_style.default_format_float.mode = .SCIENTIFIC;

    fs : float32 : 48000.0;

    impedances: Impedances;
    calc_impedances(*impedances, fs);
    state := State.{
        Cp_z = xx -118.959396,
    };

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
