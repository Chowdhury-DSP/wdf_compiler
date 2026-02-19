#import "Basic";
String :: #import "String";
Sort :: #import "Sort";
#load "utils.jai";

Token_Type :: enum {
    Ideal_Voltage_Source;
    Ideal_Current_Source;
    Resistive_Voltage_Source;
    Resistive_Current_Source;
    Resistor;
    Capacitor;
    Capacitor_Alpha;
    Inductor;
    Series;
    Parallel;
    Inverter;
    Switch;
    Resistor_Capacitor_Series;
    Resistor_Capacitor_Parallel;
    Capacitive_Voltage_Source;
    Resistive_Capacitive_Voltage_Source;
    Root_Resistive_Voltage_Source;
    Root_Capacitive_Voltage_Source;
    Root_Capacitive_Voltage_Source_par_Resistor;
    Custom;
}

Token :: struct {
    name: string;
    type: Token_Type;
    args: [] string; // Args provided to the token in WDF config
    parent: s64;
    tree_depth: s64 = 0;
    children: [..] s64;
    input: string;
    index: s64 = 0;
    param_names: [] string; // Parameter names that either become part of the generated code, or are used for some intermediate calculation(s)

    Param_Calc_Info :: struct {
        Calc_Type :: enum {
            Sum;
            Reciprocal_Sum;
        }
        param_name_idx: s64 = -1;
        calc: Calc_Type;
        child_left: *Param_Calc_Info = null;
        child_right: *Param_Calc_Info = null;
    }
    param_calc_info: [] Param_Calc_Info;
    invert := false;

    Custom_Token_Info :: struct {
        params_type: string;
        state_type: string;
        vars_type: string;
        updater_name: string;
        compute_name: string; // only if root
        incident_name: string; // only if not root
        reflected_name: string; // only if not root
    }
    custom_info: *Custom_Token_Info = null;
}

parse_custom_token_args :: (args: [] string) -> *Token.Custom_Token_Info {
    info := New(Token.Custom_Token_Info);
    name_prefix := "";
    for arg : args {
        found_key, key, value := String.split_from_left(arg, cast(u8) #char "=");
        if ! found_key { continue; }
        if key == {
            case "params";
                info.params_type = value;
            case "state";
                info.state_type = value;
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
            case "prefix";
                name_prefix = value;
        }
        #if #exists(global_args) {
            if key == {
                case "cpp_prefix";
                    if global_args.lang == "cpp" { name_prefix = value; }
                case "c_prefix";
                    if global_args.lang == "c" { name_prefix = value; }
                case "jai_prefix";
                    if global_args.lang == "jai" { name_prefix = value; }
                case "rust_prefix";
                    if global_args.lang == "rust" { name_prefix = value; }
            }
        }
    }

    if name_prefix.count > 0 {
        add_prefix :: (name: string, prefix: string) -> string {
            if name.count == 0 { return name; }
            return tprint("%1%2", prefix, name);
        }
        info.params_type = add_prefix (info.params_type, name_prefix);
        info.state_type = add_prefix (info.state_type, name_prefix);
        info.vars_type = add_prefix (info.vars_type, name_prefix);
        info.updater_name = add_prefix (info.updater_name, name_prefix);
        info.compute_name = add_prefix (info.compute_name, name_prefix);
        info.incident_name = add_prefix (info.incident_name, name_prefix);
        info.reflected_name = add_prefix (info.reflected_name, name_prefix);
    }

    return info;
}

parse_circuit :: (circuit_str: string) -> [] Token {
    make_array :: ($T: Type, count: s64) -> [] T {
        array: []T;
        array.data = cast(*T) alloc(count * size_of(T));
        array.count = count;
        return array;
    }

    resize_array :: (array: *[] $T, new_count: s64) {
        if array.count == 0 {
            array.* = make_array(T, new_count);
            return;
        }
        array_resize(array, new_count);
    }

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
            case "Ca"; new_token.type = .Capacitor_Alpha;
            case "L"; new_token.type = .Inductor;
            case "RVs"; new_token.type = .Resistive_Voltage_Source;
            case "CVs"; new_token.type = .Capacitive_Voltage_Source;
            case "RCVs"; new_token.type = .Resistive_Capacitive_Voltage_Source;
            case "RIs"; new_token.type = .Resistive_Current_Source;
            case "RCSeries"; new_token.type = .Resistor_Capacitor_Series;
            case "RCParallel"; new_token.type = .Resistor_Capacitor_Parallel;
            case "IVs";
                new_token.type = .Ideal_Voltage_Source;
                num_children = 1;
            case "IIs";
                new_token.type = .Ideal_Current_Source;
                num_children = 1;
            case "Switch";
                new_token.type = .Switch;
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
            case "Root_RVs";
                new_token.type = .Root_Resistive_Voltage_Source;
                num_children = 1;
            case "Root_CVs";
                new_token.type = .Root_Capacitive_Voltage_Source;
                num_children = 1;
            case "Root_CVs_par_R";
                new_token.type = .Root_Capacitive_Voltage_Source_par_Resistor;
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

        // param names
        if new_token.type == {
            case .Resistor; #through;
            case .Capacitor; #through;
            case .Inductor;
                param_names := make_array(string, 1);
                param_names[0] = tprint("%_value", new_token.name);
                new_token.param_names = param_names;

            case .Resistor_Capacitor_Series; #through;
            case .Resistor_Capacitor_Parallel;
                param_names := make_array(string, 2);
                param_names[0] = tprint("%_res_value", new_token.name);
                param_names[1] = tprint("%_cap_value", new_token.name);
                new_token.param_names = param_names;

            case .Ideal_Voltage_Source;
                param_names := make_array(string, 1);
                param_names[0] = tprint("%_v_value", new_token.name);
                new_token.param_names = param_names;

                if new_token.args.count == 0 {
                    resize_array(*new_token.args, 1);
                    new_token.args[0] = "?";
                }

            case .Switch;
                param_names := make_array(string, 1);
                param_names[0] = tprint("%_closed", new_token.name);
                new_token.param_names = param_names;

                if new_token.args.count == 0 {
                    resize_array(*new_token.args, 1);
                    new_token.args[0] = "?";
                }

            case .Root_Resistive_Voltage_Source; #through;
            case .Resistive_Voltage_Source;
                param_names := make_array(string, 2);
                param_names[0] = tprint("%_v_value", new_token.name);
                param_names[1] = tprint("%_res_value", new_token.name);
                new_token.param_names = param_names;

                if new_token.args.count == 1 {
                    res_arg := new_token.args[0];
                    resize_array(*new_token.args, 2);
                    new_token.args[0] = "?";
                    new_token.args[1] = res_arg;
                }

            case .Root_Capacitive_Voltage_Source; #through;
            case .Capacitive_Voltage_Source;
                param_names := make_array(string, 2);
                param_names[0] = tprint("%_v_value", new_token.name);
                param_names[1] = tprint("%_cap_value", new_token.name);
                new_token.param_names = param_names;

                if new_token.args.count == 1 {
                    cap_arg := new_token.args[0];
                    resize_array(*new_token.args, 2);
                    new_token.args[0] = "?";
                    new_token.args[1] = cap_arg;
                }

            case .Resistive_Capacitive_Voltage_Source;
                error_assert(new_token.args.count >= 2, "RCVs requires at least 2 arguments");

                param_names := make_array(string, 3);
                param_names[0] = tprint("%_v_value", new_token.name);
                param_names[1] = tprint("%_res_value", new_token.name);
                param_names[2] = tprint("%_cap_value", new_token.name);
                new_token.param_names = param_names;

                if new_token.args.count == 2 {
                    res_arg := new_token.args[0];
                    cap_arg := new_token.args[1];
                    resize_array(*new_token.args, 3);
                    new_token.args[0] = "?";
                    new_token.args[1] = res_arg;
                    new_token.args[2] = cap_arg;
                }

            case .Root_Capacitive_Voltage_Source_par_Resistor;
                error_assert(new_token.args.count >= 2, "Root_CVs_par_R requires at least 2 arguments");

                param_names := make_array(string, 3);
                param_names[0] = tprint("%_v_value", new_token.name);
                param_names[1] = tprint("%_res_value", new_token.name);
                param_names[2] = tprint("%_cap_value", new_token.name);
                new_token.param_names = param_names;

                if new_token.args.count == 2 {
                    res_arg := new_token.args[0];
                    cap_arg := new_token.args[1];
                    resize_array(*new_token.args, 3);
                    new_token.args[0] = "?";
                    new_token.args[1] = res_arg;
                    new_token.args[2] = cap_arg;
                }
        }
        resize_array(*new_token.param_calc_info, new_token.param_names.count);
        for name,idx : new_token.param_names {
            new_token.param_calc_info[idx].param_name_idx = idx;
        }
        if new_token.param_names.count > 0 { // @TODO: do we want to do this for all elements?
            old_count := new_token.args.count;
            resize_array(*new_token.args, new_token.param_names.count);
            for idx : old_count..new_token.args.count-1 {
                new_token.args[idx] = "?";
            }
        }

        if (new_token.type == .Custom) {
            num_children = string_to_int(new_token.args[0]);
            new_token.args.data = new_token.args.data + 1;
            new_token.args.count = new_token.args.count - 1;
            new_token.custom_info = parse_custom_token_args(new_token.args);
            verbose_print("Custom token: %\n", new_token.custom_info.*);
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
    verbose_print("Tokens: %\n", tokens);
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
