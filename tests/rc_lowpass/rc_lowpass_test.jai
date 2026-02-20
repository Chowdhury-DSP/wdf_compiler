#import "Basic";
#import "File";
#import "Math";
#import "Random";
#load "rc_lowpass.jai";

main :: () {
    print("RC Lowpass test\n");
    context.print_style.default_format_float.mode = .SCIENTIFIC;

    fs : float32 : 48000.0;

    impedances: Impedances;
    calc_impedances(*impedances, fs);
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

    cli_args := get_command_line_arguments();
    if cli_args.count > 1 && cli_args[1] == "run_bench" {
        print("Running bench...\n");

        M :: 100_000_000;
        n_iter :: 4;

        data_in : [..] float;
        array_resize(*data_in, M);
        data_out : [..] float;
        array_resize(*data_out, M);

        time_accum : Apollo_Time = .{};
        save_out := 0.0;
        for 0..n_iter-1 {
            for 0..M-1 {
                data_in[it] = random_get_zero_to_one() * 2.0 - 1.0;
            }

            start := current_time_monotonic();
            for 0..M-1 {
                data_out[it] = process(*state, *impedances, data_in[it]);
            }
            end := current_time_monotonic();
            save_out += data_out[M-1];
            time_accum += (end - start);
        }
        ns_per_sample := (cast(float) to_nanoseconds(time_accum)) / n_iter / M;

        context.print_style.default_format_float.mode = .SHORTEST;
        print("% ns/sample\n", ns_per_sample);
    }
}
