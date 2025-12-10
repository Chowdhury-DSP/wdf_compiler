#import "Basic";
#import "File";
#import "Math";
#load "bassman_tone_stack.jai";

main :: () {
    print("Bassman Tone Stack test\n");
    context.print_style.default_format_float.mode = .SCIENTIFIC;

    fs : float32 : 48000.0;

    params := Params.{
        Vin_Res3m_res_value = 12.5e3,
        Res2_Res3p_value = 512.5e3,
        Res1p_Res1m_value = 250.0e3,
    };
    impedances: Impedances;
    calc_impedances(*impedances, fs, params);
    state := State.{};

    data_str, _ := read_entire_file("data.bin");
    ref_output : [] float32 = .{data=cast(*float32)data_str.data,
                                count=data_str.count/size_of(float32)};

    max_error : float32 = 0.0;
    for 0..(ref_output.count-1)
    {
        v := 1.0;
        v_Vin_Res3m, v_Res1p_Res1m_Cap1 := process(*state, *impedances, v);
        test_output := v_Res1p_Res1m_Cap1 + (v_Vin_Res3m - v);
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
