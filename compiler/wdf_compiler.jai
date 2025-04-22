#import "Basic";
CLI :: #import "Command_Line";
File :: #import "File";
String :: #import "String";
Sort :: #import "Sort";

Args :: struct {
    verbose : bool = false; @"?Verbose"
}

main :: () {
    args_success, args, is_set, free_args := CLI.parse_arguments (Args);

    if !args_success || free_args.count < 1 {
        CLI.show_help (Args, CLI.Default_Argument_Flags, .[]);
        exit (1);
    }

    filename := free_args[0];
    print("Compiling WDF from file %\n", filename);

    file_str, fread_success := File.read_entire_file(filename);
    if !fread_success { exit (1); }

    sections := parse_file_sections (file_str);
    print("circuit:\n%\n", sections.circuit);
    print("inputs:\n%\n", sections.inputs);
    print("outputs:\n%\n", sections.outputs);
}

File_Sections :: struct {
    circuit: string;
    inputs: string;
    outputs: string;
}

parse_file_sections :: (file_str: string) -> File_Sections {
    Section_Type :: enum {
        Circuit;
        Inputs;
        Outputs;
    }

    Section_Info :: struct {
        start_in_file: s64;
        start_after_tag: s64;
        type: Section_Type;
    }

    section_info: [..] Section_Info;

    add_section_info :: (file_str: string,
                         section_info: *[..] Section_Info,
                         tag: string,
                         type: Section_Type) {
        section_start := String.find_index_from_left (file_str, tag);
        if section_start < 0 { return; }

        start_after_tag := section_start + tag.count;
        array_add (section_info, Section_Info.{ section_start, start_after_tag, type });
    }
    add_section_info (file_str, *section_info, "circuit:", .Circuit);
    add_section_info (file_str, *section_info, "inputs:", .Inputs);
    add_section_info (file_str, *section_info, "outputs:", .Outputs);

    Sort.quick_sort (section_info, x => -x.start_in_file);

    sections: File_Sections;
    file_str_temp := file_str;
    for info: section_info {
        section_str := String.trim (string.{ file_str_temp.count - info.start_after_tag, file_str_temp.data + info.start_after_tag });
        file_str_temp.count = info.start_in_file;

        if info.type == {
            case .Circuit; sections.circuit = section_str;
            case .Inputs; sections.inputs = section_str;
            case .Outputs; sections.outputs = section_str;
        }
    }
    return sections;
}
