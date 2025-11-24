#import "Basic";
File :: #import "File";
#load "../../compiler/wdf_parser.jai";

Op :: enum {
    LOAD;
    STORE;
    ADD; // includes: add, subtract, and negate (for now)
    MUL;
    DIV;
    // @TODO: should we try to include fused operations?
}
OPS_COUNT :: #run enum_highest_value(Op);

Op_With_Count :: struct {
    op: Op;
    count: int;
}

Component_Info :: [] Op_With_Count;

TOKEN_COUNT :: #run enum_highest_value(Token_Type);
Component_Info_Table :: [TOKEN_COUNT] Component_Info;
generate_component_info_table :: () -> Component_Info_Table {
    combine :: (components: [] Component_Info) -> Component_Info {
        combined_info: [OPS_COUNT] Op_With_Count;
        for component_info : components {
            for op : component_info {
                combined_info[op.op].op = op.op;
                combined_info[op.op].count += op.count;
            }
        }
        return combined_info;
    }

    table : Component_Info_Table;

    using Token_Type;
    table[Ideal_Voltage_Source] = .[.{.MUL, 1}, .{.ADD, 1}];
    table[Ideal_Current_Source] = .[.{.MUL, 2}, .{.ADD, 1}];
    table[Resistive_Voltage_Source] = .[];  // this is a no-op
    table[Resistive_Current_Source] = .[.{.MUL, 1}];
    table[Resistor] = .[]; // this is a no-op
    table[Capacitor] = .[.{.LOAD, 1}, .{.STORE, 1}, .{.ADD, 2}];
    table[Inductor] = .[.{.LOAD, 1}, .{.STORE, 1}, .{.ADD, 3}];
    table[Series] = .[.{.LOAD, 1}, .{.MUL, 1}, .{.ADD, 4}];
    table[Parallel] = .[.{.LOAD, 1}, .{.MUL, 1}, .{.ADD, 2}];
    table[Inverter] = .[.{.ADD, 2}];

    table[Resistor_Capacitor_Series] = combine (.[
        table[Resistor],
        table[Capacitor],
        table[Series],
    ]);
    // @TODO: what should we do with "combined" and "custom" components?

    return table;
}

Op_Info :: struct {
    latency: float;
    throughput: float;
}

CPU_Type :: enum {
    M1;
    ZEN4;
}

Op_Info_Table :: [OPS_COUNT] Op_Info;
generate_op_info_table :: (cpu_type: CPU_Type) -> Op_Info_Table {

    table: Op_Info_Table;

    using Op;
    if (cpu_type == .M1) {
        // reference: https://dougallj.github.io/applecpu/firestorm.html
        table[LOAD]  = .{latency=2.5, throughput=0.333};
        table[STORE] = .{latency=0.5, throughput=1.0};
        table[ADD]   = .{latency=3.0, throughput=0.25};
        table[MUL]   = .{latency=4.0, throughput=0.25};
    } else if (cpu_type == .ZEN4) {
        // https://www.agner.org/optimize/instruction_tables.pdf
        table[LOAD]  = .{latency=5.0, throughput=0.5}; // MOVSS/D x,m32/64
        table[STORE] = .{latency=4.0, throughput=1.0}; // MOVSS/D m32/64,x
        table[ADD]   = .{latency=3.0, throughput=0.5};
        table[MUL]   = .{latency=3.0, throughput=0.5};
    }

    return table;
}
op_info_table :: #run generate_op_info_table(.ZEN4);

print_ops :: (ops_table: [] int) {
    ops_str := "Operations: [";
    for op_count, it : ops_table {
        if op_count > 0 { ops_str = tprint("%1%3 %2, ", ops_str, cast(Op) it, op_count); }
    }
    ops_str.count -= 2;
    print("%]\n", ops_str);
}

main :: () {
    print("WDF perf\n");

    args := get_command_line_arguments();
    wdf_file := args[1];
    print("Generating performance estimate for: %\n", wdf_file);

    file_str, fread_success := File.read_entire_file(wdf_file);
    if !fread_success { exit (1); }

    sections := parse_file_sections (file_str);
    outputs := String.split(sections.outputs, "\n");
    for *output : outputs { output.* = String.trim(output.*); }
    circuit_tokens := parse_circuit(sections.circuit);

    component_info_table := generate_component_info_table();

    add_cycles :: (throughput_counter: *float, latency_counter: *float, ops_table: [] int, op: Op_With_Count) {
        ops_table[op.op] += op.count;
        op_info := op_info_table[op.op];
        throughput_counter.* += op_info.throughput * op.count;
        latency_counter.* += op_info.latency * op.count;
    }

    throughput_bound_cycles := 0.0;
    latency_bound_cycles := 0.0;
    ops_table: [OPS_COUNT] int;

    for token : circuit_tokens {
        token_ops := component_info_table[token.type];
        for op : token_ops {
            add_cycles (*throughput_bound_cycles, *latency_bound_cycles, ops_table, op);
        }
    }

    for output : outputs {
        if output[0] == "V" {
            // voltage:
            //   - 1 mul
            //   - 1 add
            add_cycles (*throughput_bound_cycles, *latency_bound_cycles, ops_table, .{.MUL, 1});
            add_cycles (*throughput_bound_cycles, *latency_bound_cycles, ops_table, .{.ADD, 1});
        } else if output[0] == "I" {
            // current:
            //   - 1 load
            //   - 2 mul
            //   - 1 add
            add_cycles (*throughput_bound_cycles, *latency_bound_cycles, ops_table, .{.LOAD, 1});
            add_cycles (*throughput_bound_cycles, *latency_bound_cycles, ops_table, .{.MUL,  2});
            add_cycles (*throughput_bound_cycles, *latency_bound_cycles, ops_table, .{.ADD,  1});
        }
    }

    print("Cycles/Samples (throughput-bound): %\n", throughput_bound_cycles);
    print("Cycles/Samples (latency-bound): %\n", latency_bound_cycles);
    print_ops(ops_table);

    for arg_idx : 2..args.count-1 {
        parts := String.split(args[arg_idx], ",");
        name := parts[0];
        cycles := string_to_float(parts[1]);
        score := (latency_bound_cycles - cycles) / (latency_bound_cycles - throughput_bound_cycles);
        print("% score: %\n", name, score);
    }
}

// @TODO: re-run poly and RT-WDF numbers...
// ./perf_estimator ../rc_lowpass/rc_lowpass.wdf wdf_compiler,19.06 chowdsp_wdf,24.09 chowdsp_wdf_poly,27.51 rt-wdf,81.40
// ./perf_estimator ../preamp_eq_comb/preamp_eq_comb.wdf wdf_compiler,70.61 chowdsp_wdf,95.83 chowdsp_wdf_poly,99.69 rt-wdf,411.35
