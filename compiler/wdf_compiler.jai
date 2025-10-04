#import "Basic";
CLI :: #import "Command_Line";
File :: #import "File";
String :: #import "String";
Sort :: #import "Sort";

Args :: struct {
    verbose : bool = false; @"?Verbose"
}

error_assert :: (cond: bool, message: string) #expand {
    if ! cond {
        print("Compilation error: %\n", message);
        exit(1);
    }
}

main :: () {
    // context setup...
    context.allocator = temp;
    context.print_style.default_format_float.mode = .SCIENTIFIC;

    args_success, args, is_set, free_args := CLI.parse_arguments (Args);

    if !args_success || free_args.count < 2 {
        print("Incorrect arguments! Usage: wdf_compiler <input_file> <output_file>\n");
        // CLI.show_help (Args, CLI.Default_Argument_Flags, .[]);
        exit (1);
    }

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
    assert(inputs.count == 1); // @TODO: support multiple inputs
    outputs := String.split(sections.outputs, "\n");

    meta_info := parse_meta_info(sections.meta);
    gen_code := start_gen_code(meta_info);
    circuit_tokens := parse_circuit(sections.circuit);
    gen_code = tprint("%\n%", gen_code, generate_circuit(circuit_tokens, inputs));
    gen_code = tprint("%\n%", gen_code, generate_updaters(circuit_tokens, inputs, meta_info));
    gen_code = tprint("%\n%", gen_code, generate_process(circuit_tokens, inputs, outputs));
    gen_code = tprint("%\n%", gen_code, end_gen_code(meta_info));

    gen_file, success := File.file_open(output_file, true);
    File.file_write(*gen_file, gen_code);
    // print(gen_code);
}

start_gen_code :: (meta_info: Meta_Info) -> string {
    // print("%\n", meta_info);
    gen_code := "#pragma once\n";

    if (meta_info.extra_includes.count > 0) {
        includes_code := "";
        for include : meta_info.extra_includes {
            includes_code = tprint("%#include %\n", includes_code, include);
        }
        gen_code = tprint("%\n%", gen_code, includes_code);
    }

    if (meta_info.namespace.count > 0) {
        gen_code = tprint("%\nnamespace %\n{\n", gen_code, meta_info.namespace);
    }
    return gen_code;
}

end_gen_code :: (meta_info: Meta_Info) -> string {
    gen_code := "";
    if (meta_info.namespace.count > 0) {
        gen_code = tprint("} // namespace %\n\n", meta_info.namespace);
    }
    return gen_code;
}

Meta_Info :: struct {
    namespace := "";
    impedance_updaters : [..] string;
    extra_includes : [..] string;
}

parse_meta_info :: (meta_info_str: string) -> Meta_Info {
    meta_info : Meta_Info;

    info_lines := String.split(meta_info_str, cast(u8) #char "\n");
    for line : info_lines {
        line_trim := String.trim(line);
        tokens := String.split(line_trim, cast(u8) #char ":");
        if tokens.count != 2 { continue; }

        key := String.trim(tokens[0]);
        value := String.trim(tokens[1]);

        if key == {
            case "namespace";
                meta_info.namespace = value;
            case "calc_impedances";
                array_add(*meta_info.impedance_updaters, value);
            case "include";
                array_add(*meta_info.extra_includes, value);
        }
    }

    // print("%\n", meta_info);
    return meta_info;
}

Token_Type :: enum {
        Ideal_Voltage_Source;
        Ideal_Current_Source;
        Resistive_Voltage_Source;
        Resistive_Current_Source;
        Resistor;
        Capacitor;
        Inductor;
        Series;
        Parallel;
        Inverter;
        Resistor_Capacitor_Series;
        Resistor_Capacitor_Parallel;
        Capacitive_Voltage_Source;
        Custom;
    }

Token :: struct {
    name: string;
    type: Token_Type;
    args: [] string;
    parent: s64;
    tree_depth: s64 = 0;
    children: [..] s64;
    input: string;
    index: s64 = 0;

    Custom_Token_Info :: struct {
        params_type: string;
        state_type: string;
        vars_type: string;
        updater_name: string;
        compute_name: string; // only if root
        incident_name: string; // only if not root
        reflected_name: string; // only if not root
    }
    custom_info: *Custom_Token_Info;
}

generate_struct :: (impedance_vars: [] string, name: string) -> string {
    struct_code := tprint("struct % {\n", name);
    for var : impedance_vars {
        struct_code = tprint("%    %\n", struct_code, var);
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
    error_assert (success, tprint("Unable to parse value string: %", val_str));
    if remainder.count == 0 { return val; }
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

generate_process :: (tokens: [] Token, inputs: [] string, outputs: [] string) -> string {
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
        error_assert(input_token != null, tprint("Unable to find input circuit element: %", input));
        array_add(*input_args, tprint("float %", input));

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

    down_tree_statements : [..] string;
    up_tree_statements : [..] string;
    root_token := *tokens[0];
    for *token : tokens {
        // print("%\n", token.*);

        if token == root_token {
            if token.type == {
                case .Ideal_Voltage_Source;
                    child_token := tokens[token.children[0]];
                    array_add(*up_tree_statements, tprint("const auto %1_a = %2_b; // %1 incident", token.name, child_token.name));
                    array_add(*down_tree_statements, tprint("const auto %1_b = -%1_a + 2 * %2; // %1 reflected", token.name, token.input));
                    array_add(*down_tree_statements, tprint("const auto %2_a = %1_b; // %2 incident", token.name, child_token.name));
                case .Ideal_Current_Source;
                    child_token := tokens[token.children[0]];
                    array_add(*up_tree_statements, tprint("const auto %1_a = %2_b; // %1 incident", token.name, child_token.name));
                    array_add(*down_tree_statements, tprint("const auto %1_b = 2 * impedances.%3_R * %2 + %1_a; // %1 reflected (@TODO: 2 * R?)", token.name, token.input, child_token.name));
                    array_add(*down_tree_statements, tprint("const auto %2_a = %1_b; // %2 incident", token.name, child_token.name));
                case .Custom;
                    num_children := token.children.count;
                    has_state := token.custom_info.state_type.count > 0;
                    has_vars := token.custom_info.vars_type.count > 0;
                    if num_children == 1 {
                        child_token := tokens[token.children[0]];
                        args := "";
                        if has_vars { args = tprint("%1impedances.%2_vars, ", args, token.name); }
                        if has_state { args = tprint("%1impedances.%2_state, ", args, token.name); }
                        array_add(*down_tree_statements, tprint("const auto %2_a = %4(%3%2_b); // %2 incident", token.name, child_token.name, args, token.custom_info.compute_name));
                    } else {
                        child_token := tokens[token.children[0]];

                        // declare inputs
                        // @TODO: make the alignment here configurable?
                        statement := tprint("alignas (16) const float %_ins[%] { ", token.name, num_children);
                        for child_idx : token.children {
                            statement = tprint("%1%2_b, ", statement, tokens[child_idx].name);
                        }
                        statement.count -= 2;
                        statement = tprint("%1 }; // % inputs\n", statement, token.name);

                        // declare outputs
                        statement = tprint("%    alignas (16) float %_outs[%]; // %2 outputs \n", statement, token.name, num_children);

                        args := "";
                        if has_vars { args = tprint("%1impedances.%2_vars, ", args, token.name); }
                        if has_state { args = tprint("%1impedances.%2_state, ", args, token.name); }
                        args = tprint("%1%2_ins, %2_outs", args, token.name);
                        statement = tprint("%1    %2(%3); // %4 incident\n", statement, token.custom_info.compute_name, args, token.name);

                        // unwrap outputs
                        for child_idx : token.children {
                            statement = tprint("%1    const auto %2_a = %3_outs[%4];\n", statement, tokens[child_idx].name, token.name, it_index);
                        }
                        statement.count -= 1;

                        array_add(*down_tree_statements, statement);
                    }
            }
            continue;
        }

        if token.type == {
            case .Resistor;
                array_add(*up_tree_statements, tprint("const auto %1_b = 0; // %1 reflected", token.name));
            case .Capacitor;
                array_add(*up_tree_statements, tprint("const auto %1_b = state.%1_z; // %1 reflected", token.name));
                array_add(*down_tree_statements, tprint("state.%1_z = %1_a; // %1 state update", token.name));
            case .Inductor;
                array_add(*up_tree_statements, tprint("const auto %1_b = -state.%1_z; // %1 reflected", token.name));
                array_add(*down_tree_statements, tprint("state.%1_z = %1_a; // %1 state update", token.name));
            case .Resistor_Capacitor_Series;
                array_add(*up_tree_statements, tprint("const auto %1_b = -state.%1_z; // %1 reflected", token.name));
                array_add(*down_tree_statements, tprint("state.%1_z -= impedances.%1_T_over_T_plus_2RC * (%1_a + state.%1_z); // %1 state update", token.name));
            case .Resistor_Capacitor_Parallel;
                array_add(*up_tree_statements, tprint("const auto %1_b = impedances.%1_twoRC_over_twoRC_plus_T * state.%1_z; // %1 reflected", token.name));
                array_add(*down_tree_statements, tprint("state.%1_z = %1_b + %1_a - state.%1_z; // %1 state update", token.name));
            case .Resistive_Voltage_Source;
                array_add(*up_tree_statements, tprint("const auto %1_b = %2; // %1 reflected", token.name, token.input));
            case .Resistive_Current_Source;
                array_add(*up_tree_statements, tprint("const auto %1_b = impedances.%1_R * %2; // %1 reflected", token.name, token.input));
            case .Capacitive_Voltage_Source;
                array_add(*up_tree_statements, tprint("state.%1_v1 = %2; // %1 state update", token.name, token.input));
                array_add(*up_tree_statements, tprint("const auto %1_b = state.%1_z + %2 - state.%1_v1; // %1 reflected", token.name, token.input));
                array_add(*down_tree_statements, tprint("state.%1_z = %1_a; // %1 state update", token.name));
            case .Series;
                child0_token := tokens[token.children[0]];
                child1_token := tokens[token.children[1]];
                array_add(*up_tree_statements, tprint("const auto %1_b = -(%2_b + %3_b); // %1 reflected", token.name, child0_token.name, child1_token.name));
                array_add(*down_tree_statements, tprint("const auto %2_a = %2_b - impedances.%1_pr * (%1_a + %2_b + %3_b); // %2 incident", token.name, child0_token.name, child1_token.name));
                array_add(*down_tree_statements, tprint("const auto %3_a = -(%1_a + %2_a); // %3 incident", token.name, child0_token.name, child1_token.name));
            case .Parallel;
                child0_token := tokens[token.children[0]];
                child1_token := tokens[token.children[1]];
                array_add(*up_tree_statements, tprint("const auto %1_b = %3_b - impedances.%1_pr * (%3_b - %2_b); // %1 reflected", token.name, child0_token.name, child1_token.name));
                array_add(*down_tree_statements, tprint("const auto %2_a = %1_b - %2_b + %1_a; // %2 incident", token.name, child0_token.name));
                array_add(*down_tree_statements, tprint("const auto %3_a = %1_b - %3_b + %1_a; // %3 incident", token.name, child0_token.name, child1_token.name));
            case .Inverter;
                child_token := tokens[token.children[0]];
                array_add(*up_tree_statements, tprint("const auto %1_b = -%2_b; // %1 reflected", token.name, child_token.name));
                array_add(*down_tree_statements, tprint("const auto %2_a = -%1_a; // %2 incident", token.name, child_token.name));
            case .Custom;
                error_assert (false, "NOT IMPLEMENTED (generate process for custom non-root node)");
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

        if output_info[0] == {
            case "V";
                return_var := tprint("v_%", output_token.name);
                array_add(*code_statements, tprint("const auto %2 = (%1_a + %1_b) * 0.5f; // %1 voltage", output_token.name, return_var));
                array_add(*return_vars, return_var);
            case "I";
                return_var := tprint("i_%", output_token.name);
                array_add(*code_statements, tprint("const auto %2 = (%1_a - %1_b) * (0.5f * impedances.%1_G); // %1 current", output_token.name, return_var));
                array_add(*return_vars, return_var);
            case;
                error_assert(false, tprint("Invalid output type: %", output_info[0]));
        }
    }
    if return_vars.count == 1 {
        array_add(*code_statements, tprint("return %;", return_vars[0]));
    } else {
        return_statement := "return std::make_tuple(";
        for var : return_vars {
            return_statement = tprint("%1%2, ", return_statement, var);
        }
        return_statement.count -= 2;
        return_statement = tprint("%);", return_statement);
        array_add(*code_statements, return_statement);
    }

    return generate_method("process",
                           input_args,
                           code_statements,
                           "auto");
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

write_param :: (params_vars : *[..] string, token: *Token, idx := 0, suffix := "value") -> bool {
    is_defined_arg := is_param_defined(token, idx);
    if is_defined_arg {
        array_add(params_vars, tprint("static constexpr float %1_%3 = %2;", token.name, get_value(token.args[idx]), suffix));
    } else {
        array_add(params_vars, tprint("float %1_%2;", token.name, suffix));
    }

    return is_defined_arg;
}

Circuit_Vars :: struct {
    state_vars : [..] string;
    impedance_vars : [..] string;
    params_vars : [..] string;
}

generate_circuit_vars :: (tokens: [] Token, inputs: [] string) -> Circuit_Vars {
    state_vars : [..] string;
    impedance_vars : [..] string;
    params_vars : [..] string;

    impedance_update_todos : [..] *Token;
    root_token := *tokens[0];
    for *token : tokens {
        if token != root_token {
            array_add(*impedance_vars, tprint("float %_R;", token.name));
            array_add(*impedance_vars, tprint("float %_G;", token.name));
            // @TODO: we don't always need to "save" all of these values?
        }

        if token.type == {
            case .Resistor;
                write_param(*params_vars, token);
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Capacitor;
                write_param(*params_vars, token);
                array_add(*state_vars, tprint("float %_z;", token.name));
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Inductor;
                write_param(*params_vars, token);
                array_add(*state_vars, tprint("float %_z;", token.name));
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Resistor_Capacitor_Series;
                write_param(*params_vars, token, 0, "res_value");
                write_param(*params_vars, token, 1, "cap_value");
                array_add(*state_vars, tprint("float %_z;", token.name));
                array_add(*impedance_vars, tprint("float %1_T_over_T_plus_2RC;", token.name));
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Resistor_Capacitor_Parallel;
                write_param(*params_vars, token, 0, "res_value");
                write_param(*params_vars, token, 1, "cap_value");
                array_add(*state_vars, tprint("float %_z;", token.name));
                array_add(*impedance_vars, tprint("float %1_twoRC_over_twoRC_plus_T;", token.name));
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Ideal_Voltage_Source;
                if ! is_input(token, inputs) {
                    is_known := write_param(*params_vars, token, 0, "v_value");
                    if is_known {
                        array_add(*impedance_vars, tprint("static constexpr float %_V = Params::%1_v_value;", token.name));
                    } else {
                        array_add(*impedance_vars, tprint("float %_V;", token.name));
                    }
                    token.input = tprint("impedances.%1_V", token.name);
                }

            case .Ideal_Current_Source;
                if ! is_input(token, inputs) {
                    is_known := write_param(*params_vars, token, 0, "i_value");
                    if is_known {
                        array_add(*impedance_vars, tprint("static constexpr float %_I = Params::%1_i_value;", token.name));
                    } else {
                        array_add(*impedance_vars, tprint("float %_I;", token.name));
                    }
                    token.input = tprint("impedances.%1_I", token.name);
                }

            case .Resistive_Voltage_Source;
                is_input_source := is_input(token, inputs);

                if ! is_input_source {
                    is_known := write_param(*params_vars, token, 0, "v_value");
                    if is_known {
                        array_add(*impedance_vars, tprint("static constexpr float %_V = Params::%1_v_value;", token.name));
                    } else {
                        array_add(*impedance_vars, tprint("float %_V;", token.name));
                    }
                    token.input = tprint("impedances.%1_V", token.name);
                }

                if is_input_source {
                    write_param(*params_vars, token, 0, "res_value");
                } else {
                    write_param(*params_vars, token, 1, "res_value");
                }
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Capacitive_Voltage_Source;
                is_input_source := is_input(token, inputs);
                error_assert(is_input_source, tprint("Capacitive Voltage Source % is required to be an input source!", token.name));

                array_add(*state_vars, tprint("float %_z;", token.name));
                array_add(*state_vars, tprint("float %_v1;", token.name));
                write_param(*params_vars, token, 0, "cap_value");
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Resistive_Current_Source;
                is_input_source := is_input(token, inputs);
                if ! is_input(token, inputs) {
                    is_known := write_param(*params_vars, token, 0, "i_value");
                    if is_known {
                        array_add(*impedance_vars, tprint("static constexpr float %_I = Params::%1_i_value;", token.name));
                    } else {
                        array_add(*impedance_vars, tprint("float %_I;", token.name));
                    }
                    token.input = tprint("impedances.%1_I", token.name);
                }

                if is_input_source {
                    write_param(*params_vars, token, 0, "res_value");
                } else {
                    write_param(*params_vars, token, 1, "res_value");
                }
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Custom;
                if token.children.count > 0 { continue; }
                error_assert (false, "NOT IMPLEMENTED (generate impedance updates for custom non-root node)");
                // @TODO: implement me!
        }
    }

    while impedance_update_todos.count > 0 {
        Sort.quick_sort (impedance_update_todos, x => x.tree_depth);
        impedance_todo := impedance_update_todos[impedance_update_todos.count - 1];
        token := impedance_todo;
        if token.type == {
            case .Series;
                array_add(*impedance_vars, tprint("float %1_pr;", token.name));
            case .Parallel;
                array_add(*impedance_vars, tprint("float %1_pr;", token.name));
            case .Custom;
                if (token.custom_info.params_type.count > 0) {
                    array_add(*params_vars, tprint("%1 %2_params {};", token.custom_info.params_type, token.name));
                }
                if (token.custom_info.state_type.count > 0) {
                    array_add(*state_vars, tprint("%1 %2_state {};", token.custom_info.state_type, token.name));
                }
                if (token.custom_info.vars_type.count > 0) {
                    array_add(*impedance_vars, tprint("%1 %2_vars {};", token.custom_info.vars_type, token.name));
                }
        }

        // This is leaky, but I don't think we care?
        impedance_update_todos.count -= 1;

        if token != root_token {
            add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token, false);
        } else {
            // print("Finished climbing impedance tree!");
        }
    }

    return Circuit_Vars.{state_vars=state_vars, impedance_vars=impedance_vars, params_vars=params_vars};
}

generate_updater :: (tokens: [] Token, inputs: [] string, tokens_filter: [] string = .[]) -> [..] string {
    impedance_updates : [..] string;
    impedance_update_todos : [..] *Token;
    array_add(*impedance_updates, "[[maybe_unused]] const auto T = 1.0f / fs;\n");

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
                array_add(*impedance_updates, tprint("impedances.%1_R = params.%1_value;", token.name));
                array_add(*impedance_updates, tprint("impedances.%1_G = 1.0f / impedances.%1_R;\n", token.name));
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Capacitor;
                array_add(*impedance_updates, tprint("// Computing impedance for: %1;", token.name));
                array_add(*impedance_updates, tprint("impedances.%1_G = 2.0f * params.%1_value * fs;", token.name));
                array_add(*impedance_updates, tprint("impedances.%1_R = 1.0f / impedances.%1_G;\n", token.name));
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Inductor;
                array_add(*impedance_updates, tprint("// Computing impedance for: %1;", token.name));
                array_add(*impedance_updates, tprint("impedances.%1_R = 2.0f * params.%1_value * fs;", token.name));
                array_add(*impedance_updates, tprint("impedances.%1_G = 1.0f / impedances.%1_R;\n", token.name));
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Resistor_Capacitor_Series;
                array_add(*impedance_updates, tprint("// Computing impedance for: %1;", token.name));
                array_add(*impedance_updates, tprint("impedances.%1_R = (T / (2.0f * params.%1_cap_value)) + params.%1_res_value;", token.name));
                array_add(*impedance_updates, tprint("impedances.%1_G = 1.0f / impedances.%1_R;", token.name));
                array_add(*impedance_updates, tprint("impedances.%1_T_over_T_plus_2RC = T / (2.0f * params.%1_cap_value * params.%1_res_value + T);\n", token.name));
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Resistor_Capacitor_Parallel;
                array_add(*impedance_updates, tprint("// Computing impedance for: %1;", token.name));
                array_add(*impedance_updates, tprint("const auto %1_2RC = 2.0f * params.%1_cap_value * params.%1_res_value;", token.name));
                array_add(*impedance_updates, tprint("impedances.%1_R = params.%1_res_value * T / (%1_2RC + T);", token.name));
                array_add(*impedance_updates, tprint("impedances.%1_G = 1.0f / impedances.%1_R;", token.name));
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
                array_add(*impedance_updates, tprint("impedances.%1_R = params.%1_res_value;", token.name));
                array_add(*impedance_updates, tprint("impedances.%1_G = 1.0f / impedances.%1_R;\n", token.name));
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Capacitive_Voltage_Source;
                array_add(*impedance_updates, tprint("// Computing impedance for: %1;", token.name));
                array_add(*impedance_updates, tprint("impedances.%1_G = 2.0f * params.%1_cap_value * fs;", token.name));
                array_add(*impedance_updates, tprint("impedances.%1_R = 1.0f / impedances.%1_G;\n", token.name));
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
                array_add(*impedance_updates, tprint("impedances.%1_R = params.%1_res_value;", token.name));
                array_add(*impedance_updates, tprint("impedances.%1_G = 1.0f / impedances.%1_R;\n", token.name));
                add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token);

            case .Custom;
                if token.children.count > 0 { continue; }
                error_assert (false, "NOT IMPLEMENTED (generate impedance updates for custom non-root node)");
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
                array_add(*impedance_updates, tprint("// Computing impedance for: %1;", token.name));
                array_add(*impedance_updates, tprint("impedances.%1_R = impedances.%2_R + impedances.%3_R;", token.name, child0_token.name, child1_token.name));
                array_add(*impedance_updates, tprint("impedances.%1_G = 1.0f / impedances.%1_R;", token.name));
                array_add(*impedance_updates, tprint("impedances.%1_pr = impedances.%2_R * impedances.%1_G;\n", token.name, child0_token.name));
            case .Parallel;
                child0_token := tokens[impedance_todo.children[0]];
                child1_token := tokens[impedance_todo.children[1]];
                array_add(*impedance_updates, tprint("// Computing impedance for: %1;", token.name));
                array_add(*impedance_updates, tprint("impedances.%1_G = impedances.%2_G + impedances.%3_G;", token.name, child0_token.name, child1_token.name));
                array_add(*impedance_updates, tprint("impedances.%1_R = 1.0f / impedances.%1_G;", token.name));
                array_add(*impedance_updates, tprint("impedances.%1_pr = impedances.%2_G * impedances.%1_R;\n", token.name, child0_token.name));
            case .Inverter;
                child_token := tokens[impedance_todo.children[0]];
                array_add(*impedance_updates, tprint("// Computing impedance for: %1;", token.name));
                array_add(*impedance_updates, tprint("impedances.%1_R = impedances.%2_R;", token.name, child_token.name));
                array_add(*impedance_updates, tprint("impedances.%1_G = 1.0f / impedances.%1_R;", token.name));
            case .Custom;
                has_params := token.custom_info.params_type.count > 0;
                has_state := token.custom_info.state_type.count > 0;
                has_vars := token.custom_info.vars_type.count > 0;

                array_add(*impedance_updates, tprint("// Computing impedance for: %1;", token.name));
                if token == root_token {
                    root_update := tprint("%1(", token.custom_info.updater_name);
                    if has_vars {
                        root_update = tprint("%1impedances.%2_vars, ", root_update, token.name);
                    }
                    if has_params {
                        root_update = tprint("%1params.%2_params, ", root_update, token.name);
                    }
                    for child : token.children {
                        root_update = tprint("%1impedances.%2_R, ", root_update, tokens[child].name);
                        root_update = tprint("%1impedances.%2_G, ", root_update, tokens[child].name);
                    }
                    root_update.count -= 2;
                    root_update = tprint("%1);", root_update);
                    array_add(*impedance_updates, root_update);
                } else {
                    error_assert (false, "NOT IMPLEMENTED (generate impedance updates for non-root node)");
                    // @TODO: we need to update the custom component's R and G here?
                }
        }

        // This is leaky, but I don't think we care?
        impedance_update_todos.count -= 1;

        if token != root_token {
            add_impedance_update_todo(*impedance_update_todos, *tokens[token.parent], token, false);
        } else {
            // print("Finished climbing impedance tree!");
        }
    }

    return impedance_updates;
}

generate_circuit :: (tokens: [] Token, inputs: [] string) -> string {
    circuit_vars := generate_circuit_vars(tokens, inputs);


    gen_code := generate_struct(circuit_vars.params_vars, "Params");
    gen_code = tprint("%\n%", gen_code, generate_struct(circuit_vars.impedance_vars, "Impedances"));
    gen_code = tprint("%\n%", gen_code, generate_struct(circuit_vars.state_vars, "State"));

    return gen_code;
}

generate_updaters :: (tokens: [] Token, inputs: [] string, meta_info: Meta_Info) -> string {
    // global updater
    impedance_updates := generate_updater(tokens, inputs);
    gen_code := generate_method("calc_impedances",
                                .["Impedances& impedances", "float fs", "Params params = {}"],
                                impedance_updates);

    // updaters from meta
    for updater : meta_info.impedance_updaters {
        parts := String.split(updater, cast(u8) #char "(");
        updater_name := parts[0];
        param_tokens := String.split(String.trim(parts[1], " )"), cast(u8) #char ",");
        for *t : param_tokens { t.* = String.trim(t.*); }
        updates := generate_updater(tokens, inputs, param_tokens);
        updater_method := generate_method(updater_name,
                                          .["Impedances& impedances", "float fs", "Params params"],
                                          updates);
        gen_code = tprint("%\n%", gen_code, updater_method);
    }

    return gen_code;
}

parse_custom_token_args :: (args: [] string) -> *Token.Custom_Token_Info {
    info := New(Token.Custom_Token_Info);
    for arg : args {
        found_key, key, value := String.split_from_left(arg, cast(u8) #char "=");
        if ! found_key { continue; }
        if key == {
            case "params";
                info.params_type = value;
            case "state";
                info.params_type = value;
            case "vars";
                info.vars_type = value;
            case "updater";
                info.updater_name = value;
            case "compute";
                info.compute_name = value;
            case "incident";
                info.incident_name = value;
            case "reflected";
                info.reflected_name = value;
        }
    }
    return info;
}

parse_circuit :: (circuit_str: string) -> [] Token {
    split_tokens := String.split(circuit_str, cast(u8) #char ";");

    tokens : [..] Token;
    Children :: struct {
        token_index: int;
        num_children: int;
        countdown: int;
    }
    children_stack: [..] Children;

    for *token : split_tokens {
        token_str := String.trim(token.*);
        if token_str.count == 0 { continue; }

        found_token_tag, tag, args := String.split_from_left(token_str, #char "(");
        // remove closing parenthese from args
        error_assert(args[args.count - 1] == #char ")", tprint("Token % is missing closing parenthese", tag));
        args.count = args.count - 1;

        num_children := 0;
        new_token: Token;
        if tag == {
            case "R"; new_token.type = .Resistor;
            case "C"; new_token.type = .Capacitor;
            case "L"; new_token.type = .Inductor;
            case "RVs"; new_token.type = .Resistive_Voltage_Source;
            case "CVs"; new_token.type = .Capacitive_Voltage_Source;
            case "RIs"; new_token.type = .Resistive_Current_Source;
            case "RCSeries"; new_token.type = .Resistor_Capacitor_Series;
            case "RCParallel"; new_token.type = .Resistor_Capacitor_Parallel;
            case "IVs";
                new_token.type = .Ideal_Voltage_Source;
                num_children = 1;
            case "IIs";
                new_token.type = .Ideal_Current_Source;
                num_children = 1;
            case "Series";
                new_token.type = .Series;
                num_children = 2;
            case "Parallel";
                new_token.type = .Parallel;
                num_children = 2;
            case "Inverter";
                new_token.type = .Inverter;
                num_children = 1;
            case "Custom";
                new_token.type = .Custom;
            case; error_assert(false, tprint("Unknown token type: %", tag));
        }

        args_split := String.split(args, cast(u8) #char ",");
        new_token.name = args_split[0];

        for old_token : tokens {
            error_assert(new_token.name != old_token.name, tprint("Duplicate token name: %", new_token.name));
        }

        new_token.args.data = args_split.data + 1;
        new_token.args.count = args_split.count - 1;
        for *arg : new_token.args {
            arg.* = String.trim(arg.*);
        }

        if (new_token.type == .Custom) {
            num_children = string_to_int(new_token.args[0]);
            new_token.args.data = new_token.args.data + 1;
            new_token.args.count = new_token.args.count - 1;
            new_token.custom_info = parse_custom_token_args(new_token.args);
            // print("%\n", new_token.custom_info.*);
        }

        if children_stack.count != 0 {
            parent_children := peek_pointer(children_stack);
            parent_children.countdown = parent_children.countdown - 1;
            assert(parent_children.countdown >= 0);
            new_token.parent = tokens[parent_children.token_index].index;
            new_token.tree_depth = tokens[new_token.parent].tree_depth + 1;
            array_add(*tokens[parent_children.token_index].children, tokens.count);
            if parent_children.countdown == 0 {
                pop(*children_stack);
            }
        }

        new_token.index = tokens.count;
        array_add(*tokens, new_token);

        if num_children != 0 {
            array_add(*children_stack, .{peek_pointer(tokens).index, num_children, num_children});
        }
    }
    // print("%\n", tokens);
    return tokens;
}

File_Sections :: struct {
    circuit: string;
    inputs: string;
    outputs: string;
    meta: string;
}

parse_file_sections :: (file_str: string) -> File_Sections {
    Section_Type :: enum {
        Circuit;
        Inputs;
        Outputs;
        Meta;
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
    add_section_info (file_str, *section_info, "meta:", .Meta);

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
            case .Meta; sections.meta = section_str;
        }
    }
    return sections;
}
