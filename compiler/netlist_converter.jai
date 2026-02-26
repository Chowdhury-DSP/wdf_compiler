Decomp_Kind :: enum {
    LEAF;
    SERIES;
    PARALLEL;
}

Leaf_Kind :: enum {
    NONE;
    VOLTAGE_SOURCE;
    CURRENT_SOURCE;
    RESISTOR;
    CAPACITOR;
    INDUCTOR;
}

Node_Output_Type :: enum {
    NONE;
    VOLTAGE;
    CURRENT;
}

SP_Node :: struct {
    kind: Decomp_Kind;
    leaf: Leaf_Kind;

    component_id: string; // for LEAF
    parent := -1;
    children: [2]int = .[-1, -1];   // for SERIES/PARALLEL: indices into sp_nodes
    value: string;
    output: Node_Output_Type;
    is_input := false;
    is_root := false;
}

Edge :: struct {
    n1: int;
    n2: int;
    sp_node: int;   // index into SP list
    active := true;
}

// Graph :: ;
Graph :: struct {
    edges: [..] Edge;
    adjacency: [..][..]int;
}

add_component :: (graph: *Graph, sp_nodes: *[..] SP_Node,
                  n1: int, n2: int, component_id: string,
                  value_str: string, meta: []string) {
    leaf := SP_Node.{
        kind = .LEAF,
        component_id = component_id,
        value = value_str,
    };

    if component_id[0] == {
        case "V"; #through;
        case "v"; leaf.leaf = .VOLTAGE_SOURCE;
        case "I"; #through;
        case "i"; leaf.leaf = .CURRENT_SOURCE;
        case "R"; #through;
        case "r"; leaf.leaf = .RESISTOR;
        case "C"; #through;
        case "c"; leaf.leaf = .CAPACITOR;
        case "L"; #through;
        case "l"; leaf.leaf = .INDUCTOR;
        case; assert(false, tprint("Netlist Error: Unknown component type: %", component_id[0]));
    }

    for param : meta {
        if param == {
            case "root"; leaf.is_root = true;
            case "input"; leaf.is_input = true;
            case "output:V"; leaf.output = .VOLTAGE;
            case "output:I"; leaf.output = .CURRENT;
        }
    }

    leaf_id := sp_nodes.count;
    array_add(sp_nodes, leaf);

    edge := Edge.{
        n1 = n1,
        n2 = n2,
        sp_node = leaf_id,
    };

    edge_id := graph.edges.count;
    array_add(*graph.edges, edge);

    if n1 >= graph.adjacency.count || n2 >= graph.adjacency.count {
        array_resize(*graph.adjacency, max(n1, n2) + 1, initialize=false);
    }
    array_add(*graph.adjacency[n1], edge_id);
    array_add(*graph.adjacency[n2], edge_id);
}

parse_netlist :: (netlist_str: string) -> Graph, [..] SP_Node {
    lines := String.split(netlist_str, "\n");

    graph: Graph;
    sp_nodes : [..] SP_Node;
    array_reserve(*sp_nodes, lines.count * 2);

    node_names: [..] string;
    array_reserve(*node_names, lines.count * 2);

    get_node_idx :: (node_names: *[..]string, name: string) -> int {
        found, idx := array_find(node_names.*, name);
        if found return idx;

        idx = node_names.count;
        array_add(node_names, name);
        return idx;
    }

    for line : lines {
        if line.count == 0 { continue; } // empty line
        if line[0] == "*" || line[0] == "." { continue; } // comment or heading line

        parts := String.split(line, " ");
        assert(parts.count >= 3,
               tprint("Netlist entry needs at least three parts: name, node 1, node 2. Parts: %", parts));
        node2 := get_node_idx(*node_names, parts[2]);
        node1 := get_node_idx(*node_names, parts[1]);
        value_str := "";
        if parts.count > 3 && parts[3].count > 0 && is_digit(parts[3][0]) { value_str = parts[3]; }

        meta: []string;
        for part : parts {
            if part == ";" {
                meta.data = parts.data + it_index + 1;
                meta.count = parts.count - it_index - 1;
                break;
            }
        }

        add_component(*graph, *sp_nodes, node1, node2, parts[0], value_str, meta);
    }

    return graph, sp_nodes;
}

find_series_node :: (graph: *Graph) -> int, [2] int
{
    for adjacency: graph.adjacency {
        active_edges: [2]int;
        active_edges_count := 0;
        for e_id: adjacency {
            if graph.edges[e_id].active {
                if active_edges_count == 2 { continue adjacency; }
                active_edges[active_edges_count] = e_id;
                active_edges_count += 1;
            }
        }
        if active_edges_count == 2 { return it_index, active_edges; }
    }

    return -1, .[-1, -1];
}

contract_series :: (graph: *Graph, sp_nodes: *[..]SP_Node,
                    node: int, active_edges: [2] int)
{
    e1_id := active_edges[0];
    e2_id := active_edges[1];

    e1 := graph.edges[e1_id];
    e2 := graph.edges[e2_id];

    // determine outer nodes
    other1 := ifx e1.n1 == node then e1.n2 else e1.n1;
    other2 := ifx e2.n1 == node then e2.n2 else e2.n1;

    new_node := SP_Node.{
        kind = .SERIES,
        children = .[e1.sp_node, e2.sp_node],
    };

    new_sp_id := sp_nodes.count;
    array_add(sp_nodes, new_node);
    sp_nodes.*[e1.sp_node].parent = new_sp_id;
    sp_nodes.*[e2.sp_node].parent = new_sp_id;

    // replace e1
    graph.edges[e1_id].n1 = other1;
    graph.edges[e1_id].n2 = other2;
    graph.edges[e1_id].sp_node = new_sp_id;

    // deactivate e2
    graph.edges[e2_id].active = false;
}

find_parallel_pair :: (graph: *Graph) -> int, int
{
    for i: 0..graph.edges.count-1 {
        e1 := graph.edges[i];
        if !e1.active continue;

        for j: i+1..graph.edges.count-1 {
            e2 := graph.edges[j];
            if !e2.active continue;

            if ((e1.n1 == e2.n1 && e1.n2 == e2.n2) ||
                (e1.n1 == e2.n2 && e1.n2 == e2.n1))
            {
                return i, j;
            }
        }
    }

    return -1, -1;
}

contract_parallel :: (graph: *Graph, sp_nodes: *[..]SP_Node,
                      e1_id: int, e2_id: int)
{
    e1 := graph.edges[e1_id];
    e2 := graph.edges[e2_id];

    new_node := SP_Node.{
        kind = .PARALLEL,
        children = .[e1.sp_node, e2.sp_node],
    };

    new_sp_id := sp_nodes.count;
    array_add(sp_nodes, new_node);
    sp_nodes.*[e1.sp_node].parent = new_sp_id;
    sp_nodes.*[e2.sp_node].parent = new_sp_id;

    // Replace e1 with new synthetic edge
    graph.edges[e1_id].sp_node = new_sp_id;

    // deactivate e2
    graph.edges[e2_id].active = false;
}

reduce_to_sp_tree :: (graph: *Graph, sp_nodes: *[..]SP_Node) -> int
{
    while true {
        e1, e2 := find_parallel_pair(graph);
        if e1 != -1 {
            contract_parallel(graph, sp_nodes, e1, e2);
            continue;
        }

        series_node, active_edges := find_series_node(graph);
        if series_node != -1 {
            contract_series(graph, sp_nodes, series_node, active_edges);
            continue;
        }

        break;
    }

    // Count remaining active edges
    remaining := -1;
    count := 0;

    for edge : graph.edges {
        if edge.active {
            remaining = it_index;
            count += 1;
        }
    }

    if count != 1 {
        assert(false, "Netlist Error: Circuit graph is not series-parallel!");
    }

    junction_root_idx := graph.edges[remaining].sp_node;
    junction_root := sp_nodes.*[graph.edges[remaining].sp_node];
    element_root_idx := junction_root.children[1];
    first_child_idx := junction_root.children[0];
    sp_nodes.*[element_root_idx].children[0] = first_child_idx;
    sp_nodes.*[element_root_idx].parent = -1;
    sp_nodes.*[first_child_idx].parent = element_root_idx;

    // @TODO: removing the junction node like this seems janky!
    assert(junction_root_idx == sp_nodes.count - 1);
    sp_nodes.count -= 1;

    return element_root_idx; // return "root" node index
}

reroot :: (sp_nodes: []SP_Node) -> int
{
    new_root := -1;
    for node : sp_nodes {
        if node.is_root { new_root = it_index; break; }
    }
    // print("New root: %\n", new_root);
    // print("New root: %\n", sp_nodes[new_root]);

    find_path_to_root :: (sp_nodes: []SP_Node, start: int) -> [..]int {
        path: [..]int;
        current := start;

        while current != -1 {
            array_add(*path, current);
            current = sp_nodes[current].parent;
        }

        return path;  // from leaf up to root
    }

    path := find_path_to_root(sp_nodes, new_root);
    // print("%\n", path);

    for i : 0..path.count-2 {
        // print("%\n", sp_nodes);
        // print_sp_graph(sp_nodes, new_root);

        child := path[i];
        parent := path[i+1];
        if parent == -1 break;

        // Remove child from parent's children
        for *child_idx : sp_nodes[parent].children {
            val := child_idx.*;
            // print("%\n", val);
            // child_idx.* = sp_nodes[new_root].children[it_index];
            if val == child { child_idx.* = -1; }
        }

        // Add parent as child of current
        for *child_idx : sp_nodes[child].children {
            val := child_idx.*;
            if val == -1 {
                child_idx.* = parent;
                break;
            }
        }

        // Fix parent pointers
        sp_nodes[parent].parent = child;
        if i + 2 < path.count {
            grandparent := path[i+2];
            sp_nodes[child].parent  = grandparent;
        }
    }

    return new_root;
}

generate_wdf_string :: (nodes: []SP_Node, root: int) -> string {
    builder: String_Builder;
    print_to_builder(*builder, "circuit:\n");

    print_element :: (builder: *String_Builder, nodes: []SP_Node, index: int, depth := 0) {
        node := nodes[index];
        type_name := "";
        elem_name := "";
        if node.kind == .SERIES {
            type_name = "Series";
            elem_name = tprint("S%", index);
        } else if node.kind == .PARALLEL {
            type_name = "Parallel";
            elem_name = tprint("P%", index);
        } else {
            elem_name = node.component_id;
            if node.leaf == .RESISTOR {
                type_name = "R";
            } else if node.leaf == .CAPACITOR {
                type_name = "C";
            } else if node.leaf == .INDUCTOR {
                type_name = "L";
            } else if node.leaf == .VOLTAGE_SOURCE {
                if node.is_root type_name = "IVs";
                else type_name = "RVs";
            } else if node.leaf == .CURRENT_SOURCE {
                if node.is_root type_name = "IIs";
                else type_name = "RIs";
            }
        }

        name := ifx node.kind == .LEAF then node.component_id else tprint("%", node.kind);

        for i: 0..depth-1 {
            print_to_builder(builder, "    ");
        }
        print_to_builder(builder, "%(%", type_name, elem_name);
        if node.value.count > 0 { print_to_builder(builder, ", %", node.value); }
        print_to_builder(builder, ");\n");

        for child_idx : node.children {
            if child_idx >= 0 { print_element(builder, nodes, child_idx, depth + 1); }
        }
    }
    print_element(*builder, nodes, root);

    print_to_builder(*builder, "\ninputs:\n");
    for node : nodes {
        if node.is_input print_to_builder(*builder, "%\n", node.component_id);
    }

    print_to_builder(*builder, "\noutputs:\n");
    for node : nodes {
        if node.output == .VOLTAGE print_to_builder(*builder, "V:%\n", node.component_id);
        if node.output == .CURRENT print_to_builder(*builder, "I:%\n", node.component_id);
    }

    return builder_to_string(*builder);
}
