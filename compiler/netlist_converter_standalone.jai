#import "Basic";
File :: #import "File";
String :: #import "String";
#import "Print_Color";
#load "netlist_converter.jai";

main :: () {
    args := get_command_line_arguments();
    netlist_file := args[1];
    wdf_file := ifx args.count >= 3 then args[2] else "";
    print("Converting netlist file % to %\n\n", netlist_file, wdf_file);

    netlist_file_str, fread_success := File.read_entire_file(netlist_file);
    assert(fread_success, "Unable to open file!");

    graph, sp_nodes, circuit_meta := parse_netlist(netlist_file_str);
    // print("%\n", graph);
    // print("%\n", sp_nodes);

    root := reduce_to_sp_tree(*graph, *sp_nodes);
    // print("%\n", graph);
    // print("%\n", sp_nodes);
    // print_sp_graph(sp_nodes, root);

    root = reroot(sp_nodes);
    // print("%\n", sp_nodes);
    print_sp_graph(sp_nodes, root);

    wdf_str := generate_wdf_string(sp_nodes, root, circuit_meta);
    // print(wdf_str);

    if wdf_file.count > 0 {
        out_file, success := File.file_open(wdf_file, true);
        assert(success, "Unable to open output file!");
        File.file_write(*out_file, wdf_str);
    }
}
