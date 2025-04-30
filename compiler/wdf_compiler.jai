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
    // print("circuit:\n%\n", sections.circuit);
    // print("inputs:\n%\n", sections.inputs);
    // print("outputs:\n%\n", sections.outputs);

    circuit_tokens := parse_circuit(sections.circuit);
    generate_circuit(circuit_tokens);
}

Token_Type :: enum {
        Ideal_Voltage_Source;
        Ideal_Current_Source;
        Resistor;
        Capacitor;
        Series;
        Parallel;
    }

Token :: struct {
    name: string;
    type: Token_Type;
    args: [] string;
    parent: *Token;
}

generate_circuit :: (tokens: [] Token) {
    is_root :: (token: Token) -> bool {
        if token.type == {
            case .Ideal_Voltage_Source; return true;
            case .Ideal_Current_Source; return true;
        }
        return false;
    }

    state_vars : [..] string;
    impedance_vars : [..] string;
    methods : [..] string;

    for token : tokens {
        array_add(*state_vars, tprint("%_a", token.name));
        array_add(*state_vars, tprint("%_b", token.name));
        
        if token.type == {
            case .Capacitor; array_add(*state_vars, tprint("%_z", token.name));
        }

        if ! is_root(token) {
            array_add(*impedance_vars, tprint("%_R", token.name));
            array_add(*impedance_vars, tprint("%_G", token.name));


        }
    }

    print("%\n", tokens);
    print("%\n", state_vars);
    print("%\n", impedance_vars);
}

parse_circuit :: (circuit_str: string) -> [] Token {
    split_tokens := String.split(circuit_str, cast(u8) #char ";");

    tokens : [..] Token;
    Children :: struct {
        token: *Token;
        num_children: int;
        countdown: int;
    }
    children_stack: [..] Children;

    for *token : split_tokens {
        token_str := String.trim(token.*);
        if token_str.count == 0 { continue; }
        
        found_token_tag, tag, args := String.split_from_left(token_str, #char "(");
        // remove closing parenthese from args
        assert(args[args.count - 1] == #char ")", tprint("Token % is missing closing parenthese", tag));
        args.count = args.count - 1;

        num_children := 0;
        new_token: Token;
        if tag == {
            case "R"; new_token.type = .Resistor;
            case "C"; new_token.type = .Capacitor;
            case "IVs";
                new_token.type = .Ideal_Voltage_Source;
                num_children = 1;
            case "Series";
                new_token.type = .Series;
                num_children = 2;
            case; assert(false, tprint("Unknown token type: %", tag));
        }

        args_split := String.split(args, cast(u8) #char ",");
        new_token.name = args_split[0];

        new_token.args.data = args_split.data + 1;
        new_token.args.count = args_split.count - 1;

        if children_stack.count != 0 {
            parent_children := peek_pointer(children_stack);
            parent_children.countdown = parent_children.countdown - 1;
            new_token.parent = parent_children.token;

        }
        
        array_add(*tokens, new_token);
        
        if num_children != 0 {
            array_add(*children_stack, .{peek_pointer(tokens), num_children, num_children});
        }
    }
    // print("%\n", tokens);
    return tokens;
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
