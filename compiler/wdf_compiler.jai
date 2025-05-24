#import "Basic";
CLI :: #import "Command_Line";
File :: #import "File";
String :: #import "String";
Sort :: #import "Sort";

Args :: struct {
    verbose : bool = false; @"?Verbose"
}

main :: () {
    context.allocator = temp;
    args_success, args, is_set, free_args := CLI.parse_arguments (Args);

    if !args_success || free_args.count < 2 {
        CLI.show_help (Args, CLI.Default_Argument_Flags, .[]);
        exit (1);
    }

    input_file := free_args[0];
    output_file := free_args[1];
    print("Compiling WDF from file %, to %\n", input_file, output_file);

    file_str, fread_success := File.read_entire_file(input_file);
    if !fread_success { exit (1); }

    sections := parse_file_sections (file_str);
    // print("circuit:\n%\n", sections.circuit);
    // print("inputs:\n%\n", sections.inputs);
    // print("outputs:\n%\n", sections.outputs);

    circuit_tokens := parse_circuit(sections.circuit);
    gen_code := generate_circuit(circuit_tokens);
    gen_code = tprint("%\n%", gen_code, generate_process(circuit_tokens, sections.inputs, sections.outputs));

    gen_file, success := File.file_open(output_file, true);
    File.file_write(*gen_file, gen_code);
    // print(gen_code);
}

Token_Type :: enum {
        Ideal_Voltage_Source;
        Ideal_Current_Source;
        Resistor;
        Capacitor;
        Inductor;
        Series;
        Parallel;
    }

Token :: struct {
    name: string;
    type: Token_Type;
    args: [] string;
    parent: *Token;
    children: [..] s64;
}

generate_vars_struct :: (impedance_vars: [] string, name: string) -> string {
    struct_code := tprint("struct % {\n", name);
    for var : impedance_vars {
        struct_code = tprint("%    float %;\n", struct_code, var);
    }
    struct_code = tprint("%};\n", struct_code);
    return struct_code;
}

generate_method :: (name: string, args: [] string, lines: [] string, return_type := "void") -> string {
    method_code := tprint("%1 %2(", return_type, name);
    for arg, idx : args {
        if idx != 0 { method_code = tprint("%1, ", method_code); }
        method_code = tprint("%1%2", method_code, arg);
    }
    method_code = tprint("%1) {\n", method_code);
    for line : lines {
        method_code = tprint("%1    %2\n", method_code, line);
    }
    method_code = tprint("%1}\n", method_code);

    return method_code;
}

get_value :: (val_str: string) -> float32 {
    val, success, remainder := string_to_float(val_str);
    assert(success);
    if remainder[0] == {
        case #char "k";
            val *= 1.0e3;
        case #char "m";
            val *= 1.0e-3;
        case #char "u";
            val *= 1.0e-6;
        case #char "n";
            val *= 1.0e-9;
        case #char "p";
            val *= 1.0e-12;
    }
    return val;
}

generate_process :: (tokens: [] Token, input_config: string, output_config: string) -> string {
    inputs := String.split(input_config, "\n");

    input_args : [..] string;
    array_add (*input_args, "State& state");
    array_add (*input_args, "const Impedances& impedances");

    for input : inputs {
        input_token : *Token;
        for *token : tokens {
            if token.name == input {
                input_token = token;
                break;
            }
        }
        assert(input_token != null);
        array_add(*input_args, tprint("float %", input));

        if input_token.type == {
            case .Ideal_Voltage_Source;
                input_token.args = .[input];
            case .Ideal_Current_Source;
                input_token.args = .[input];
        }
    }

    down_tree_statements : [..] string;
    up_tree_statements : [..] string;
    root_token := *tokens[0];
    for *token : tokens {
        print("%\n", token.*);

        if token == root_token {
            if token.type == {
                case .Ideal_Voltage_Source;
                    child_token := tokens[token.children[0]];
                    array_add(*up_tree_statements, tprint("state.%1_a = state.%2_b; // %1 incident", token.name, child_token.name));
                    array_add(*down_tree_statements, tprint("state.%1_b = -state.%1_a + 2 * %2; // %1 reflected", token.name, token.args[0]));
                    array_add(*down_tree_statements, tprint("state.%2_a = state.%1_b; // %2 incident", token.name, child_token.name));
                case .Ideal_Current_Source;
                    child_token := tokens[token.children[0]];
                    array_add(*up_tree_statements, tprint("state.%1_a = state.%2_b; // %1 incident", token.name, child_token.name));
                    array_add(*down_tree_statements, tprint("state.%1_b = 2 * impedances.%3_R * %2 + state.%1_a; // %1 reflected (@TODO: 2 * R?)", token.name, token.args[0], child_token.name));
                    array_add(*down_tree_statements, tprint("state.%2_a = state.%1_b; // %2 incident", token.name, child_token.name));
            }
            continue;
        }

        if token.type == {
            case .Resistor;
                array_add(*up_tree_statements, tprint("state.%1_b = 0; // %1 reflected", token.name));
            case .Capacitor;
                array_add(*up_tree_statements, tprint("state.%1_b = -state.%1_z; // %1 reflected", token.name));
                array_add(*down_tree_statements, tprint("state.%1_z = state.%1_a; // %1 state update", token.name));
            case .Inductor;
                array_add(*up_tree_statements, tprint("state.%1_b = state.%1_z; // %1 reflected", token.name));
                array_add(*down_tree_statements, tprint("state.%1_z = state.%1_a; // %1 state update", token.name));
            case .Series;
                child0_token := tokens[token.children[0]];
                child1_token := tokens[token.children[1]];
                array_add(*up_tree_statements, tprint("state.%1_b = -(state.%2_b + state.%3_b); // %1 reflected", token.name, child0_token.name, child1_token.name));
                array_add(*down_tree_statements, tprint("const auto pr_%1 = impedances.%2_R * impedances.%1_G; // %1 coefficient calculation (@TODO: pre-compute this)", token.name, child0_token.name));
                array_add(*down_tree_statements, tprint("state.%2_a = state.%2_b - pr_%1 * (state.%1_a + state.%2_b + state.%3_b); // %2 incident", token.name, child0_token.name, child1_token.name));
                array_add(*down_tree_statements, tprint("state.%3_a = -(state.%1_a + state.%2_a); // %3 incident", token.name, child0_token.name, child1_token.name));
        }
    }

    // print("%\n", input_args);
    // print("%\n", up_tree_statements);
    // print("%\n", down_tree_statements);

    code_statements : [..] string;
    array_resize(*code_statements, up_tree_statements.count + down_tree_statements.count);

    for statement, it : up_tree_statements {
        code_statements[up_tree_statements.count - it - 1] = statement;
    }
    for statement, it : down_tree_statements {
        code_statements[up_tree_statements.count + it] = statement;
    }

    // print("%\n", code_statements);
    array_add(*code_statements, "");

    outputs := String.split(output_config, "\n");
    assert(outputs.count == 1); // @TODO
    for output : outputs {
        output_info := String.split(output, ":");
        // print("%\n", output_info);

        output_token : *Token;
        for *token : tokens {
            if token.name == output_info[1] {
                output_token = token;
                break;
            }
        }
        assert(output_token != null);
        // print("%\n", output_token.*);

        if output_info[0] == {
            case "V";
                array_add(*code_statements, tprint("return (state.%1_a + state.%1_b) * 0.5f; // %1 voltage", output_token.name));
            case "I";
                array_add(*code_statements, tprint("return (state.%1_a - state.%1_b) * (0.5f * impedances.%1_G); // %1 current", output_token.name));
        }
    }

    return generate_method("process",
                           input_args,
                           code_statements,
                           "float");
}

generate_circuit :: (tokens: [] Token) -> string {
    state_vars : [..] string;
    impedance_vars : [..] string;
    
    impedance_updates : [..] string;
    Impedance_Update_Info :: struct {
        token: *Token;
        children: [..] *Token;
    }
    impedance_update_todos : [..] Impedance_Update_Info;
    add_impedance_update_todo :: (todos: *[..] Impedance_Update_Info, token: *Token, new_child: *Token) {
        for *todo : todos.* {
            if todo.token == token {
                array_add(*todo.children, new_child);
                return;
            }
        }
        new_todo: Impedance_Update_Info;
        new_todo.token = token;
        array_add(*new_todo.children, new_child);
        array_add(todos, new_todo);
    }

    root_token := *tokens[0];
    for *token : tokens {
        array_add(*state_vars, tprint("%_a", token.name));
        array_add(*state_vars, tprint("%_b", token.name));
        
        if token != root_token {
            array_add(*impedance_vars, tprint("%_R", token.name));
            array_add(*impedance_vars, tprint("%_G", token.name));
            // @TODO: some components have extra "intermediate" values we might want to compute here...
            // e.g. the Series adaptor.
        }

        if token.type == {
            case .Resistor;
                array_add(*impedance_updates, tprint("// Computing impedance for: %1;", token.name));
                if token.args.count > 0 {
                    array_add(*impedance_updates, tprint("static constexpr float %1_value = %2;", token.name, get_value(token.args[0])));
                }
                array_add(*impedance_updates, tprint("impedances.%1_R = %1_value;", token.name));
                array_add(*impedance_updates, tprint("impedances.%1_G = 1.0f / impedances.%1_R;\n", token.name));
                add_impedance_update_todo(*impedance_update_todos, token.parent, token);
            case .Capacitor;
                array_add(*impedance_updates, tprint("// Computing impedance for: %1;", token.name));
                if token.args.count > 0 {
                    array_add(*impedance_updates, tprint("static constexpr float %1_value = %2;", token.name, get_value(token.args[0])));
                }
                array_add(*state_vars, tprint("%_z", token.name));
                array_add(*impedance_updates, tprint("impedances.%1_G = 2.0f * %1_value * fs;", token.name));
                array_add(*impedance_updates, tprint("impedances.%1_R = 1.0f / impedances.%1_G;\n", token.name));
                add_impedance_update_todo(*impedance_update_todos, token.parent, token);
            case .Inductor;
                array_add(*impedance_updates, tprint("// Computing impedance for: %1;", token.name));
                if token.args.count > 0 {
                    array_add(*impedance_updates, tprint("static constexpr float %1_value = %2;", token.name, get_value(token.args[0])));
                }
                array_add(*state_vars, tprint("%_z", token.name));
                array_add(*impedance_updates, tprint("impedances.%1_R = 2.0f * %1_value * fs;", token.name));
                array_add(*impedance_updates, tprint("impedances.%1_G = 1.0f / impedances.%1_R;\n", token.name));
                add_impedance_update_todo(*impedance_update_todos, token.parent, token);
        }
    }

    while impedance_update_todos.count > 0 {
        impedance_todo := impedance_update_todos[0];
        token := impedance_todo.token;
        if token.type == {
            case .Series;
                child0_token := impedance_todo.children[0];
                child1_token := impedance_todo.children[1];
                array_add(*impedance_updates, tprint("// Computing impedance for: %1;", token.name));
                array_add(*impedance_updates, tprint("impedances.%1_R = impedances.%2_R + impedances.%3_R;", token.name, child0_token.name, child1_token.name));
                array_add(*impedance_updates, tprint("impedances.%1_G = 1.0f / impedances.%1_R;\n", token.name));
        }

        if token.parent != root_token {
            // @TODO: test me!!
            add_impedance_update_todo(*impedance_update_todos, token.parent, token);
        } else {
            // print("Finished climbing impedance tree!");
        }

        // This is leaky, but I don't think we care?
        impedance_update_todos.data += 1;
        impedance_update_todos.count -= 1;
    }

    // print("%\n", tokens);
    // print("%\n", state_vars);
    // print("%\n", impedance_vars);
    // print("%\n", impedance_updates);
    // print("%\n", impedance_update_todos);

    gen_code := "#pragma once\n";
    gen_code = tprint("%\n%", gen_code, generate_vars_struct(impedance_vars, "Impedances"));
    gen_code = tprint("%\n%", gen_code, generate_vars_struct(state_vars, "State"));
    gen_code = tprint("%\n%", gen_code, generate_method("calc_impedances",
                                                        .["Impedances& impedances", "float fs"],
                                                        impedance_updates));
    return gen_code;
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
            case "L"; new_token.type = .Inductor;
            case "IVs";
                new_token.type = .Ideal_Voltage_Source;
                num_children = 1;
            case "IIs";
                new_token.type = .Ideal_Current_Source;
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
        for *arg : new_token.args {
            arg.* = String.trim(arg.*);
        }

        if children_stack.count != 0 {
            parent_children := peek_pointer(children_stack);
            parent_children.countdown = parent_children.countdown - 1;
            new_token.parent = parent_children.token;
            array_add(*parent_children.token.children, tokens.count);
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
