#import "Basic";
CLI :: #import "Command_Line";
File :: #import "File";
Process :: #import "Process";
#load "wdf_parser.jai";
#load "scratch.jai";

Args :: struct {
    verbose      : bool = false;     @"?Verbose"
    version      : bool = false;     @"?Version"
    lang         : string = "cpp";   @"?Language of generated code. Currently supports cpp (default) and jai."
    dtype        : string = "float"; @"?Data type to use for generated code. (Default: float)"
    align        : int = 16;         @"?Default data alignment to use for generated code. (Default: 16)"
    no_reduce    : bool = false;     @"?Skip circuit element reductions (Default: false)"
    one_multiply : bool = false;     @"?Use 1-multiply junctions for series adaptors and others (Default: false)"
}
argument_flags :: CLI.Default_Argument_Flags;
global_args: Args;

get_compiler_version :: () -> string {
    result, output, err, timed_out := Process.run_command("git", "rev-parse", "--short", "HEAD", capture_and_return_output=true);
    return tprint("1.0.0.%", String.trim(output));
}

COMPILER_VERSION :: #run get_compiler_version();

main :: () {
    // context setup...
    context.allocator = temp;
    context.print_style.default_format_float.mode = .SCIENTIFIC;
    context.print_style.default_format_float.zero_removal = .ONE_ZERO_AFTER_DECIMAL;
    set_initial_data(*scratch, SCRATCH_SIZE, alloc(SCRATCH_SIZE));

    args_success, args, is_set, free_args := CLI.parse_arguments (Args, argument_flags);
    if ! args_success { exit(1); }

    if args.version {
        print("wdf_compiler version: %\n", COMPILER_VERSION);
        exit(0);
    }

    if args.lang != "cpp" && args.lang != "jai" {
        print("Unknown language! Supported languages are: cpp, jai\n");
        CLI.show_help (Args, argument_flags, .[]);
        exit(1);
    }

    if !args_success || free_args.count < 2 {
        print("Incorrect arguments! Usage: wdf_compiler <input_file> <output_file>\n");
        CLI.show_help (Args, argument_flags, .[]);
        exit(1);
    }
    global_args = args;
    verbose_print("Args: %\n", args);

    input_file := free_args[0];
    output_file := free_args[1];
    print("Compiling WDF from file %, to %\n", input_file, output_file);

    file_str, fread_success := File.read_entire_file(input_file);
    if !fread_success { exit (1); }

    sections := parse_file_sections (file_str);
    error_assert(sections.circuit.count > 0, "Missing \"circuit\" section");
    error_assert(sections.inputs.count > 0, "Missing \"input\" section");
    error_assert(sections.outputs.count > 0, "Missing \"output\" section");

    inputs := String.split(sections.inputs, "\n");
    for *input : inputs { input.* = String.trim(input.*); }
    outputs := String.split(sections.outputs, "\n");
    for *output : outputs { output.* = String.trim(output.*); }

    meta_info := parse_meta_info(sections.meta);
    circuit_tokens := parse_circuit(sections.circuit);

    if ! args.no_reduce {
        circuit_tokens = reduce_circuit(circuit_tokens, inputs, outputs);
    }

    process_code, required_impedances := generate_process(circuit_tokens, inputs, outputs);
    updaters_code := generate_updaters(circuit_tokens, inputs, meta_info, *required_impedances);
    circuit_code := generate_circuit(circuit_tokens, inputs, required_impedances);

    gen_code := start_gen_code(meta_info);
    gen_code = tprint("%\n%", gen_code, circuit_code);
    gen_code = tprint("%\n%", gen_code, updaters_code);
    gen_code = tprint("%\n%", gen_code, process_code);
    gen_code = tprint("%\n%", gen_code, end_gen_code(meta_info));

    gen_file, success := File.file_open(output_file, true);
    File.file_write(*gen_file, gen_code);
    // print(gen_code);

    verbose_print("Bytes allocated: %\n", context.temporary_storage.total_bytes_occupied);
}

start_gen_code :: (meta_info: Meta_Info) -> string {
    ts := context.temporary_storage;
    start_scratch();
    builder: String_Builder;

    cli_args := get_command_line_arguments();
    print_to_builder(*builder, "// Auto-generated with wdf_compiler version %.\n// Command: wdf_compiler", COMPILER_VERSION);
    for i : 1..cli_args.count - 1 {
        print_to_builder(*builder, " %", cli_args[i]);
    }

    print_to_builder(*builder, "\n");
    if global_args.lang == "cpp" { print_to_builder(*builder, "\n#pragma once\n"); }

    if global_args.lang == "jai" {
        if meta_info.extra_loads.count > 0 {
            loads_builder: String_Builder;
            for load : meta_info.extra_loads {
                print_to_builder(*loads_builder, "#load %;\n", load);
            }
            print_to_builder(*builder, "\n%", builder_to_string(*loads_builder));
        }
    } else {
        if meta_info.extra_includes.count > 0 {
            includes_builder: String_Builder;
            for include : meta_info.extra_includes {
                print_to_builder(*includes_builder, "#include %\n", include);
            }
            print_to_builder(*builder, "\n%", builder_to_string(*includes_builder));
        }
    }

    if global_args.lang == "cpp" {
        if (meta_info.namespace.count > 0) {
            print_to_builder(*builder, "\nnamespace %\n{\n", meta_info.namespace);
        }
    }

    context.temporary_storage = ts;
    return builder_to_string(*builder);
}

end_gen_code :: (meta_info: Meta_Info) -> string {
    ts := context.temporary_storage;
    start_scratch();
    builder: String_Builder;

    if global_args.lang == "cpp" {
        if (meta_info.namespace.count > 0) {
            print_to_builder(*builder, "} // namespace %\n\n", meta_info.namespace);
        }
    }
    context.temporary_storage = ts;
    return builder_to_string(*builder);
}

Meta_Info :: struct {
    namespace := "";
    impedance_updaters : [..] string;
    extra_includes : [..] string;
    extra_loads : [..] string;
}

parse_meta_info :: (meta_info_str: string) -> Meta_Info {
    meta_info : Meta_Info;

    ts := context.temporary_storage;
    start_scratch();

    info_lines := String.split(meta_info_str, cast(u8) #char "\n");
    for line : info_lines {
        line_trim := String.trim(line);
        was_split, key, value := String.split_from_left(line_trim, cast(u8) #char ":");
        if was_split == false { continue; }

        key = String.trim(key);
        value = String.trim(value);

        if key == {
            case "namespace";
                meta_info.namespace = value;
            case "calc_impedances";
                array_add(*meta_info.impedance_updaters, value);
            case "include";
                array_add(*meta_info.extra_includes, value);
            case "load";
                array_add(*meta_info.extra_loads, value);
        }
    }

    context.temporary_storage = ts;
    verbose_print("Meta info: %\n", meta_info);
    return meta_info;
}

generate_struct :: (impedance_vars: [] string, name: string) -> string {
    ts := context.temporary_storage;
    start_scratch();
    builder: String_Builder;

    if global_args.lang == "jai" {
        print_to_builder(*builder, "% :: struct {\n", name);
    } else {
        print_to_builder(*builder, "struct % {\n", name);
    }
    for var : impedance_vars {
        print_to_builder(*builder, "    %\n", var);
    }
    if global_args.lang == "jai" {
        print_to_builder(*builder, "}\n");
    } else {
        print_to_builder(*builder, "};\n");
    }

    context.temporary_storage = ts;
    return builder_to_string(*builder);
}

Method_Arg :: struct {
    name: string;
    type: string; // @TODO: should this be enum?
    default_value: string;
    is_reference: bool;
    is_const: bool;
}

get_return_type :: (return_types : [] string) -> string {
    return_type := "void";
    for type, it : return_types {
        if it == 0 {
            return_type = type;
            continue;
        }
        return_type = tprint("%1, %2", return_type, type);
    }

    if global_args.lang == "cpp" && return_types.count > 1 {
        return_type = tprint("std::tuple<%>", return_type);
    }

    return return_type;
}

generate_method :: (name: string,
                    args: [] Method_Arg,
                    lines: [] string,
                    return_types : [] string = .[]) -> string {
    ts := context.temporary_storage;
    start_scratch();
    builder: String_Builder;

    return_type := get_return_type(return_types);

    if global_args.lang == "jai" {
        print_to_builder(*builder, "% :: (", name);
    } else {
        print_to_builder(*builder, "inline %1 %2(", return_type, name); // @TODO: do we always want these methods to be marked inline?
    }
    for arg, idx : args {
        if idx != 0 { print_to_builder(*builder, ", "); }
        if global_args.lang == "jai" {
            print_to_builder(*builder, "%: ", arg.name);
            if arg.is_reference { print_to_builder(*builder, "*"); }
            print_to_builder(*builder, "%", arg.type);
            if arg.default_value.count > 0 { print_to_builder(*builder, " = %", arg.default_value); }
        } else {
            if arg.is_const { print_to_builder(*builder, "const "); }
            print_to_builder(*builder, "%", arg.type);
            if arg.is_reference { print_to_builder(*builder, "&"); }
            print_to_builder(*builder, " %", arg.name);
            if arg.default_value.count > 0 { print_to_builder(*builder, " = %", arg.default_value); }
        }
    }

    if global_args.lang == "jai" {
        if return_type == "void" {
            print_to_builder(*builder, ") {\n");
        } else {
            print_to_builder(*builder, ") -> % {\n", return_type);
        }
    } else {
        print_to_builder(*builder, ") {\n");
    }

    for line : lines {
        print_to_builder(*builder, "    %\n", line);
    }
    print_to_builder(*builder, "}\n");

    context.temporary_storage = ts;
    return builder_to_string(*builder);
}

generate_custom_process_root :: (token: Token, tokens: [] Token, down_tree_statements : *[..] string) {
    num_children := token.children.count;
    has_state := token.custom_info.state_type.count > 0;
    has_vars := token.custom_info.vars_type.count > 0;
    if num_children == 1 {
        child_token := tokens[token.children[0]];
        args := "";
        if has_vars {
            args = tprint(ifx global_args.lang == "jai" then "%1*impedances.%2_vars, "
                                                        else "%1impedances.%2_vars, ",
                          args,
                          token.name);
        }
        if has_state {
            args = tprint(ifx global_args.lang == "jai" then "%1*state.%2_state, "
                                                        else "%1state.%2_state, ",
                          args,
                          token.name);
        }
        array_add(down_tree_statements, tprint(ifx global_args.lang == "jai" then "%2_a := %4(%3%2_b); // %2 incident"
                                                                             else "const auto %2_a = %4(%3%2_b); // %2 incident",
                                               token.name,
                                               child_token.name,
                                               args,
                                               token.custom_info.compute_name));
    } else {
        if global_args.lang == "jai" {
            // declare inputs
            statement := tprint("%1_ins := %2.[", token.name, global_args.dtype);
            for child_idx : token.children {
                statement = tprint("%1%2_b, ", statement, tokens[child_idx].name);
            }
            statement.count -= 2;
            statement = tprint("%1] #align %3; // %2 inputs\n", statement, token.name, global_args.align);

            // declare outputs
            statement = tprint("%1    %2_outs: [%3] %4 = --- #align %5; // %2 outputs \n",
                               statement,
                               token.name,
                               num_children,
                               global_args.dtype,
                               global_args.align);

            args := "";
            if has_vars { args = tprint("%1*impedances.%2_vars, ", args, token.name); }
            if has_state { args = tprint("%1*state.%2_state, ", args, token.name); }
            args = tprint("%1%2_ins.data, %2_outs.data", args, token.name);
            statement = tprint("%1    %2(%3); // %4 compute\n", statement, token.custom_info.compute_name, args, token.name);

            // unwrap outputs
            for child_idx : token.children {
                statement = tprint("%1    %2_a := %3_outs[%4];\n", statement, tokens[child_idx].name, token.name, it_index);
            }
            statement.count -= 1;
            array_add(down_tree_statements, statement);
        } else {
            // declare inputs
            statement := tprint("alignas (%4) const %3 %1_ins[%2] { ",
                                token.name,
                                num_children,
                                global_args.dtype,
                                global_args.align);
            for child_idx : token.children {
                statement = tprint("%1%2_b, ", statement, tokens[child_idx].name);
            }
            statement.count -= 2;
            statement = tprint("%1 }; // % inputs\n", statement, token.name);

            // declare outputs
            statement = tprint("%    alignas (%5) %4 %2_outs[%3]; // %2 outputs \n",
                               statement,
                               token.name,
                               num_children,
                               global_args.dtype,
                               global_args.align);

            args := "";
            if has_vars { args = tprint("%1impedances.%2_vars, ", args, token.name); }
            if has_state { args = tprint("%1state.%2_state, ", args, token.name); }
            args = tprint("%1%2_ins, %2_outs", args, token.name);
            statement = tprint("%1    %2(%3); // %4 compute\n", statement, token.custom_info.compute_name, args, token.name);

            // unwrap outputs
            for child_idx : token.children {
                statement = tprint("%1    const auto %2_a = %3_outs[%4];\n", statement, tokens[child_idx].name, token.name, it_index);
            }
            statement.count -= 1;
            array_add(down_tree_statements, statement);
        }
    }
}

generate_custom_process :: (token: Token, tokens: [] Token, down_tree_statements : *[..] string, up_tree_statements : *[..] string) {
    num_children := token.children.count;
    has_state := token.custom_info.state_type.count > 0;
    has_vars := token.custom_info.vars_type.count > 0;

    if num_children == 0 {
        up_args := "";
        up_statement := "";
        down_args := "";
        if global_args.lang == "jai" {
            if has_vars {
                up_args = tprint("%1*impedances.%2_vars, ", up_args, token.name);
                down_args = tprint("%1*impedances.%2_vars, ", down_args, token.name);
            }
            if has_state {
                up_args = tprint("%*state.%2_state, ", up_args, token.name);
                down_args = tprint("%*state.%2_state, ", down_args, token.name);
            }
            up_statement = tprint("%1_b := ", token.name);
        } else {
            if has_vars {
                up_args = tprint("%1impedances.%2_vars, ", up_args, token.name);
                down_args = tprint("%1impedances.%2_vars, ", down_args, token.name);
            }
            if has_state {
                up_args = tprint("%state.%2_state, ", up_args, token.name);
                down_args = tprint("%state.%2_state, ", down_args, token.name);
            }
            up_statement = tprint("const auto %1_b = ", token.name);
        }
        if up_args.count > 0 { up_args.count -= 2; }
        up_statement = tprint("%1 %2(%3); // %4 reflected",
                              up_statement,
                              token.custom_info.reflected_name,
                              up_args,
                              token.name);
        array_add(up_tree_statements, up_statement);

        down_args = tprint("%1%2_a", down_args, token.name);
        down_statement := tprint("%1(%2); // %3 incident",
                                 token.custom_info.incident_name,
                                 down_args,
                                 token.name);
        array_add(down_tree_statements, down_statement);
    } else if num_children == 1 {
        array_add(up_tree_statements, tprint("// %1 reflected", token.name));
        array_add(down_tree_statements, tprint("// %1 incident", token.name));
        error_assert (false, "NOT IMPLEMENTED (generate process for custom non-root node with exactly 1 child");
    } else {
        if global_args.lang == "jai" {
            // declare inputs
            up_statement := tprint("%1_ins := %2.[", token.name, global_args.dtype);
            for child_idx : token.children {
                up_statement = tprint("%1%2_b, ", up_statement, tokens[child_idx].name);
            }
            up_statement.count -= 2;
            up_statement = tprint("%1] #align %3; // %2 inputs\n", up_statement, token.name, global_args.align);

            // finish up statement
            up_args := "";
            if has_vars { up_args = tprint("%1*impedances.%2_vars, ", up_args, token.name); }
            if has_state { up_args = tprint("%*state.%2_state, ", up_args, token.name); }
            up_args = tprint("%1%2_ins.data", up_args, token.name);
            up_statement = tprint("%1    %4_b := %2(%3); // %4 reflected", up_statement, token.custom_info.reflected_name, up_args, token.name);
            array_add(up_tree_statements, up_statement);

            // declare outputs
            down_statement := tprint("%1_outs: [%2] %3 = --- #align %4; // %2 outputs \n",
                                     token.name,
                                     num_children,
                                     global_args.dtype,
                                     global_args.align);

            down_args := "";
            if has_vars { down_args = tprint("%1*impedances.%2_vars, ", down_args, token.name); }
            if has_state { down_args = tprint("%*state.%2_state, ", down_args, token.name); }
            down_args = tprint("%1%2_a, %2_ins.data, %2_outs.data", down_args, token.name);
            down_statement = tprint("%1    %2(%3); // %4 compute\n", down_statement, token.custom_info.incident_name, down_args, token.name);

            // unwrap outputs
            for child_idx : token.children {
                down_statement = tprint("%1    %2_a := %3_outs[%4];\n", down_statement, tokens[child_idx].name, token.name, it_index);
            }
            down_statement.count -= 1;
            array_add(down_tree_statements, down_statement);
        } else {
            // declare inputs
            up_statement := tprint("alignas (%4) const %3 %1_ins[%2] { ",
                                   token.name,
                                   num_children,
                                   global_args.dtype,
                                   global_args.align);
            for child_idx : token.children {
                up_statement = tprint("%1%2_b, ", up_statement, tokens[child_idx].name);
            }
            up_statement.count -= 2;
            up_statement = tprint("%1 }; // % inputs\n", up_statement, token.name);

            // finish up statement
            up_args := "";
            if has_vars { up_args = tprint("%1impedances.%2_vars, ", up_args, token.name); }
            if has_state { up_args = tprint("%state.%2_state, ", up_args, token.name); }
            up_args = tprint("%1%2_ins", up_args, token.name);
            up_statement = tprint("%1    const auto %4_b = %2(%3); // %4 reflected", up_statement, token.custom_info.reflected_name, up_args, token.name);
            array_add(up_tree_statements, up_statement);

            // declare outputs
            down_statement := tprint("alignas (%4) %3 %1_outs[%2]; // %2 outputs \n",
                                     token.name,
                                     num_children,
                                     global_args.dtype,
                                     global_args.align);

            down_args := "";
            if has_vars { down_args = tprint("%1impedances.%2_vars, ", down_args, token.name); }
            if has_state { down_args = tprint("%state.%2_state, ", down_args, token.name); }
            down_args = tprint("%1%2_a, %2_ins, %2_outs", down_args, token.name);
            down_statement = tprint("%1    %2(%3); // %4 compute\n", down_statement, token.custom_info.incident_name, down_args, token.name);

            // unwrap outputs
            for child_idx : token.children {
                down_statement = tprint("%1    const auto %2_a = %3_outs[%4];\n", down_statement, tokens[child_idx].name, token.name, it_index);
            }
            down_statement.count -= 1;
            array_add(down_tree_statements, down_statement);
        }
    }
}

generate_process :: (tokens: [] Token, inputs: [] string, outputs: [] string) -> string, [..] string {
    ts := context.temporary_storage;
    start_scratch();

    input_args : [..] Method_Arg;
    array_add (*input_args, Method_Arg.{name="state", type="State", is_reference=true});
    array_add (*input_args, Method_Arg.{name="impedances", type="Impedances", is_reference=true, is_const=true});
    required_impedances : [..] string;
    required_impedances.allocator = .{ts_allocator_proc, ts};

    for input : inputs {
        input_token : *Token;
        for *token : tokens {
            if token.name == input {
                input_token = token;
                break;
            }
        }
        error_assert(input_token != null, tprint("Unable to find input circuit element: %", input));
        array_add(*input_args, Method_Arg.{name=input, type=global_args.dtype});

        if input_token.type == {
            case .Ideal_Voltage_Source; #through;
            case .Ideal_Current_Source; #through;
            case .Resistive_Voltage_Source; #through;
            case .Capacitive_Voltage_Source; #through;
            case .Resistive_Capacitive_Voltage_Source; #through;
            case .Root_Resistive_Voltage_Source; #through;
            case .Root_Capacitive_Voltage_Source; #through;
            case .Resistive_Current_Source;
                input_token.input = input;
            case;
                error_assert(false, tprint("Input element % has invalid type: %", input, input_token.type));
        }
    }

    write_define :: (name: string, value: string, comment := "") -> string {
        str := "";
        if global_args.lang == "jai" {
            str = tprint("%1 := %2;", name, value);
        } else {
            str = tprint("const auto %1 = %2;", name, value);
        }

        if comment.count > 0 { str = tprint("%1 // %2", str, comment); }

        return str;
    }

    down_tree_statements : [..] string;
    up_tree_statements : [..] string;
    root_token := *tokens[0];
    for *token : tokens {
        // print("%\n", token.*);

        if token == root_token {
            if token.type == {
                case .Ideal_Voltage_Source;
                    if ! is_input(token, inputs) { token.input = tprint("impedances.%1_V", token.name); }
                    child_token := tokens[token.children[0]];
                    array_add(*up_tree_statements, write_define(tprint("%_a", token.name),
                                                                tprint("%_b", child_token.name),
                                                                tprint("% incident", token.name)));
                    array_add(*down_tree_statements, write_define(tprint("%_b", token.name),
                                                                  tprint("-%1_a + 2 * %2", token.name, token.input),
                                                                  tprint("%1 reflected", token.name)));
                    array_add(*down_tree_statements, write_define(tprint("%_a", child_token.name),
                                                                  tprint("%_b", token.name),
                                                                  tprint("% incident", child_token.name)));
                case .Root_Resistive_Voltage_Source;
                    if ! is_input(token, inputs) { token.input = tprint("impedances.%1_V", token.name); }
                    input := ifx token.invert then tprint("-(%)", token.input) else token.input;
                    child_token := tokens[token.children[0]];
                    array_add(*up_tree_statements, write_define(tprint("%_a", token.name),
                                                                tprint("%_b", child_token.name),
                                                                tprint("% incident", token.name)));
                    array_add(*down_tree_statements, write_define(tprint("%_b", token.name),
                                                                  tprint("impedances.%1_ca * %1_a + impedances.%1_ce * %2", token.name, input),
                                                                  tprint("%1 reflected", token.name)));
                    array_add(*down_tree_statements, write_define(tprint("%_a", child_token.name),
                                                                  tprint("%_b", token.name),
                                                                  tprint("% incident", child_token.name)));
                    array_add(*required_impedances, tprint("%_ca", token.name));
                    array_add(*required_impedances, tprint("%_ce", token.name));
                case .Root_Capacitive_Voltage_Source;
                    if ! is_input(token, inputs) { token.input = tprint("impedances.%1_V", token.name); }
                    input := ifx token.invert then tprint("-(%)", token.input) else token.input;
                    child_token := tokens[token.children[0]];
                    array_add(*up_tree_statements, write_define(tprint("%_a", token.name),
                                                                tprint("%_b", child_token.name),
                                                                tprint("% incident", token.name)));
                    array_add(*down_tree_statements, write_define(tprint("%_a0a1", token.name),
                                                                  tprint("2 * (%2 + state.%1_z) + %1_a", token.name, input),
                                                                  tprint("%1 reflected", token.name)));
                    array_add(*down_tree_statements, write_define(tprint("%_b", token.name),
                                                                  tprint("impedances.%1_ca * %1_a - impedances.%1_ce * %1_a0a1", token.name),
                                                                  tprint("%1 reflected", token.name)));
                    array_add(*down_tree_statements, tprint("state.%1_z += -%1_b - %1_a0a1; // %1_z state update", token.name));
                    array_add(*down_tree_statements, write_define(tprint("%_a", child_token.name),
                                                                  tprint("%_b", token.name),
                                                                  tprint("% incident", child_token.name)));
                    array_add(*required_impedances, tprint("%_ca", token.name));
                    array_add(*required_impedances, tprint("%_ce", token.name));
                case .Ideal_Current_Source;
                    if ! is_input(token, inputs) { token.input = tprint("impedances.%1_I", token.name); }
                    child_token := tokens[token.children[0]];
                    array_add(*up_tree_statements, write_define(tprint("%_a", token.name),
                                                                tprint("%_b", child_token.name),
                                                                tprint("% incident", token.name)));
                    array_add(*down_tree_statements, write_define(tprint("%_b", token.name),
                                                                  tprint("2 * impedances.%3_R * %2 + %1_a", token.name, token.input, child_token.name),
                                                                  tprint("%1 reflected (@TODO: 2 * R?)", token.name)));
                    array_add(*down_tree_statements, write_define(tprint("%_a", child_token.name),
                                                                  tprint("%_b", token.name),
                                                                  tprint("% incident", child_token.name)));
                    array_add(*required_impedances, tprint("%_R", child_token.name));
                case .Custom;
                    generate_custom_process_root(token, tokens, *down_tree_statements);
            }
            continue;
        }

        if token.type == {
            case .Resistor;
                array_add(*up_tree_statements, write_define(tprint("%_b", token.name),
                                                            ifx global_args.lang == "jai" then tprint("cast(%) 0", global_args.dtype) else "0",
                                                            tprint("% reflected", token.name)));
            case .Capacitor;
                array_add(*up_tree_statements, write_define(tprint("%_b", token.name),
                                                            tprint("state.%_z", token.name),
                                                            tprint("% reflected", token.name)));
                array_add(*down_tree_statements, tprint("state.%1_z = %1_a; // %1 state update", token.name));
            case .Capacitor_Alpha;
                array_add(*up_tree_statements, tprint("state.%1_b1 = %1_b; // %1 state update", token.name));
                array_add(*up_tree_statements, write_define(tprint("%_b", token.name),
                                                            tprint("impedances.%1_b_coeff * state.%1_b1 + impedances.%1_a_coeff * state.%1_z", token.name),
                                                            tprint("% reflected", token.name)));
                array_add(*down_tree_statements, tprint("state.%1_z = %1_a; // %1 state update", token.name));
            case .Inductor;
                array_add(*up_tree_statements, write_define(tprint("%_b", token.name),
                                                            tprint("-state.%_z", token.name),
                                                            tprint("% reflected", token.name)));
                array_add(*down_tree_statements, tprint("state.%1_z = %1_a; // %1 state update", token.name));
            case .Resistor_Capacitor_Series;
                array_add(*up_tree_statements, write_define(tprint("%_b", token.name),
                                                            tprint("-state.%_z", token.name),
                                                            tprint("% reflected", token.name)));
                array_add(*down_tree_statements, tprint("state.%1_z = -%1_a + impedances.%1_2RC_over_T_plus_2RC * (%1_a + state.%1_z); // %1 state update", token.name));
                array_add(*required_impedances, tprint("%_2RC_over_T_plus_2RC", token.name));
            case .Resistor_Capacitor_Parallel;
                array_add(*up_tree_statements, write_define(tprint("%_b", token.name),
                                                            tprint("impedances.%1_2RC_over_2RC_plus_T * state.%1_z", token.name),
                                                            tprint("% reflected", token.name)));
                array_add(*down_tree_statements, tprint("state.%1_z = %1_b + %1_a - state.%1_z; // %1 state update", token.name));
                array_add(*required_impedances, tprint("%_2RC_over_2RC_plus_T", token.name));
            case .Resistive_Voltage_Source;
                if ! is_input(token, inputs) { token.input = tprint("impedances.%1_V", token.name); }
                input := ifx token.invert then tprint("-(%)", token.input) else token.input;
                array_add(*up_tree_statements, write_define(tprint("%_b", token.name),
                                                            input,
                                                            tprint("% reflected", token.name)));
            case .Resistive_Current_Source;
                if ! is_input(token, inputs) { token.input = tprint("impedances.%1_I", token.name); }
                array_add(*up_tree_statements, write_define(tprint("%_b", token.name),
                                                            tprint("impedances.%1_R * %2", token.name, token.input),
                                                            tprint("% reflected", token.name)));
                array_add(*required_impedances, tprint("%_R", token.name));
            case .Capacitive_Voltage_Source;
                if ! is_input(token, inputs) { token.input = tprint("impedances.%1_V", token.name); }
                input := ifx token.invert then tprint("-(%)", token.input) else token.input;
                array_add(*up_tree_statements, tprint("state.%1_v1 = %2; // %1 state update", token.name, input));
                array_add(*up_tree_statements, write_define(tprint("%_b", token.name),
                                                            tprint("state.%1_z + %2 - state.%1_v1", token.name, input),
                                                            tprint("% reflected", token.name)));
                array_add(*down_tree_statements, tprint("state.%1_z = %1_a; // %1 state update", token.name));
            case .Resistive_Capacitive_Voltage_Source;
                if ! is_input(token, inputs) { token.input = tprint("impedances.%1_V", token.name); }
                input := ifx token.invert then tprint("-(%)", token.input) else token.input;
                array_add(*up_tree_statements, write_define(tprint("%_b", token.name),
                                                            tprint("-(state.%_z + %)", token.name, input),
                                                            tprint("% reflected", token.name)));
                array_add(*down_tree_statements, tprint("state.%1_z -= impedances.%1_T_over_2RC * (%1_a - %1_b); // %1 state update", token.name));
                array_add(*required_impedances, tprint("%_T_over_2RC", token.name));
            case .Series;
                child0_token := tokens[token.children[0]];
                child1_token := tokens[token.children[1]];
                array_add(*up_tree_statements, write_define(tprint("%_b", token.name),
                                                            tprint("-(%1_b + %2_b)", child0_token.name, child1_token.name),
                                                            tprint("% reflected", token.name)));
                array_add(*down_tree_statements, write_define(tprint("%_a", child0_token.name),
                                                              tprint("%2_b - impedances.%1_pr * (%1_a - %1_b)", token.name, child0_token.name),
                                                              tprint("% incident", child0_token.name)));
                if global_args.one_multiply {
                    array_add(*down_tree_statements, write_define(tprint("%_a", child1_token.name),
                                                                tprint("-(%1_a + %2_a)", token.name, child0_token.name),
                                                                tprint("% incident", child1_token.name)));
                } else {
                    array_add(*down_tree_statements, write_define(tprint("%_a", child1_token.name),
                                                                  tprint("-%1_a - %2_b + impedances.%1_pr * (%1_a - %1_b)", token.name, child0_token.name),
                                                                  tprint("% incident", child1_token.name)));
                }
                array_add(*required_impedances, tprint("%_pr", token.name));
            case .Parallel;
                child0_token := tokens[token.children[0]];
                child1_token := tokens[token.children[1]];
                array_add(*up_tree_statements, write_define(tprint("%_b", token.name),
                                                            tprint("%3_b - impedances.%1_pr * (%3_b - %2_b)", token.name, child0_token.name, child1_token.name),
                                                            tprint("% reflected", token.name)));
                array_add(*down_tree_statements, write_define(tprint("%_a", child0_token.name),
                                                              tprint("%1_b - %2_b + %1_a", token.name, child0_token.name),
                                                              tprint("% incident", child0_token.name)));
                array_add(*down_tree_statements, write_define(tprint("%_a", child1_token.name),
                                                              tprint("%1_b - %2_b + %1_a", token.name, child1_token.name),
                                                              tprint("% incident", child1_token.name)));
                array_add(*required_impedances, tprint("%_pr", token.name));
            case .Inverter;
                child_token := tokens[token.children[0]];
                array_add(*up_tree_statements, write_define(tprint("%_b", token.name),
                                                            tprint("-%_b", child_token.name),
                                                            tprint("% reflected", token.name)));
                array_add(*down_tree_statements, write_define(tprint("%_a", child_token.name),
                                                              tprint("-%_a", token.name),
                                                              tprint("% incident", child_token.name)));
            case .Custom;
                generate_custom_process(token, tokens, *down_tree_statements, *up_tree_statements);
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

    return_vars : [..] string;
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
        error_assert(output_token != null, tprint("Unable to find output circuit element: %", output_info[1]));

        half_literal := tprint(ifx global_args.lang == "jai" then "cast(%) 0.5" else "(%) 0.5", global_args.dtype);
        if output_info[0] == {
            case "V";
                return_var := tprint("v_%", output_token.name);
                array_add(*code_statements, write_define(return_var,
                                                         tprint("(%1_a + %1_b) * %2", output_token.name, half_literal),
                                                         tprint("% voltage", output_token.name)));
                array_add(*return_vars, return_var);
            case "I";
                return_var := tprint("i_%", output_token.name);
                array_add(*code_statements, write_define(return_var,
                                                         tprint("(%1_a - %1_b) * (%2 * impedances.%1_G)", output_token.name, half_literal),
                                                         tprint("% current", output_token.name)));
                array_add(*return_vars, return_var);
                array_add(*required_impedances, tprint("%_G", output_token.name));
            case;
                error_assert(false, tprint("Invalid output type: %", output_info[0]));
        }
    }

    return_types : [..] string;
    if return_vars.count == 1 {
        array_add(*code_statements, tprint("return %;", return_vars[0]));
        array_add(*return_types, global_args.dtype);
    } else {
        return_statement := ifx global_args.lang == "jai" then "return "
                                                          else "return std::make_tuple(";
        for var : return_vars {
            return_statement = tprint("%1%2, ", return_statement, var);
            array_add(*return_types, global_args.dtype);
        }
        return_statement.count -= 2;
        return_statement = tprint(ifx global_args.lang == "jai" then "%;" else "%);",
                                  return_statement);
        array_add(*code_statements, return_statement);
    }

    method_code := generate_method("process",
                                   input_args,
                                   code_statements,
                                   return_types);
    context.temporary_storage = ts;
    return copy_string(method_code), required_impedances;
}

add_impedance_update_todo :: (todos: *[..] *Token, token: *Token, new_child: *Token, append := true) {
    for todo : todos.* {
        if todo == token { return; }
    }
    if append { array_add(todos, token); }
    else { array_insert_at(todos, token, 0); }
}

is_input :: (token: *Token, inputs: [] string) -> bool {
    found, idx := array_find(inputs, token.name);
    return found;
}

// returns true if the param value is defined in the config (i.e. known at compile-time)
is_param_defined :: (token: *Token, idx := 0) -> bool {
    is_defined_arg := idx < token.args.count;
    if is_defined_arg { is_defined_arg &= token.args[idx] != "?"; }
    return is_defined_arg;
}

write_constexpr_var :: (name: string, value: string) -> string {
    if global_args.lang == {
        case "cpp";
            dtype := global_args.dtype;
            if dtype == "float" || dtype == "double" {
                return tprint("static constexpr %3 %1 = %2;", name, value, global_args.dtype);
            } else {
                return tprint("const %3 %1 = %2;", name, value, global_args.dtype);
            }
        case "jai";
            return tprint("%1 : %3 : %2;", name, value, global_args.dtype);
    }
    return .{};
}

write_struct_field :: (name: string) -> string {
    if global_args.lang == {
        case "cpp";
            return tprint("%2 %1;", name, global_args.dtype);
        case "jai";
            return tprint("%1 : %2;", name, global_args.dtype);
    }
    return .{};
}

write_param_named :: (params_vars : *[..] string, token: *Token, name: string, idx := 0) -> bool {
    is_defined_arg := is_param_defined(token, idx);
    if is_defined_arg {
        array_add(params_vars,
                  write_constexpr_var(name,
                                      tprint("%", get_value(token.args[idx]))));
    } else {
        array_add(params_vars, write_struct_field(name));
    }

    return is_defined_arg;
}

write_param :: (params_vars : *[..] string, token: *Token, idx := 0, suffix := "value") -> bool {
    return write_param_named(params_vars, token, tprint("%1_%2", token.name, suffix), idx);
}

get_constexpr_var :: (prefix: string, name: string) -> string {
    if global_args.lang == {
        case "cpp";
            return tprint("%1::%2", prefix, name);
        case "jai";
            return tprint("%1.%2", prefix, name);
    }
    return "";
}

Circuit_Vars :: struct {
    state_vars : [..] string;
    impedance_vars : [..] string;
    params_vars : [..] string;
}

generate_circuit_vars :: (tokens: [] Token, inputs: [] string, required_impedances: [] string) -> Circuit_Vars {
    state_vars : [..] string;
    impedance_vars : [..] string;
    params_vars : [..] string;

    impedance_update_todos : [..] *Token;
    root_token := *tokens[0];
    for *token : tokens {
        if token != root_token {
            r_found, r_idx := array_find(required_impedances, tprint("%_R", token.name));
            if r_found { array_add(*impedance_vars, write_struct_field(tprint("%_R", token.name))); }

            g_found, g_idx := array_find(required_impedances, tprint("%_G", token.name));
            if g_found { array_add(*impedance_vars, write_struct_field(tprint("%_G", token.name))); }
        }

        if token.type == {
            case .Resistor;
                for name,idx : token.param_names { write_param_named(*params_vars, token, name, idx); }
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Capacitor;
                for name,idx : token.param_names { write_param_named(*params_vars, token, name, idx); }
                array_add(*state_vars, write_struct_field(tprint("%_z", token.name)));
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Capacitor_Alpha;
                write_param(*params_vars, token, 0);
                write_param(*params_vars, token, 1, "alpha");
                array_add(*state_vars, write_struct_field(tprint("%_z", token.name)));
                array_add(*state_vars, write_struct_field(tprint("%_b1", token.name)));
                array_add(*impedance_vars, write_struct_field(tprint("%1_b_coeff", token.name)));
                array_add(*impedance_vars, write_struct_field(tprint("%1_a_coeff", token.name)));
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Inductor;
                write_param_named(*params_vars, token, token.param_names[0]);
                array_add(*state_vars, write_struct_field(tprint("%_z", token.name)));
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Resistor_Capacitor_Series;
                for name,idx : token.param_names { write_param_named(*params_vars, token, name, idx); }
                array_add(*state_vars, write_struct_field(tprint("%_z", token.name)));
                array_add(*impedance_vars, write_struct_field(tprint("%1_2RC_over_T_plus_2RC", token.name)));
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Resistor_Capacitor_Parallel;
                for name,idx : token.param_names { write_param_named(*params_vars, token, name, idx); }
                array_add(*state_vars, write_struct_field(tprint("%_z", token.name)));
                array_add(*impedance_vars, write_struct_field(tprint("%1_2RC_over_2RC_plus_T", token.name)));
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Ideal_Voltage_Source;
                if ! is_input(token, inputs) {
                    is_known := write_param(*params_vars, token, 0, "v_value");
                    if is_known {
                        array_add(*impedance_vars, write_constexpr_var(tprint("%_V", token.name),
                                                                       get_constexpr_var("Params", tprint("%1_v_value", token.name))));
                    } else {
                        array_add(*impedance_vars, write_struct_field(tprint("%_V", token.name)));
                    }
                }

            case .Ideal_Current_Source;
                if ! is_input(token, inputs) {
                    is_known := write_param(*params_vars, token, 0, "i_value");
                    if is_known {
                        array_add(*impedance_vars, write_constexpr_var(tprint("%_I", token.name),
                                                                       get_constexpr_var("Params", tprint("%1_i_value", token.name))));
                    } else {
                        array_add(*impedance_vars, write_struct_field(tprint("%_I", token.name)));
                    }
                }

            case .Resistive_Voltage_Source;
                is_input_source := is_input(token, inputs);
                if ! is_input_source {
                    is_known := write_param(*params_vars, token, 0, "v_value");
                    if is_known {
                        array_add(*impedance_vars, write_constexpr_var(tprint("%_V", token.name),
                                                                       get_constexpr_var("Params", tprint("%1_v_value", token.name))));
                    } else {
                        array_add(*impedance_vars, write_struct_field(tprint("%_V", token.name)));
                    }
                }

                for name,idx : token.param_names {
                    if idx == 0 { continue; } // @TODO: handle this if we allow combined inputs
                    write_param_named(*params_vars, token, name, idx);
                }
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Root_Resistive_Voltage_Source;
                is_input_source := is_input(token, inputs);
                if ! is_input_source {
                    is_known := write_param(*params_vars, token, 0, "v_value");
                    if is_known {
                        array_add(*impedance_vars, write_constexpr_var(tprint("%_V", token.name),
                                                                       get_constexpr_var("Params", tprint("%1_v_value", token.name))));
                    } else {
                        array_add(*impedance_vars, write_struct_field(tprint("%_V", token.name)));
                    }
                }

                for name,idx : token.param_names {
                    if idx == 0 { continue; } // @TODO: handle this if we allow combined inputs
                    write_param_named(*params_vars, token, name, idx);
                }

                array_add(*impedance_vars, write_struct_field(tprint("%1_ca", token.name)));
                array_add(*impedance_vars, write_struct_field(tprint("%1_ce", token.name)));

            case .Capacitive_Voltage_Source;
                is_input_source := is_input(token, inputs);
                if ! is_input_source {
                    is_known := write_param(*params_vars, token, 0, "v_value");
                    if is_known {
                        array_add(*impedance_vars, write_constexpr_var(tprint("%_V", token.name),
                                                                       get_constexpr_var("Params", tprint("%1_v_value", token.name))));
                    } else {
                        array_add(*impedance_vars, write_struct_field(tprint("%_V", token.name)));
                    }
                }

                array_add(*state_vars, write_struct_field(tprint("%_z", token.name)));
                array_add(*state_vars, write_struct_field(tprint("%_v1", token.name)));
                for name,idx : token.param_names {
                    if idx == 0 { continue; } // @TODO: handle this if we allow combined inputs
                    write_param_named(*params_vars, token, name, idx);
                }
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Root_Capacitive_Voltage_Source;
                is_input_source := is_input(token, inputs);
                if ! is_input_source {
                    is_known := write_param(*params_vars, token, 0, "v_value");
                    if is_known {
                        array_add(*impedance_vars, write_constexpr_var(tprint("%_V", token.name),
                                                                       get_constexpr_var("Params", tprint("%1_v_value", token.name))));
                    } else {
                        array_add(*impedance_vars, write_struct_field(tprint("%_V", token.name)));
                    }
                }

                for name,idx : token.param_names {
                    if idx == 0 { continue; } // @TODO: handle this if we allow combined inputs
                    write_param_named(*params_vars, token, name, idx);
                }

                array_add(*impedance_vars, write_struct_field(tprint("%1_ca", token.name)));
                array_add(*impedance_vars, write_struct_field(tprint("%1_ce", token.name)));

                array_add(*state_vars, write_struct_field(tprint("%_z", token.name)));

            case .Resistive_Capacitive_Voltage_Source;
                is_input_source := is_input(token, inputs);
                if ! is_input_source {
                    is_known := write_param(*params_vars, token, 0, "v_value");
                    if is_known {
                        array_add(*impedance_vars, write_constexpr_var(tprint("%_V", token.name),
                                                                       get_constexpr_var("Params", tprint("%1_v_value", token.name))));
                    } else {
                        array_add(*impedance_vars, write_struct_field(tprint("%_V", token.name)));
                    }
                }

                array_add(*state_vars, write_struct_field(tprint("%_z", token.name)));
                array_add(*impedance_vars, write_struct_field(tprint("%1_T_over_2RC", token.name)));
                for name,idx : token.param_names {
                    if idx == 0 { continue; } // @TODO: handle this if we allow combined inputs
                    write_param_named(*params_vars, token, name, idx);
                }
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Resistive_Current_Source;
                is_input_source := is_input(token, inputs);
                if ! is_input(token, inputs) {
                    is_known := write_param(*params_vars, token, 0, "i_value");
                    if is_known {
                        array_add(*impedance_vars, write_constexpr_var(tprint("%_I", token.name),
                                                                       get_constexpr_var("Params", tprint("%1_i_value", token.name))));
                    } else {
                        array_add(*impedance_vars, write_struct_field(tprint("%_I", token.name)));
                    }
                }

                if is_input_source {
                    write_param(*params_vars, token, 0, "res_value");
                } else {
                    write_param(*params_vars, token, 1, "res_value");
                }
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Custom;
                if token.children.count > 0 { continue; }
                if (token.custom_info.params_type.count > 0) {
                    array_add(*params_vars,
                              tprint(ifx global_args.lang == "jai" then "%2_params: %1;" else "%1 %2_params {};",
                                     token.custom_info.params_type,
                                     token.name));
                }
                if (token.custom_info.state_type.count > 0) {
                    array_add(*state_vars,
                              tprint(ifx global_args.lang == "jai" then "%2_state: %1;" else "%1 %2_state {};",
                                     token.custom_info.state_type,
                                     token.name));
                }
                if (token.custom_info.vars_type.count > 0) {
                    array_add(*impedance_vars,
                              tprint(ifx global_args.lang == "jai" then "%2_vars: %1;" else "%1 %2_vars {};",
                                     token.custom_info.vars_type,
                                     token.name));
                }
        }
    }

    while impedance_update_todos.count > 0 {
        Sort.quick_sort (impedance_update_todos, x => x.tree_depth);
        impedance_todo := impedance_update_todos[impedance_update_todos.count - 1];
        token := impedance_todo;
        if token.type == {
            case .Series;
                array_add(*impedance_vars, write_struct_field(tprint("%1_pr", token.name)));
            case .Parallel;
                array_add(*impedance_vars, write_struct_field(tprint("%1_pr", token.name)));
            case .Custom;
                if (token.custom_info.params_type.count > 0) {
                    array_add(*params_vars,
                              tprint(ifx global_args.lang == "jai" then "%2_params: %1;" else "%1 %2_params {};",
                                     token.custom_info.params_type,
                                     token.name));
                }
                if (token.custom_info.state_type.count > 0) {
                    array_add(*state_vars,
                              tprint(ifx global_args.lang == "jai" then "%2_state: %1;" else "%1 %2_state {};",
                                     token.custom_info.state_type,
                                     token.name));
                }
                if (token.custom_info.vars_type.count > 0) {
                    array_add(*impedance_vars,
                              tprint(ifx global_args.lang == "jai" then "%2_vars: %1;" else "%1 %2_vars {};",
                                     token.custom_info.vars_type,
                                     token.name));
                }
        }

        // This is leaky, but I don't think we care?
        impedance_update_todos.count -= 1;

        if token != root_token {
            add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token, false);
        } else {
            // verbose_print("Finished climbing impedance tree!");
        }
    }

    return Circuit_Vars.{state_vars=state_vars, impedance_vars=impedance_vars, params_vars=params_vars};
}

generate_updater :: (tokens: [] Token,
                     inputs: [] string,
                     required_impedances: *[..] string,
                     tokens_filter: [] string = .[]) -> [..] string {
    write_define :: (name: string, value: string) -> string {
        if global_args.lang == {
            case "jai";
                return tprint("%1 := %2;", name, value);
            case "cpp";
                return tprint("[[maybe_unused]] const auto %1 = %2;", name, value);
        }
        return "";
    }

    add_impedance :: (updates: *[..] string, name: string, value: string, required: [] string, available_values: *[..] string) {
        array_add(updates, write_define(name, value));
        found, idx := array_find(required, name);
        if found {
            array_add(updates, tprint("impedances.%1 = %1;", name));
        }
        array_add(available_values, name);
    }

    needs_value :: (updates: *[..] string, name: string, available: [] string, required: *[..] string) {
        found, idx := array_find(available, name);
        if ! found {
            array_add(required, name);
            array_add(updates, write_define(name, tprint("impedances.%1;", name)));
        }
    }

    param_calc :: (token: Token, param_calc_info: Token.Param_Calc_Info) -> string {
        if (param_calc_info.param_name_idx >= 0) {
            return tprint("params.%1", token.param_names[param_calc_info.param_name_idx]);
        }

        left_calc := param_calc(token, param_calc_info.child_left);
        right_calc := param_calc(token, param_calc_info.child_right);

        if #complete param_calc_info.calc == {
            case .Sum;
                return tprint("sum(%1, %2)", left_calc, right_calc);
            case .Reciprocal_Sum;
                return tprint("recip_sum(%1, %2)", left_calc, right_calc);
        }
    }

    available_values : [..] string;
    impedance_updates : [..] string;
    root_impedance_updates : [..] string;
    impedance_update_todos : [..] *Token;

    if global_args.lang == {
        case "cpp";
            array_add(*impedance_updates, "[[maybe_unused]] static constexpr auto sum = [] (auto a, auto b) { return a + b; };");
            array_add(*impedance_updates, "[[maybe_unused]] static constexpr auto recip_sum = [] (auto a, auto b) { return (a * b) / (a + b); };");
        case "jai";
            array_add(*impedance_updates, "sum :: (a: $T, b: T) -> T { return a + b; }");
            array_add(*impedance_updates, "recip_sum :: (a: $T, b: T) -> T { return (a * b) / (a + b); }");
    }
    array_add(*impedance_updates, ""); // line break
    array_add(*impedance_updates, write_define("T", "1 / fs"));
    array_add(*impedance_updates, ""); // line break

    root_token := *tokens[0];
    for *token : tokens {
        if tokens_filter.count > 0 {
            token_found := false;
            for filter_token_name : tokens_filter {
                if (token.name == filter_token_name) {
                    token_found = true;
                    break;
                }
            }
            if ! token_found { continue; }
        }

        if token.type == {
            case .Resistor;
                array_add(*impedance_updates, tprint("// Computing impedance for: %1;", token.name));
                array_add(*impedance_updates, write_define(tprint("%1_value", token.name), param_calc(token, token.param_calc_info[0])));
                add_impedance(*impedance_updates, tprint("%1_R", token.name), tprint("%1_value", token.name), required_impedances.*, *available_values);
                add_impedance(*impedance_updates, tprint("%1_G", token.name), tprint("1 / %1_R", token.name), required_impedances.*, *available_values);
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Capacitor;
                array_add(*impedance_updates, tprint("// Computing impedance for: %1;", token.name));
                array_add(*impedance_updates, write_define(tprint("%1_value", token.name), param_calc(token, token.param_calc_info[0])));
                add_impedance(*impedance_updates, tprint("%1_G", token.name), tprint("2 * %1_value * fs", token.name), required_impedances.*, *available_values);
                add_impedance(*impedance_updates, tprint("%1_R", token.name), tprint("1 / %1_G", token.name), required_impedances.*, *available_values);
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Capacitor_Alpha;
                array_add(*impedance_updates, tprint("// Computing impedance for: %1;", token.name));
                array_add(*impedance_updates, tprint("impedances.%1_b_coeff = (1 - params.%1_alpha) * 0.5;", token.name));
                array_add(*impedance_updates, tprint("impedances.%1_a_coeff = (1 + params.%1_alpha) * 0.5;", token.name));
                add_impedance(*impedance_updates, tprint("%1_G", token.name), tprint("(1 + params.%1_alpha) * params.%1_value * fs", token.name), required_impedances.*, *available_values);
                add_impedance(*impedance_updates, tprint("%1_R", token.name), tprint("1 / %1_G", token.name), required_impedances.*, *available_values);
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Inductor;
                array_add(*impedance_updates, tprint("// Computing impedance for: %1;", token.name));
                array_add(*impedance_updates, write_define(tprint("%1_value", token.name), param_calc(token, token.param_calc_info[0])));
                add_impedance(*impedance_updates, tprint("%1_R", token.name), tprint("2 * %1_value * fs", token.name), required_impedances.*, *available_values);
                add_impedance(*impedance_updates, tprint("%1_G", token.name), tprint("1 / %1_R", token.name), required_impedances.*, *available_values);
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Resistor_Capacitor_Series;
                array_add(*impedance_updates, tprint("// Computing impedance for: %1;", token.name));
                array_add(*impedance_updates, write_define(tprint("%1_res_value", token.name), param_calc(token, token.param_calc_info[0])));
                array_add(*impedance_updates, write_define(tprint("%1_cap_value", token.name), param_calc(token, token.param_calc_info[1])));
                add_impedance(*impedance_updates, tprint("%1_R", token.name), tprint("(T / (2 * %1_cap_value)) + %1_res_value", token.name), required_impedances.*, *available_values);
                add_impedance(*impedance_updates, tprint("%1_G", token.name), tprint("1 / %1_R", token.name), required_impedances.*, *available_values);
                array_add(*impedance_updates, tprint("impedances.%1_2RC_over_T_plus_2RC = (2 * %1_cap_value * %1_res_value) / (2 * %1_cap_value * %1_res_value + T);\n", token.name));
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Resistor_Capacitor_Parallel;
                array_add(*impedance_updates, tprint("// Computing impedance for: %1;", token.name));
                array_add(*impedance_updates, write_define(tprint("%1_res_value", token.name), param_calc(token, token.param_calc_info[0])));
                array_add(*impedance_updates, write_define(tprint("%1_cap_value", token.name), param_calc(token, token.param_calc_info[1])));
                array_add(*impedance_updates, write_define(tprint("%1_2RC", token.name), tprint("2 * %1_cap_value * %1_res_value", token.name)));
                add_impedance(*impedance_updates, tprint("%1_R", token.name), tprint("%1_res_value * T / (%1_2RC + T)", token.name), required_impedances.*, *available_values);
                add_impedance(*impedance_updates, tprint("%1_G", token.name), tprint("1 / %1_R", token.name), required_impedances.*, *available_values);
                array_add(*impedance_updates, tprint("impedances.%1_2RC_over_2RC_plus_T = %1_2RC / (%1_2RC + T);\n", token.name));
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Ideal_Voltage_Source;
                if ! is_input(token, inputs) {
                    is_known := is_param_defined(token, 0);
                    if ! is_known {
                        array_add(*root_impedance_updates, tprint("// Computing voltage for: %1;", token.name));
                        array_add(*root_impedance_updates, tprint("impedances.%1_V = params.%1_v_value;\n", token.name));
                    }
                }

            case .Ideal_Current_Source;
                if ! is_input(token, inputs) {
                    is_known := is_param_defined(token, 0);
                    if ! is_known {
                        array_add(*root_impedance_updates, tprint("// Computing current for: %1;", token.name));
                        array_add(*root_impedance_updates, tprint("impedances.%1_I = params.%1_i_value;\n", token.name));
                    }
                }

            case .Resistive_Voltage_Source;
                if ! is_input(token, inputs) {
                    is_known := is_param_defined(token, 0);
                    if ! is_known {
                        array_add(*impedance_updates, tprint("// Computing voltage for: %1;", token.name));
                        array_add(*impedance_updates, tprint("impedances.%1_V = params.%1_v_value;\n", token.name));
                    }
                }

                array_add(*impedance_updates, tprint("// Computing impedance for: %1;", token.name));
                array_add(*impedance_updates, write_define(tprint("%1_res_value", token.name), param_calc(token, token.param_calc_info[1])));
                add_impedance(*impedance_updates, tprint("%1_R", token.name), tprint("%1_res_value", token.name), required_impedances.*, *available_values);
                add_impedance(*impedance_updates, tprint("%1_G", token.name), tprint("1 / %1_R", token.name), required_impedances.*, *available_values);
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Root_Resistive_Voltage_Source;
                if ! is_input(token, inputs) {
                    is_known := is_param_defined(token, 0);
                    if ! is_known {
                        array_add(*root_impedance_updates, tprint("// Computing voltage for: %1;", token.name));
                        array_add(*root_impedance_updates, tprint("impedances.%1_V = params.%1_v_value;\n", token.name));
                    }
                }

                child_token := tokens[token.children[0]];
                array_add(*root_impedance_updates, tprint("// Computing impedance for: %1;", token.name));
                array_add(*root_impedance_updates, write_define(tprint("%1_res_value", token.name), param_calc(token, token.param_calc_info[1])));
                array_add(*root_impedance_updates, tprint("impedances.%1_ca = (%1_res_value - %2_R) / (%1_res_value + %2_R);", token.name, child_token.name));
                array_add(*root_impedance_updates, tprint("impedances.%1_ce = (2 * %2_R) / (%1_res_value + %2_R);\n", token.name, child_token.name));

            case .Capacitive_Voltage_Source;
                if ! is_input(token, inputs) {
                    is_known := is_param_defined(token, 0);
                    if ! is_known {
                        array_add(*impedance_updates, tprint("// Computing voltage for: %1;", token.name));
                        array_add(*impedance_updates, tprint("impedances.%1_V = params.%1_v_value;\n", token.name));
                    }
                }

                array_add(*impedance_updates, tprint("// Computing impedance for: %1;", token.name));
                array_add(*impedance_updates, write_define(tprint("%1_cap_value", token.name), param_calc(token, token.param_calc_info[1])));
                add_impedance(*impedance_updates, tprint("%1_G", token.name), tprint("2 * %1_cap_value * fs", token.name), required_impedances.*, *available_values);
                add_impedance(*impedance_updates, tprint("%1_R", token.name), tprint("1 / %1_G", token.name), required_impedances.*, *available_values);
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Root_Capacitive_Voltage_Source;
                if ! is_input(token, inputs) {
                    is_known := is_param_defined(token, 0);
                    if ! is_known {
                        array_add(*root_impedance_updates, tprint("// Computing voltage for: %1;", token.name));
                        array_add(*root_impedance_updates, tprint("impedances.%1_V = params.%1_v_value;\n", token.name));
                    }
                }

                child_token := tokens[token.children[0]];
                array_add(*root_impedance_updates, tprint("// Computing impedance for: %1;", token.name));
                array_add(*root_impedance_updates, write_define(tprint("%1_cap_value", token.name), param_calc(token, token.param_calc_info[1])));
                array_add(*root_impedance_updates, tprint("impedances.%1_ca = T / (T + 2 * %2_R * %1_cap_value);", token.name, child_token.name));
                array_add(*root_impedance_updates, tprint("impedances.%1_ce = (2 * %2_R * %1_cap_value) / (T + 2 * %2_R * %1_cap_value);\n", token.name, child_token.name));

            case .Resistive_Capacitive_Voltage_Source;
                if ! is_input(token, inputs) {
                    is_known := is_param_defined(token, 0);
                    if ! is_known {
                        array_add(*impedance_updates, tprint("// Computing voltage for: %1;", token.name));
                        array_add(*impedance_updates, tprint("impedances.%1_V = params.%1_v_value;\n", token.name));
                    }
                }

                array_add(*impedance_updates, tprint("// Computing impedance for: %1;", token.name));
                array_add(*impedance_updates, write_define(tprint("%1_res_value", token.name), param_calc(token, token.param_calc_info[1])));
                array_add(*impedance_updates, write_define(tprint("%1_cap_value", token.name), param_calc(token, token.param_calc_info[2])));
                add_impedance(*impedance_updates, tprint("%1_R", token.name), tprint("(T / (2 * %1_cap_value)) + %1_res_value", token.name), required_impedances.*, *available_values);
                add_impedance(*impedance_updates, tprint("%1_G", token.name), tprint("1 / %1_R", token.name), required_impedances.*, *available_values);
                array_add(*impedance_updates, tprint("impedances.%1_T_over_2RC = T / (2 * %1_cap_value * %1_res_value);\n", token.name));
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Resistive_Current_Source;
                if ! is_input(token, inputs) {
                    is_known := is_param_defined(token, 0);
                    if ! is_known {
                        array_add(*impedance_updates, tprint("// Computing current for: %1;", token.name));
                        array_add(*impedance_updates, tprint("impedances.%1_I = params.%1_i_value;\n", token.name));
                    }
                }

                array_add(*impedance_updates, tprint("// Computing impedance for: %1;", token.name));
                add_impedance(*impedance_updates, tprint("%1_R", token.name), tprint("params.%1_res_value", token.name), required_impedances.*, *available_values);
                add_impedance(*impedance_updates, tprint("%1_G", token.name), tprint("1 / %1_R", token.name), required_impedances.*, *available_values);
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Custom;
                if token.children.count > 0 { continue; }
                has_params := token.custom_info.params_type.count > 0;
                has_state := token.custom_info.state_type.count > 0;
                has_vars := token.custom_info.vars_type.count > 0;

                array_add(*impedance_updates, tprint("// Computing impedance for: %1;", token.name));
                custom_update := "";
                if global_args.lang == "jai" {
                    custom_update = tprint("%1_R_calc := ", token.name);
                } else {
                    custom_update = tprint("[[maybe_unused]] const auto %1_R_calc = ", token.name);
                }

                custom_update = tprint("%1%2(", custom_update, token.custom_info.updater_name);
                if has_vars {
                    custom_update = tprint(ifx global_args.lang == "jai" then "%1*impedances.%2_vars, " else "%1impedances.%2_vars, ",
                                           custom_update,
                                           token.name);
                }
                if has_params {
                    custom_update = tprint("%1params.%2_params, ", custom_update, token.name);
                }
                custom_update = tprint("%1fs);", custom_update);
                array_add(*impedance_updates, custom_update);

                add_impedance(*impedance_updates, tprint("%1_R", token.name), tprint("%1_R_calc", token.name), required_impedances.*, *available_values);
                add_impedance(*impedance_updates, tprint("%1_G", token.name), tprint("1 / %1_R", token.name), required_impedances.*, *available_values);
        }
    }

    while impedance_update_todos.count > 0 {
        Sort.quick_sort (impedance_update_todos, x => x.tree_depth);
        impedance_todo := impedance_update_todos[impedance_update_todos.count - 1];
        token := impedance_todo;
        if token.type == {
            case .Series;
                child0_token := tokens[impedance_todo.children[0]];
                child1_token := tokens[impedance_todo.children[1]];
                needs_value(*impedance_updates, tprint("%1_R", child0_token.name), available_values, required_impedances);
                needs_value(*impedance_updates, tprint("%1_R", child1_token.name), available_values, required_impedances);
                array_add(*impedance_updates, tprint("// Computing impedance for: %1;", token.name));
                add_impedance(*impedance_updates, tprint("%1_R", token.name), tprint("%1_R + %2_R", child0_token.name, child1_token.name), required_impedances.*, *available_values);
                add_impedance(*impedance_updates, tprint("%1_G", token.name), tprint("1 / %1_R", token.name), required_impedances.*, *available_values);
                array_add(*impedance_updates, tprint("impedances.%1_pr = %2_R * %1_G;\n", token.name, child0_token.name));
            case .Parallel;
                child0_token := tokens[impedance_todo.children[0]];
                child1_token := tokens[impedance_todo.children[1]];
                needs_value(*impedance_updates, tprint("%1_G", child0_token.name), available_values, required_impedances);
                needs_value(*impedance_updates, tprint("%1_G", child1_token.name), available_values, required_impedances);
                array_add(*impedance_updates, tprint("// Computing impedance for: %1;", token.name));
                add_impedance(*impedance_updates, tprint("%1_G", token.name), tprint("%1_G + %2_G", child0_token.name, child1_token.name), required_impedances.*, *available_values);
                add_impedance(*impedance_updates, tprint("%1_R", token.name), tprint("1 / %1_G", token.name), required_impedances.*, *available_values);
                array_add(*impedance_updates, tprint("impedances.%1_pr = %2_G * %1_R;\n", token.name, child0_token.name));
            case .Inverter;
                child_token := tokens[impedance_todo.children[0]];
                needs_value(*impedance_updates, tprint("%1_R", child_token.name), available_values, required_impedances);
                array_add(*impedance_updates, tprint("// Computing impedance for: %1;", token.name));
                add_impedance(*impedance_updates, tprint("%1_R", token.name), tprint("%1_R", child_token.name), required_impedances.*, *available_values);
                add_impedance(*impedance_updates, tprint("%1_G", token.name), tprint("1 / %1_R", token.name), required_impedances.*, *available_values);
            case .Custom;
                has_params := token.custom_info.params_type.count > 0;
                has_state := token.custom_info.state_type.count > 0;
                has_vars := token.custom_info.vars_type.count > 0;

                array_add(*impedance_updates, tprint("// Computing impedance for: %1;", token.name));
                custom_update := "";
                if token != root_token {
                    if global_args.lang == "jai" {
                        custom_update = tprint("%1_R_calc := ", token.name);
                    } else {
                        custom_update = tprint("[[maybe_unused]] const auto %1_R_calc = ", token.name);
                    }
                }

                custom_update = tprint("%1%2(", custom_update, token.custom_info.updater_name);
                if has_vars {
                    custom_update = tprint(ifx global_args.lang == "jai" then "%1*impedances.%2_vars, " else "%1impedances.%2_vars, ",
                                           custom_update,
                                           token.name);
                }
                if has_params {
                    custom_update = tprint("%1params.%2_params, ", custom_update, token.name);
                }
                for child : token.children {
                    // @TODO: I don't think I have a test case for these `needs_value` calls yet?
                    needs_value(*impedance_updates, tprint("%1_R", tokens[child].name), available_values, required_impedances);
                    needs_value(*impedance_updates, tprint("%1_G", tokens[child].name), available_values, required_impedances);
                    custom_update = tprint("%1%2_R, ", custom_update, tokens[child].name);
                    custom_update = tprint("%1%2_G, ", custom_update, tokens[child].name);
                }
                custom_update.count -= 2;
                custom_update = tprint("%1);", custom_update);
                array_add(*impedance_updates, custom_update);

                if token != root_token {
                    add_impedance(*impedance_updates, tprint("%1_R", token.name), tprint("%1_R_calc", token.name), required_impedances.*, *available_values);
                    add_impedance(*impedance_updates, tprint("%1_G", token.name), tprint("1 / %1_R", token.name), required_impedances.*, *available_values);
                }
        }

        // This is leaky, but I don't think we care?
        impedance_update_todos.count -= 1;

        if token != root_token {
            add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token, false);
        } else {
            // verbose_print("Finished climbing impedance tree!");
        }
    }

    if (root_impedance_updates.count > 0) {
        array_add(*impedance_updates, "");
        for update : root_impedance_updates { array_add(*impedance_updates, update); }
    }

    return impedance_updates;
}

generate_circuit :: (tokens: [] Token, inputs: [] string, required_impedances: [] string) -> string {
    circuit_vars := generate_circuit_vars(tokens, inputs, required_impedances);

    ts := context.temporary_storage;
    start_scratch();
    builder: String_Builder;

    print_to_builder(*builder, generate_struct(circuit_vars.params_vars, "Params"));
    print_to_builder(*builder, "\n%", generate_struct(circuit_vars.impedance_vars, "Impedances"));
    print_to_builder(*builder, "\n%", generate_struct(circuit_vars.state_vars, "State"));

    context.temporary_storage = ts;
    return builder_to_string(*builder);
}

generate_updaters :: (tokens: [] Token,
                      inputs: [] string,
                      meta_info: Meta_Info,
                      required_impedances: *[..] string) -> string {
    ts := context.temporary_storage;
    start_scratch();
    builder: String_Builder;

    updater_args := Method_Arg.[
        .{name="impedances", type="Impedances", is_reference=true},
        .{name="fs", type=global_args.dtype},
        .{name="params", type="Params"},
    ];

    // updaters from meta
    meta_updaters: [..] string;
    for updater : meta_info.impedance_updaters {
        parts := String.split(updater, cast(u8) #char "(");
        updater_name := parts[0];
        param_tokens := String.split(String.trim(parts[1], " )"), cast(u8) #char ",");
        for *t : param_tokens { t.* = String.trim(t.*); }
        updates := generate_updater(tokens, inputs, required_impedances, param_tokens);
        array_add(*meta_updaters, generate_method(updater_name,
                                                  updater_args,
                                                  updates));
    }

    // global updater
    // we don't want this one to affect the required impedances!
    save_impedances := required_impedances.*;
    defer required_impedances.* = save_impedances;
    if global_args.lang == "jai" {
        updater_args[2].default_value = ".{}";
    } else {
        updater_args[2].default_value = "{}";
    }
    impedance_updates := generate_updater(tokens, inputs, required_impedances);
    gen_code := generate_method("calc_impedances",
                                updater_args,
                                impedance_updates);
    for updater_method : meta_updaters {
        gen_code = tprint("%\n%", gen_code, updater_method);
    }

    context.temporary_storage = ts;
    return copy_string(gen_code);
}

reduce_circuit :: (original_tokens: [] Token, inputs: [] string, outputs: [] string) -> [] Token {
    remove_token :: (tokens: *[] Token, index: s64) {
        for *token : tokens.* {
            if token.index > index { token.index -= 1; }
            if token.parent > index { token.parent -= 1; }
            for *child_idx : token.children {
                if child_idx.* > index { child_idx.* -= 1; }
            }
        }
        array_ordered_remove_by_index(tokens, index);
    }

    sub_input :: (inputs: [] string, old: Token, new: Token) {
        for *input : inputs {
            if input.* == old.name { input.* = new.name; }
        }
    }

    join_arrays :: (arr1: [] $T, arr2: [] T) -> [] T {
        new_arr : [] string;
        array_resize(*new_arr, arr1.count + arr2.count);
        for idx : 0..arr1.count-1 { new_arr[idx] = arr1[idx]; }
        for idx : arr1.count..new_arr.count-1 { new_arr[idx] = arr2[idx - arr1.count]; }
        return new_arr;
    }

    update_param_name_idxs :: (info: *Token.Param_Calc_Info, shift: s64) {
        if (info.param_name_idx >= 0) {
            info.param_name_idx += shift;
            return;
        }

        update_param_name_idxs(info.child_left, shift);
        update_param_name_idxs(info.child_right, shift);
    }

    make_1port :: (original_token: Token, idx: s64, e1: Token, e2: Token, type: Token_Type, calc: Token.Param_Calc_Info.Calc_Type) -> Token {
        new_args := join_arrays (e1.args, e2.args);
        param_names := join_arrays (e1.param_names, e2.param_names);
        update_param_name_idxs(*e2.param_calc_info[0], e1.param_names.count);

        param_calc_info: [] Token.Param_Calc_Info;
        array_resize(*param_calc_info, 1);
        param_calc_info[0] = .{
            calc = calc,
            child_left = *e1.param_calc_info[0],
            child_right = *e2.param_calc_info[0],
        };

        return  Token.{
            name = original_token.name,
            type = type,
            args = new_args,
            parent = original_token.parent,
            tree_depth = original_token.tree_depth,
            index = idx,
            param_names = param_names,
            param_calc_info = param_calc_info,
        };
    }

    make_rc :: (original_token: Token, idx: s64, res: Token, cap: Token, type: Token_Type) -> Token {
        new_args := join_arrays (res.args, cap.args);
        param_names := join_arrays (res.param_names, cap.param_names);
        update_param_name_idxs(*cap.param_calc_info[0], res.param_names.count);

        param_calc_info: [] Token.Param_Calc_Info;
        array_resize(*param_calc_info, 2);
        param_calc_info[0] = res.param_calc_info[0];
        param_calc_info[1] = cap.param_calc_info[0];

        return  Token.{
            name = original_token.name,
            type = type,
            args = new_args,
            parent = original_token.parent,
            tree_depth = original_token.tree_depth,
            index = idx,
            param_names = param_names,
            param_calc_info = param_calc_info,
        };
    }

    rc_add :: (original_token: Token, idx: s64, rc: Token, new: Token) -> Token {
        new_args := join_arrays (rc.args, new.args);
        param_names := join_arrays (rc.param_names, new.param_names);
        update_param_name_idxs(*new.param_calc_info[0], rc.param_names.count);

        param_calc_info: [] Token.Param_Calc_Info;
        array_resize(*param_calc_info, 2);
        if (new.type == .Resistor) {
            param_calc_info[0] = .{
                calc = ifx rc.type == .Resistor_Capacitor_Series then .Sum else .Reciprocal_Sum,
                child_left = *rc.param_calc_info[0],
                child_right = *new.param_calc_info[0],
            };
            param_calc_info[1] = rc.param_calc_info[1];
        } else if (new.type == .Capacitor) {
            param_calc_info[0] = rc.param_calc_info[0];
            param_calc_info[1] = .{
                calc = ifx rc.type == .Resistor_Capacitor_Series then .Reciprocal_Sum else .Sum,
                child_left = *rc.param_calc_info[1],
                child_right = *new.param_calc_info[0],
            };
        }

        return  Token.{
            name = original_token.name,
            type = rc.type,
            args = new_args,
            parent = original_token.parent,
            tree_depth = original_token.tree_depth,
            index = idx,
            param_names = param_names,
            param_calc_info = param_calc_info,
        };
    }

    vs_add :: (original_token: Token, idx: s64, vs: Token, new: Token) -> Token {
        new_args := join_arrays (vs.args, new.args);
        param_names := join_arrays (vs.param_names, new.param_names);
        update_param_name_idxs(*new.param_calc_info[0], vs.param_names.count);

        param_calc_info: [] Token.Param_Calc_Info;
        array_resize(*param_calc_info, 2);
        if (new.type == .Resistor) {
            param_calc_info[0] = vs.param_calc_info[0];
            param_calc_info[1] = .{
                calc = .Sum,
                child_left = *vs.param_calc_info[1],
                child_right = *new.param_calc_info[0],
            };
        } else if (new.type == .Capacitor) {
            param_calc_info[0] = vs.param_calc_info[0];
            param_calc_info[1] = .{
                calc = .Reciprocal_Sum,
                child_left = *vs.param_calc_info[1],
                child_right = *new.param_calc_info[0],
            };
        }

        return  Token.{
            name = original_token.name,
            type = vs.type,
            args = new_args,
            parent = original_token.parent,
            tree_depth = original_token.tree_depth,
            index = idx,
            param_names = param_names,
            param_calc_info = param_calc_info,
            invert = ! vs.invert,
        };
    }

    make_rcvs :: (original_token: Token, idx: s64, combo: Token, new: Token) -> Token {
        new_args := join_arrays (combo.args, new.args);
        param_names := join_arrays (combo.param_names, new.param_names);
        update_param_name_idxs(*new.param_calc_info[0], combo.param_names.count);

        param_calc_info: [] Token.Param_Calc_Info;
        array_resize(*param_calc_info, 3);
        param_calc_info[0] = combo.param_calc_info[0];
        if new.type == .Resistor {
            param_calc_info[1] = new.param_calc_info[0]; // res
            param_calc_info[2] = combo.param_calc_info[1]; // cap
        } else if new.type == .Capacitor {
            param_calc_info[1] = combo.param_calc_info[1]; // res
            param_calc_info[2] = new.param_calc_info[0]; // cap
        }

        return  Token.{
            name = original_token.name,
            type = .Resistive_Capacitive_Voltage_Source,
            args = new_args,
            parent = original_token.parent,
            tree_depth = original_token.tree_depth,
            index = idx,
            param_names = param_names,
            param_calc_info = param_calc_info,
            invert = combo.invert,
        };
    }

    rcvs_add :: (original_token: Token, idx: s64, vs: Token, new: Token) -> Token {
        new_args := join_arrays (vs.args, new.args);
        param_names := join_arrays (vs.param_names, new.param_names);
        update_param_name_idxs(*new.param_calc_info[0], vs.param_names.count);

        param_calc_info: [] Token.Param_Calc_Info;
        array_resize(*param_calc_info, 3);
        param_calc_info[0] = vs.param_calc_info[0];
        if (new.type == .Resistor) {
            param_calc_info[1] = .{
                calc = .Sum,
                child_left = *vs.param_calc_info[1],
                child_right = *new.param_calc_info[0],
            };
            param_calc_info[2] = vs.param_calc_info[2];
        } else {
            assert(false);
        }

        return  Token.{
            name = original_token.name,
            type = vs.type,
            args = new_args,
            parent = original_token.parent,
            tree_depth = original_token.tree_depth,
            index = idx,
            param_names = param_names,
            param_calc_info = param_calc_info,
            invert = ! vs.invert,
        };
    }

    // For the reduction to work, each element's children need to have higher indexes
    for token,idx : original_tokens {
        for child_idx : token.children {
            assert(child_idx > idx);
        }
    }

    tokens := array_copy(original_tokens);

    reductions_count := 1;
    reduction_iters := 0;
    total_reductions := 0;
    while reductions_count > 0 {
        reductions_count = 0;

        idx := 0;
        while idx < tokens.count {
            token := tokens[idx];
            if token.type == {
                case .Series;
                    child0 := tokens[token.children[0]];
                    child1 := tokens[token.children[1]];

                    is_child_output := false;
                    for output : outputs {
                        output_info := String.split(output, ":");
                        is_child_output |= child0.name == output_info[1];
                        is_child_output |= child1.name == output_info[1];
                    }

                    // R + R
                    if (child0.type == .Resistor && child1.type == .Resistor && ! is_child_output) {
                        tokens[idx] = make_1port(token, idx, child0, child1, .Resistor, .Sum);
                        remove_token(*tokens, token.children[1]);
                        remove_token(*tokens, token.children[0]);
                        reductions_count += 1;
                    }

                    // C + C
                    if (child0.type == .Capacitor && child1.type == .Capacitor && ! is_child_output) {
                        tokens[idx] = make_1port(token, idx, child0, child1, .Capacitor, .Reciprocal_Sum);
                        remove_token(*tokens, token.children[1]);
                        remove_token(*tokens, token.children[0]);
                        reductions_count += 1;
                    }

                    // RCSeries
                    if (child0.type == .Resistor && child1.type == .Capacitor && ! is_child_output) {
                        tokens[idx] = make_rc(token, idx, child0, child1, .Resistor_Capacitor_Series);
                        remove_token(*tokens, token.children[1]);
                        remove_token(*tokens, token.children[0]);
                        reductions_count += 1;
                    }
                    if (child1.type == .Resistor && child0.type == .Capacitor && ! is_child_output) {
                        tokens[idx] = make_rc(token, idx, child1, child0, .Resistor_Capacitor_Series);
                        remove_token(*tokens, token.children[1]);
                        remove_token(*tokens, token.children[0]);
                        reductions_count += 1;
                    }

                    // RCSeries + R
                    if (child0.type == .Resistor_Capacitor_Series && child1.type == .Resistor && ! is_child_output) {
                        tokens[idx] = rc_add(token, idx, child0, child1);
                        remove_token(*tokens, token.children[1]);
                        remove_token(*tokens, token.children[0]);
                        reductions_count += 1;
                    }
                    if (child0.type == .Resistor && child1.type == .Resistor_Capacitor_Series && ! is_child_output) {
                        tokens[idx] = rc_add(token, idx, child1, child0);
                        remove_token(*tokens, token.children[1]);
                        remove_token(*tokens, token.children[0]);
                        reductions_count += 1;
                    }

                    // RCSeries + C
                    if (child0.type == .Resistor_Capacitor_Series && child1.type == .Capacitor && ! is_child_output) {
                        tokens[idx] = rc_add(token, idx, child0, child1);
                        remove_token(*tokens, token.children[1]);
                        remove_token(*tokens, token.children[0]);
                        reductions_count += 1;
                    }
                    if (child0.type == .Capacitor && child1.type == .Resistor_Capacitor_Series && ! is_child_output) {
                        tokens[idx] = rc_add(token, idx, child1, child0);
                        remove_token(*tokens, token.children[1]);
                        remove_token(*tokens, token.children[0]);
                        reductions_count += 1;
                    }

                    // RVs + R
                    if (child0.type == .Resistive_Voltage_Source && child1.type == .Resistor && ! is_child_output) {
                        tokens[idx] = vs_add(token, idx, child0, child1);
                        sub_input(inputs, child0, tokens[idx]);
                        remove_token(*tokens, token.children[1]);
                        remove_token(*tokens, token.children[0]);
                        reductions_count += 1;
                    }
                    if (child0.type == .Resistor && child1.type == .Resistive_Voltage_Source && ! is_child_output) {
                        tokens[idx] = vs_add(token, idx, child1, child0);
                        sub_input(inputs, child1, tokens[idx]);
                        remove_token(*tokens, token.children[1]);
                        remove_token(*tokens, token.children[0]);
                        reductions_count += 1;
                    }

                    // CVs + C
                    if (child0.type == .Capacitive_Voltage_Source && child1.type == .Capacitor && ! is_child_output) {
                        tokens[idx] = vs_add(token, idx, child0, child1);
                        sub_input(inputs, child0, tokens[idx]);
                        remove_token(*tokens, token.children[1]);
                        remove_token(*tokens, token.children[0]);
                        reductions_count += 1;
                    }
                    if (child0.type == .Capacitor && child1.type == .Capacitive_Voltage_Source && ! is_child_output) {
                        tokens[idx] = vs_add(token, idx, child1, child0);
                        sub_input(inputs, child1, tokens[idx]);
                        remove_token(*tokens, token.children[1]);
                        remove_token(*tokens, token.children[0]);
                        reductions_count += 1;
                    }

                    // RVs + C
                    if (child0.type == .Resistive_Voltage_Source && child1.type == .Capacitor && ! is_child_output) {
                        tokens[idx] = make_rcvs(token, idx, child0, child1);
                        sub_input(inputs, child0, tokens[idx]);
                        remove_token(*tokens, token.children[1]);
                        remove_token(*tokens, token.children[0]);
                        reductions_count += 1;
                    }
                    if (child0.type == .Capacitor && child1.type == .Resistive_Voltage_Source && ! is_child_output) {
                        tokens[idx] = make_rcvs(token, idx, child1, child0);
                        sub_input(inputs, child1, tokens[idx]);
                        remove_token(*tokens, token.children[1]);
                        remove_token(*tokens, token.children[0]);
                        reductions_count += 1;
                    }

                    // RCVs + R
                    if (child0.type == .Resistive_Capacitive_Voltage_Source && child1.type == .Resistor && ! is_child_output) {
                        tokens[idx] = rcvs_add(token, idx, child0, child1);
                        sub_input(inputs, child0, tokens[idx]);
                        remove_token(*tokens, token.children[1]);
                        remove_token(*tokens, token.children[0]);
                        reductions_count += 1;
                    }
                    if (child0.type == .Resistor && child1.type == .Resistive_Capacitive_Voltage_Source && ! is_child_output) {
                        tokens[idx] = rcvs_add(token, idx, child1, child0);
                        sub_input(inputs, child1, tokens[idx]);
                        remove_token(*tokens, token.children[1]);
                        remove_token(*tokens, token.children[0]);
                        reductions_count += 1;
                    }

                    // @TODO:
                    // CVs + R
                    // RCVs + C
                    // RCVs + RCSeries

                case .Parallel;
                    child0 := tokens[token.children[0]];
                    child1 := tokens[token.children[1]];

                    is_child_output := false;
                    for output : outputs {
                        output_info := String.split(output, ":");
                        is_child_output |= child0.name == output_info[1];
                        is_child_output |= child1.name == output_info[1];
                    }

                    // R || R
                    if (child0.type == .Resistor && child1.type == .Resistor && ! is_child_output) {
                        tokens[idx] = make_1port(token, idx, child0, child1, .Resistor, .Reciprocal_Sum);
                        remove_token(*tokens, token.children[1]);
                        remove_token(*tokens, token.children[0]);
                        reductions_count += 1;
                    }

                    // C || C
                    if (child0.type == .Capacitor && child1.type == .Capacitor && ! is_child_output) {
                        tokens[idx] = make_1port(token, idx, child0, child1, .Capacitor, .Sum);
                        remove_token(*tokens, token.children[1]);
                        remove_token(*tokens, token.children[0]);
                        reductions_count += 1;
                    }

                    // RCParallel
                    if (child0.type == .Resistor && child1.type == .Capacitor && ! is_child_output) {
                        tokens[idx] = make_rc(token, idx, child0, child1, .Resistor_Capacitor_Parallel);
                        remove_token(*tokens, token.children[1]);
                        remove_token(*tokens, token.children[0]);
                        reductions_count += 1;
                    }
                    if (child1.type == .Resistor && child0.type == .Capacitor && ! is_child_output) {
                        tokens[idx] = make_rc(token, idx, child1, child0, .Resistor_Capacitor_Parallel);
                        remove_token(*tokens, token.children[1]);
                        remove_token(*tokens, token.children[0]);
                        reductions_count += 1;
                    }

                    // RCParallel || R
                    if (child0.type == .Resistor_Capacitor_Parallel && child1.type == .Resistor && ! is_child_output) {
                        tokens[idx] = rc_add(token, idx, child0, child1);
                        remove_token(*tokens, token.children[1]);
                        remove_token(*tokens, token.children[0]);
                        reductions_count += 1;
                    }
                    if (child0.type == .Resistor && child1.type == .Resistor_Capacitor_Parallel && ! is_child_output) {
                        tokens[idx] = rc_add(token, idx, child1, child0);
                        remove_token(*tokens, token.children[1]);
                        remove_token(*tokens, token.children[0]);
                        reductions_count += 1;
                    }

                    // RCParallel + C
                    if (child0.type == .Resistor_Capacitor_Parallel && child1.type == .Capacitor && ! is_child_output) {
                        tokens[idx] = rc_add(token, idx, child0, child1);
                        remove_token(*tokens, token.children[1]);
                        remove_token(*tokens, token.children[0]);
                        reductions_count += 1;
                    }
                    if (child0.type == .Capacitor && child1.type == .Resistor_Capacitor_Parallel && ! is_child_output) {
                        tokens[idx] = rc_add(token, idx, child1, child0);
                        remove_token(*tokens, token.children[1]);
                        remove_token(*tokens, token.children[0]);
                        reductions_count += 1;
                    }

                case .Ideal_Voltage_Source;
                    child := tokens[token.children[0]];
                    if (child.type == .Series) {
                        grandchild0 := tokens[child.children[0]];
                        grandchild1 := tokens[child.children[1]];

                        is_grandchild0_output := false;
                        is_grandchild1_output := false;
                        for output : outputs {
                            output_info := String.split(output, ":");
                            is_grandchild0_output |= grandchild0.name == output_info[1];
                            is_grandchild1_output |= grandchild1.name == output_info[1];
                        }

                        root_r_or_cvs :: (tokens: *[] Token, idx: s64, r_idx: s64, child_idx: s64, type: Token_Type) {
                            token := tokens.*[idx];
                            res := tokens.*[r_idx];
                            new_args := join_arrays (token.args, res.args);
                            param_names := join_arrays (token.param_names, res.param_names);
                            update_param_name_idxs(*res.param_calc_info[0], token.param_names.count);

                            children: [..] s64;
                            array_add(*children, child_idx);

                            param_calc_info: [] Token.Param_Calc_Info;
                            array_resize(*param_calc_info, 2);
                            param_calc_info[1] = res.param_calc_info[0];

                            tokens.*[idx] = Token.{
                                name = token.name,
                                type = type,
                                args = new_args,
                                parent = token.parent,
                                tree_depth = token.tree_depth,
                                index = idx,
                                param_names = param_names,
                                param_calc_info = param_calc_info,
                                input = token.input,
                                children = children,
                                invert = type == .Root_Resistive_Voltage_Source,
                            };

                            tokens.*[child_idx].parent = idx;
                            remove_token(tokens, r_idx);
                            remove_token(tokens, token.children[0]);
                            for *token : tokens.* { if token.tree_depth > 1 { token.tree_depth -= 1; } }
                        }

                        if (grandchild0.type == .Resistor && ! is_grandchild0_output) {
                            root_r_or_cvs(*tokens, idx, grandchild0.index, grandchild1.index, .Root_Resistive_Voltage_Source);
                            reductions_count += 1;
                        }

                        if (grandchild1.type == .Resistor && ! is_grandchild1_output) {
                            root_r_or_cvs(*tokens, idx, grandchild1.index, grandchild0.index, .Root_Resistive_Voltage_Source);
                            reductions_count += 1;
                        }

                        if (grandchild0.type == .Capacitor && ! is_grandchild0_output) {
                            root_r_or_cvs(*tokens, idx, grandchild0.index, grandchild1.index, .Root_Capacitive_Voltage_Source);
                            reductions_count += 1;
                        }

                        if (grandchild1.type == .Capacitor && ! is_grandchild1_output) {
                            root_r_or_cvs(*tokens, idx, grandchild1.index, grandchild0.index, .Root_Capacitive_Voltage_Source);
                            reductions_count += 1;
                        }
                    }

                case .Root_Resistive_Voltage_Source;
                    child := tokens[token.children[0]];
                    if (child.type == .Series) {
                        grandchild0 := tokens[child.children[0]];
                        grandchild1 := tokens[child.children[1]];

                        is_grandchild0_output := false;
                        is_grandchild1_output := false;
                        for output : outputs {
                            output_info := String.split(output, ":");
                            is_grandchild0_output |= grandchild0.name == output_info[1];
                            is_grandchild1_output |= grandchild1.name == output_info[1];
                        }

                        root_rvs_add_r :: (tokens: *[] Token, idx: s64, r_idx: s64, child_idx: s64) {
                            token := tokens.*[idx];
                            res := tokens.*[r_idx];

                            new_args := join_arrays (token.args, res.args);
                            param_names := join_arrays (token.param_names, res.param_names);
                            update_param_name_idxs(*res.param_calc_info[0], token.param_names.count);

                            children: [..] s64;
                            array_add(*children, child_idx);

                            param_calc_info: [] Token.Param_Calc_Info;
                            array_resize(*param_calc_info, 2);
                            param_calc_info[1] = .{
                                calc = .Sum,
                                child_left = *token.param_calc_info[1],
                                child_right = *res.param_calc_info[0],
                            };

                            tokens.*[idx] = Token.{
                                name = token.name,
                                type = .Root_Resistive_Voltage_Source,
                                args = new_args,
                                parent = token.parent,
                                tree_depth = token.tree_depth,
                                index = idx,
                                param_names = param_names,
                                param_calc_info = param_calc_info,
                                input = token.input,
                                children = children,
                                invert = !token.invert,
                            };

                            tokens.*[child_idx].parent = idx;
                            remove_token(tokens, r_idx);
                            remove_token(tokens, token.children[0]);
                            for *token : tokens.* { if token.tree_depth > 1 { token.tree_depth -= 1; } }
                        }

                        if (grandchild0.type == .Resistor && ! is_grandchild0_output) {
                            root_rvs_add_r(*tokens, idx, grandchild0.index, grandchild1.index);
                            reductions_count += 1;
                        }

                        if (grandchild1.type == .Resistor && ! is_grandchild1_output) {
                            root_rvs_add_r(*tokens, idx, grandchild1.index, grandchild0.index);
                            reductions_count += 1;
                        }
                    }
            }
            idx += 1;
        }
        reduction_iters += 1;
        total_reductions += reductions_count;
    }

    verbose_print("Performed %1 reductions over %2 iterations\n",
                  total_reductions,
                  reduction_iters);

    return tokens;
}
