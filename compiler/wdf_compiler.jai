#import "Basic";
CLI :: #import "Command_Line";
File :: #import "File";
Process :: #import "Process";
#load "wdf_parser.jai";

Args :: struct {
    verbose   : bool = false;     @"?Verbose"
    version   : bool = false;     @"?Version"
    lang      : string = "cpp";   @"?Language of generated code. Currently supports cpp (default) and jai."
    dtype     : string = "float"; @"?Data type to use for generated code. (Default: float)"
    align     : int = 16;         @"?Default data alignment to use for generated code. (Default: 16)"
    no_reduce : bool = false;     @"?Perform reductions on the circuit where possible (Default: true)"
}
argument_flags :: CLI.Default_Argument_Flags; // |.DOUBLE_DASH_REQUIRED;
global_args: Args;

get_compiler_version :: () -> string {
    result, output, err, timed_out := Process.run_command("git", "rev-parse", "--short", "HEAD", capture_and_return_output=true);
    return tprint("0.1.0.%", String.trim(output));
}

COMPILER_VERSION :: #run get_compiler_version();

main :: () {
    // context setup...
    context.allocator = temp;
    context.print_style.default_format_float.mode = .SCIENTIFIC;
    context.print_style.default_format_float.zero_removal = .ONE_ZERO_AFTER_DECIMAL;

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
        circuit_tokens = reduce_circuit(circuit_tokens, outputs);
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
}

start_gen_code :: (meta_info: Meta_Info) -> string {
    cli_args := get_command_line_arguments();
    gen_code := tprint("// Auto-generated with wdf_compiler version %.\n// Command: wdf_compiler", COMPILER_VERSION);
    for i : 1..cli_args.count - 1 {
        gen_code = tprint("%1 %2", gen_code, cli_args[i]);
    }

    gen_code = tprint("%1\n", gen_code);
    if global_args.lang == "cpp" { gen_code = tprint("%1\n#pragma once\n", gen_code); }

    if global_args.lang == "jai" {
        if meta_info.extra_loads.count > 0 {
            includes_code := "";
            for include : meta_info.extra_loads {
                includes_code = tprint("%#load %;\n", includes_code, include);
            }
            gen_code = tprint("%\n%", gen_code, includes_code);
        }
    } else {
        if meta_info.extra_includes.count > 0 {
            includes_code := "";
            for include : meta_info.extra_includes {
                includes_code = tprint("%#include %\n", includes_code, include);
            }
            gen_code = tprint("%\n%", gen_code, includes_code);
        }
    }

    if global_args.lang == "cpp" {
        if (meta_info.namespace.count > 0) {
            gen_code = tprint("%\nnamespace %\n{\n", gen_code, meta_info.namespace);
        }
    }
    return gen_code;
}

end_gen_code :: (meta_info: Meta_Info) -> string {
    gen_code := "";
    if global_args.lang == "cpp" {
        if (meta_info.namespace.count > 0) {
            gen_code = tprint("} // namespace %\n\n", meta_info.namespace);
        }
    }
    return gen_code;
}

Meta_Info :: struct {
    namespace := "";
    impedance_updaters : [..] string;
    extra_includes : [..] string;
    extra_loads : [..] string;
}

parse_meta_info :: (meta_info_str: string) -> Meta_Info {
    meta_info : Meta_Info;

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

    verbose_print("Meta info: %\n", meta_info);
    return meta_info;
}

generate_struct :: (impedance_vars: [] string, name: string) -> string {
    struct_code := "";
    if global_args.lang == "jai" {
        struct_code = tprint("%1%2 :: struct {\n", struct_code, name);
    } else {
        struct_code = tprint("%1struct %2 {\n", struct_code, name);
    }
    for var : impedance_vars {
        struct_code = tprint("%    %\n", struct_code, var);
    }
    if global_args.lang == "jai" {
        struct_code = tprint("%}\n", struct_code);
    } else {
        struct_code = tprint("%};\n", struct_code);
    }
    return struct_code;
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
    return_type := get_return_type(return_types);

    method_code := "";
    if global_args.lang == "jai" {
        method_code = tprint("%1%2 :: (", method_code, name);
    } else {
        method_code = tprint("%1inline %2 %3(", method_code, return_type, name); // @TODO: do we always want these methods to be marked inline?
    }
    for arg, idx : args {
        if idx != 0 { method_code = tprint("%1, ", method_code); }
        if global_args.lang == "jai" {
            method_code = tprint("%1%2: ", method_code, arg.name);
            if arg.is_reference { method_code = tprint("%1*", method_code); }
            method_code = tprint("%1%2", method_code, arg.type);
            if arg.default_value.count > 0 { method_code = tprint("%1 = %2", method_code, arg.default_value); }
        } else {
            if arg.is_const { method_code = tprint("%1const ", method_code); }
            method_code = tprint("%1%2", method_code, arg.type);
            if arg.is_reference { method_code = tprint("%1&", method_code); }
            method_code = tprint("%1 %2", method_code, arg.name);
            if arg.default_value.count > 0 { method_code = tprint("%1= %2", method_code, arg.default_value); }
        }
    }

    if global_args.lang == "jai" {
        if return_type == "void" {
            method_code = tprint("%1) {\n", method_code);
        } else {
            method_code = tprint("%1) -> %2 {\n", method_code, return_type);
        }
    } else {
        method_code = tprint("%1) {\n", method_code);
    }

    for line : lines {
        method_code = tprint("%1    %2\n", method_code, line);
    }
    method_code = tprint("%1}\n", method_code);

    return method_code;
}

get_value :: (val_str: string) -> float32 {
    val, success, remainder := string_to_float(val_str);
    error_assert (success, tprint("Unable to parse value string: %", val_str));
    if remainder.count == 0 { return val; }
    if remainder[0] == {
        case #char "M";
            val *= 1.0e6;
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

    if num_children == 1 {
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
    input_args : [..] Method_Arg;
    array_add (*input_args, Method_Arg.{name="state", type="State", is_reference=true});
    array_add (*input_args, Method_Arg.{name="impedances", type="Impedances", is_reference=true, is_const=true});
    required_impedances : [..] string;

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
                array_add(*down_tree_statements, tprint("state.%1_z -= impedances.%1_T_over_T_plus_2RC * (%1_a + state.%1_z); // %1 state update", token.name));
                array_add(*required_impedances, tprint("%_T_over_T_plus_2RC", token.name));
            case .Resistor_Capacitor_Parallel;
                array_add(*up_tree_statements, write_define(tprint("%_b", token.name),
                                                            tprint("impedances.%1_twoRC_over_twoRC_plus_T * state.%1_z", token.name),
                                                            tprint("% reflected", token.name)));
                array_add(*down_tree_statements, tprint("state.%1_z = %1_b + %1_a - state.%1_z; // %1 state update", token.name));
                array_add(*required_impedances, tprint("%_twoRC_over_twoRC_plus_T", token.name));
            case .Resistive_Voltage_Source;
                if ! is_input(token, inputs) { token.input = tprint("impedances.%1_V", token.name); }
                array_add(*up_tree_statements, write_define(tprint("%_b", token.name),
                                                            token.input,
                                                            tprint("% reflected", token.name)));
            case .Resistive_Current_Source;
                if ! is_input(token, inputs) { token.input = tprint("impedances.%1_I", token.name); }
                array_add(*up_tree_statements, write_define(tprint("%_b", token.name),
                                                            tprint("impedances.%1_R * %2", token.name, token.input),
                                                            tprint("% reflected", token.name)));
                array_add(*required_impedances, tprint("%_R", token.name));
            case .Capacitive_Voltage_Source;
                array_add(*up_tree_statements, tprint("state.%1_v1 = %2; // %1 state update", token.name, token.input));
                array_add(*up_tree_statements, write_define(tprint("%_b", token.name),
                                                            tprint("state.%1_z + %2 - state.%1_v1", token.name, token.input),
                                                            tprint("% reflected", token.name)));
                array_add(*down_tree_statements, tprint("state.%1_z = %1_a; // %1 state update", token.name));
            case .Series;
                child0_token := tokens[token.children[0]];
                child1_token := tokens[token.children[1]];
                array_add(*up_tree_statements, write_define(tprint("%_b", token.name),
                                                            tprint("-(%1_b + %2_b)", child0_token.name, child1_token.name),
                                                            tprint("% reflected", token.name)));
                array_add(*down_tree_statements, write_define(tprint("%_a", child0_token.name),
                                                              tprint("%2_b - impedances.%1_pr * (%1_a - %1_b)", token.name, child0_token.name),
                                                              tprint("% incident", child0_token.name)));
                array_add(*down_tree_statements, write_define(tprint("%_a", child1_token.name),
                                                              tprint("-(%1_a + %2_a)", token.name, child0_token.name),
                                                              tprint("% incident", child1_token.name)));
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

    return generate_method("process",
                           input_args,
                           code_statements,
                           return_types),
            required_impedances;
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
                write_param_named(*params_vars, token, token.param_names[0]);
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Capacitor;
                write_param_named(*params_vars, token, token.param_names[0]);
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
                write_param_named(*params_vars, token, token.param_names[0], 0);
                write_param_named(*params_vars, token, token.param_names[1], 1);
                array_add(*state_vars, write_struct_field(tprint("%_z", token.name)));
                array_add(*impedance_vars, write_struct_field(tprint("%1_T_over_T_plus_2RC", token.name)));
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Resistor_Capacitor_Parallel;
                write_param_named(*params_vars, token, token.param_names[0], 0);
                write_param_named(*params_vars, token, token.param_names[1], 1);
                array_add(*state_vars, write_struct_field(tprint("%_z", token.name)));
                array_add(*impedance_vars, write_struct_field(tprint("%1_twoRC_over_twoRC_plus_T", token.name)));
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

                if is_input_source {
                    write_param(*params_vars, token, 0, "res_value");
                } else {
                    write_param(*params_vars, token, 1, "res_value");
                }
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Capacitive_Voltage_Source;
                // @TODO: why is this so different from Resistive_Voltage_Source?
                is_input_source := is_input(token, inputs);
                error_assert(is_input_source, tprint("Capacitive Voltage Source % is required to be an input source!", token.name));

                array_add(*state_vars, write_struct_field(tprint("%_z", token.name)));
                array_add(*state_vars, write_struct_field(tprint("%_v1", token.name)));
                write_param(*params_vars, token, 0, "cap_value");
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
                error_assert (false, "NOT IMPLEMENTED (generate circuit vars for custom leaf node)");
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

    available_values : [..] string;
    impedance_updates : [..] string;
    impedance_update_todos : [..] *Token;
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
                array_add(*impedance_updates, write_define(tprint("%1_value", token.name), tprint("params.%1;", token.param_names[0])));
                add_impedance(*impedance_updates, tprint("%1_R", token.name), tprint("%1_value", token.name), required_impedances.*, *available_values);
                add_impedance(*impedance_updates, tprint("%1_G", token.name), tprint("1 / %1_R", token.name), required_impedances.*, *available_values);
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Capacitor;
                array_add(*impedance_updates, tprint("// Computing impedance for: %1;", token.name));
                array_add(*impedance_updates, write_define(tprint("%1_value", token.name), tprint("params.%1;", token.param_names[0])));
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
                array_add(*impedance_updates, write_define(tprint("%1_value", token.name), tprint("params.%1;", token.param_names[0])));
                add_impedance(*impedance_updates, tprint("%1_R", token.name), tprint("2 * %1_value * fs", token.name), required_impedances.*, *available_values);
                add_impedance(*impedance_updates, tprint("%1_G", token.name), tprint("1 / %1_R", token.name), required_impedances.*, *available_values);
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Resistor_Capacitor_Series;
                array_add(*impedance_updates, tprint("// Computing impedance for: %1;", token.name));
                array_add(*impedance_updates, write_define(tprint("%1_res_value", token.name), tprint("params.%1;", token.param_names[0])));
                array_add(*impedance_updates, write_define(tprint("%1_cap_value", token.name), tprint("params.%1;", token.param_names[1])));
                add_impedance(*impedance_updates, tprint("%1_R", token.name), tprint("(T / (2 * %1_cap_value)) + %1_res_value", token.name), required_impedances.*, *available_values);
                add_impedance(*impedance_updates, tprint("%1_G", token.name), tprint("1 / %1_R", token.name), required_impedances.*, *available_values);
                array_add(*impedance_updates, tprint("impedances.%1_T_over_T_plus_2RC = T / (2 * %1_cap_value * %1_res_value + T);\n", token.name));
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Resistor_Capacitor_Parallel;
                array_add(*impedance_updates, tprint("// Computing impedance for: %1;", token.name));
                array_add(*impedance_updates, write_define(tprint("%1_res_value", token.name), tprint("params.%1;", token.param_names[0])));
                array_add(*impedance_updates, write_define(tprint("%1_cap_value", token.name), tprint("params.%1;", token.param_names[1])));
                array_add(*impedance_updates, write_define(tprint("%1_2RC", token.name), tprint("2 * %1_cap_value * %1_res_value;", token.name)));
                add_impedance(*impedance_updates, tprint("%1_R", token.name), tprint("%1_res_value * T / (%1_2RC + T)", token.name), required_impedances.*, *available_values);
                add_impedance(*impedance_updates, tprint("%1_G", token.name), tprint("1 / %1_R", token.name), required_impedances.*, *available_values);
                array_add(*impedance_updates, tprint("impedances.%1_twoRC_over_twoRC_plus_T = %1_2RC / (%1_2RC + T);\n", token.name));
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Ideal_Voltage_Source;
                if ! is_input(token, inputs) {
                    is_known := is_param_defined(token, 0);
                    if ! is_known {
                        array_add(*impedance_updates, tprint("// Computing voltage for: %1;", token.name));
                        array_add(*impedance_updates, tprint("impedances.%1_V = params.%1_v_value;\n", token.name));
                    }
                }

            case .Ideal_Current_Source;
                if ! is_input(token, inputs) {
                    is_known := is_param_defined(token, 0);
                    if ! is_known {
                        array_add(*impedance_updates, tprint("// Computing current for: %1;", token.name));
                        array_add(*impedance_updates, tprint("impedances.%1_I = params.%1_i_value;\n", token.name));
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
                add_impedance(*impedance_updates, tprint("%1_R", token.name), tprint("params.%1_res_value", token.name), required_impedances.*, *available_values);
                add_impedance(*impedance_updates, tprint("%1_G", token.name), tprint("1 / %1_R", token.name), required_impedances.*, *available_values);
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Capacitive_Voltage_Source;
                array_add(*impedance_updates, tprint("// Computing impedance for: %1;", token.name));
                add_impedance(*impedance_updates, tprint("%1_G", token.name), tprint("2 * params.%1_cap_value * fs", token.name), required_impedances.*, *available_values);
                add_impedance(*impedance_updates, tprint("%1_R", token.name), tprint("1 / %1_G", token.name), required_impedances.*, *available_values);
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
                error_assert (false, "NOT IMPLEMENTED (generate impedance updates for custom leaf node)");
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

    return impedance_updates;
}

generate_circuit :: (tokens: [] Token, inputs: [] string, required_impedances: [] string) -> string {
    circuit_vars := generate_circuit_vars(tokens, inputs, required_impedances);


    gen_code := generate_struct(circuit_vars.params_vars, "Params");
    gen_code = tprint("%\n%", gen_code, generate_struct(circuit_vars.impedance_vars, "Impedances"));
    gen_code = tprint("%\n%", gen_code, generate_struct(circuit_vars.state_vars, "State"));

    return gen_code;
}

generate_updaters :: (tokens: [] Token,
                      inputs: [] string,
                      meta_info: Meta_Info,
                      required_impedances: *[..] string) -> string {
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

    return gen_code;
}

reduce_circuit :: (original_tokens: [] Token, outputs: [] string) -> [] Token {
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

    make_rc_series :: (original_token: Token, idx: s64, res: Token, cap: Token) -> Token {
        new_args : [..] string;
        array_add(*new_args, res.args[0]);
        array_add(*new_args, cap.args[0]);
        param_names : [..] string;
        array_add(*param_names, tprint("%_value", res.name));
        array_add(*param_names, tprint("%_value", cap.name));
        return  Token.{
            name = original_token.name,
            type = .Resistor_Capacitor_Series,
            args = new_args,
            parent = original_token.parent,
            tree_depth = original_token.tree_depth,
            index = idx,
            param_names = param_names,
        };
    }
    make_rc_parallel :: (original_token: Token, idx: s64, res: Token, cap: Token) -> Token {
        new_args : [..] string;
        array_add(*new_args, res.args[0]);
        array_add(*new_args, cap.args[0]);
        param_names : [..] string;
        array_add(*param_names, tprint("%_value", res.name));
        array_add(*param_names, tprint("%_value", cap.name));
        return  Token.{
            name = original_token.name,
            type = .Resistor_Capacitor_Parallel,
            args = new_args,
            parent = original_token.parent,
            tree_depth = original_token.tree_depth,
            index = idx,
            param_names = param_names,
        };
    }

    // @TODO: what if we want to combine circuits, where 1 element is a parameter?
    // (e.g. 2 resistors in parallel, where 1 is a potentiometer)
    // @TODO: what to do if combined element is an input?

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

                    // RCSeries
                    if (child0.type == .Resistor && child1.type == .Capacitor && ! is_child_output) {
                        tokens[idx] = make_rc_series(token, idx, child0, child1);
                        remove_token(*tokens, token.children[1]);
                        remove_token(*tokens, token.children[0]);
                        reductions_count += 1;
                    }
                    if (child1.type == .Resistor && child0.type == .Capacitor && ! is_child_output) {
                        tokens[idx] = make_rc_series(token, idx, child1, child0);
                        remove_token(*tokens, token.children[1]);
                        remove_token(*tokens, token.children[0]);
                        reductions_count += 1;
                    }

                case .Parallel;
                    child0 := tokens[token.children[0]];
                    child1 := tokens[token.children[1]];

                    is_child_output := false;
                    for output : outputs {
                        output_info := String.split(output, ":");
                        is_child_output |= child0.name == output_info[1];
                        is_child_output |= child1.name == output_info[1];
                    }

                    // RCParallel
                    if (child0.type == .Resistor && child1.type == .Capacitor && ! is_child_output) {
                        tokens[idx] = make_rc_parallel(token, idx, child0, child1);
                        remove_token(*tokens, token.children[1]);
                        remove_token(*tokens, token.children[0]);
                        reductions_count += 1;
                    }
                    if (child1.type == .Resistor && child0.type == .Capacitor && ! is_child_output) {
                        tokens[idx] = make_rc_parallel(token, idx, child1, child0);
                        remove_token(*tokens, token.children[1]);
                        remove_token(*tokens, token.children[0]);
                        reductions_count += 1;
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
