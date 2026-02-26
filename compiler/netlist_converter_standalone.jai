#import "Basic";
File :: #import "File";
String :: #import "String";
#import "Print_Color";
#load "netlist_converter.jai";

print_sp_graph :: (nodes: []SP_Node, root: int) {
    print_element :: (nodes: []SP_Node, index: int, depth := 0, last := false) {
        node := nodes[index];
        name := ifx node.kind == .LEAF then node.component_id else tprint("%", node.kind);

        for i: 0..depth-1 {
            if i == depth - 1 {
                if last { print("└── "); }
                else { print("├── "); }
            } else {
                if i == 0 { print("    "); }
                else { print("│   "); }
            }
        }

        color := Console_Color.WHITE;
        if node.is_input color = .RED;
        else if node.is_root color = .YELLOW;
        else if node.output == .VOLTAGE color = .GREEN;
        else if node.output == .CURRENT color = .MAGENTA;
        print_color("%\n", name, color = color);

        for child_idx : node.children {
            if child_idx >= 0 {
                print_element(nodes, child_idx, depth + 1,
                              node.kind == .LEAF || it_index == node.children.count-1);
            }
        }
    }
    print_element(nodes, root);
}

main :: () {
    args := get_command_line_arguments();
    netlist_file := args[1];
    wdf_file := args[2];
    print("Converting netlist file % to %\n\n", netlist_file, wdf_file);

    netlist_file_str, fread_success := File.read_entire_file(netlist_file);
    assert(fread_success, "Unable to open file!");

    graph, sp_nodes := parse_netlist(netlist_file_str);
    // print("%\n", graph);
    // print("%\n", sp_nodes);

    root := reduce_to_sp_tree(*graph, *sp_nodes);
    // print("%\n", graph);
    // print("%\n", sp_nodes);
    // print_sp_graph(sp_nodes, root);

    root = reroot(sp_nodes);
    // print("%\n", sp_nodes);
    print_sp_graph(sp_nodes, root);

    wdf_str := generate_wdf_string(sp_nodes, root);
    // print(wdf_str);

    out_file, success := File.file_open(wdf_file, true);
    assert(success, "Unable to open output file!");
    File.file_write(*out_file, wdf_str);
}
