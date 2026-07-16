#import "Basic";
CLI :: #import "Command_Line";
File :: #import "File";
#import "wdf_compile";

argument_flags :: CLI.Default_Argument_Flags;

main :: () {
    // context setup...
    context.allocator = temp;
    context.print_style.default_format_float.mode = .SCIENTIFIC;
    context.print_style.default_format_float.zero_removal = .ONE_ZERO_AFTER_DECIMAL;

    args_success, args, is_set, free_args := CLI.parse_arguments(Args, argument_flags);
    if ! args_success { exit(1); }

    if args.version {
        print("wdf_compiler version: %\n", COMPILER_VERSION);
        exit(0);
    }

    if args.lang != "cpp" && args.lang != "jai" && args.lang != "c" && args.lang != "rust" {
        print("Unknown language! Supported languages are: cpp, jai, c, rust\n");
        CLI.show_help (Args, argument_flags, .[]);
        exit(1);
    }

    if args.lang == "rust" && ! is_set.dtype { args.dtype = "f32"; }

    if !args_success || free_args.count < 2 {
        print("Incorrect arguments! Usage: wdf_compiler <input_file> <output_file>\n");
        CLI.show_help (Args, argument_flags, .[]);
        exit(1);
    }

    input_file := free_args[0];
    output_file := free_args[1];
    print("Compiling WDF from file %, to %\n", input_file, output_file);

    file_str, fread_success := File.read_entire_file(input_file);
    if !fread_success { exit (1); }

    output_dir := directory_of_path(output_file);
    success, gen_code := compile_wdf(file_str, args, output_dir);
    if !success { exit(1); }

    gen_file, open_success := File.file_open(output_file, true);
    File.file_write(*gen_file, gen_code);
}
