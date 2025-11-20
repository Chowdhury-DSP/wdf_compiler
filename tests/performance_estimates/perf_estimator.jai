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
    exp_cycles: float;
}

PIPELINE :: 0.75; // 1 = fully pipelined, 0 = no pipelining

Op_Info_Table :: [OPS_COUNT] Op_Info;
generate_op_info_table_m1 :: () -> Op_Info_Table {
    // reference: https://dougallj.github.io/applecpu/firestorm.html

    table: Op_Info_Table;

    using Op;
    table[LOAD]  = .{latency=2.5, throughput=0.333};
    table[STORE] = .{latency=0.5, throughput=1.0};
    table[ADD]   = .{latency=3.0, throughput=0.25};
    table[MUL]   = .{latency=4.0, throughput=0.25};

    for *op : table {
        op.exp_cycles = op.throughput * PIPELINE + op.latency * (1.0 - PIPELINE);
    }

    return table;
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
    op_info_table := generate_op_info_table_m1();

    cycles := 0.0;
    ops_table: [OPS_COUNT] int;

    for token : circuit_tokens {
        token_ops := component_info_table[token.type];
        for op : token_ops {
            ops_table[op.op] += op.count;
            op_info := op_info_table[op.op];
            cycles += op_info.exp_cycles * op.count;
        }
    }

    for output : outputs {
        if output[0] == "V" {
            // voltage:
            //   - 1 mul
            //   - 1 add
            ops_table[Op.ADD] += 1;
            cycles += op_info_table[Op.ADD].exp_cycles * 1;
            ops_table[Op.MUL] += 1;
            cycles += op_info_table[Op.MUL].exp_cycles * 1;
        } else if output[0] == "I" {
            // current:
            //   - 1 load
            //   - 2 mul
            //   - 1 add
            ops_table[Op.LOAD] += 1;
            cycles += op_info_table[Op.LOAD].exp_cycles * 1;
            ops_table[Op.ADD] += 1;
            cycles += op_info_table[Op.ADD].exp_cycles * 1;
            ops_table[Op.MUL] += 2;
            cycles += op_info_table[Op.MUL].exp_cycles * 2;
        }
    }

    print("Cycles/Samples: %\n", cycles);
    for op_count, it : ops_table {
        if op_count > 0 {
            print("%: %\n", cast(Op) it, op_count);
        }
    }
}

// jai perf_estimator.jai && ./perf_estimator ../preamp_eq_comb/preamp_eq_comb.wdf
