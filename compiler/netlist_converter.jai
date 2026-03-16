Decomp_Kind :: enum {
    LEAF;
    SERIES;
    PARALLEL;
    RIGID;
}

Leaf_Kind :: enum {
    NONE;
    VOLTAGE_SOURCE;
    CURRENT_SOURCE;
    RESISTOR;
    CAPACITOR;
    INDUCTOR;
    CUSTOM;
}

Node_Output_Type :: enum {
    NONE;
    VOLTAGE;
    CURRENT;
}

SPQR_Node :: struct {
    kind: Decomp_Kind;
    leaf: Leaf_Kind;

    component_id: string; // for LEAF

    parent := -1;
    children: [..] int; // for SERIES/PARALLEL/RIGID: indices into spqr_nodes

    // Only used if kind == RIGID
    rigid_vertices: int;
    rigid_edges: [..] struct { // same order as "children"
        child: int; // index into spqr_nodes
        v1: int;
        v2: int;
        virtual: bool;
    };

    value: string;
    output: Node_Output_Type;
    is_input := false;
    is_root := false;
}

Edge :: struct {
    n1: int;
    n2: int;
    spqr_node: int;   // index into SP list
    active := true;
    virtual := false; // true if edge represents a contracted subgraph
}

// Graph :: ;
Graph :: struct {
    edges: [..] Edge;
    adjacency: [..][..]int;
}

add_component :: (graph: *Graph, spqr_nodes: *[..] SPQR_Node,
                  n1: int, n2: int, component_id: string,
                  value_str: string, meta: []string) {

    get_custom_meta :: (meta: []string) -> string {
        for m_str : meta {
            if String.begins_with(m_str, "custom:") {
                return String.slice(m_str, 7, m_str.count - 7);
            }
        }
        return .{};
    }

    leaf := SPQR_Node.{
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
        case;
            leaf.leaf = .CUSTOM;
            leaf.value = get_custom_meta(meta);
    }

    for param : meta {
        if param == {
            case "root"; leaf.is_root = true;
            case "input"; leaf.is_input = true;
            case "output:V"; leaf.output = .VOLTAGE;
            case "output:I"; leaf.output = .CURRENT;
        }
    }

    leaf_id := spqr_nodes.count;
    array_add(spqr_nodes, leaf);

    edge := Edge.{
        n1 = n1,
        n2 = n2,
        spqr_node = leaf_id,
    };

    edge_id := graph.edges.count;
    array_add(*graph.edges, edge);

    if n1 >= graph.adjacency.count || n2 >= graph.adjacency.count {
            array_resize(*graph.adjacency, max(n1, n2) + 1, initialize=true);
    }
    array_add(*graph.adjacency[n1], edge_id);
    array_add(*graph.adjacency[n2], edge_id);
}

parse_netlist :: (netlist_str: string) -> Graph, [..] SPQR_Node, [..] string {
    lines := String.split(netlist_str, "\n");

    graph: Graph;
    spqr_nodes : [..] SPQR_Node;
    circuit_meta : [..] string;
    array_reserve(*spqr_nodes, lines.count * 2);

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
        line_trim := String.trim(line);
        if line_trim.count == 0 { continue; } // empty line

        if String.begins_with(line_trim, ".meta") {
            array_add(*circuit_meta, String.slice(line_trim, 6, line_trim.count - 6));
        }

        if line_trim[0] == "*" || line_trim[0] == "." { continue; } // comment or heading line

        parts := String.split(line_trim, " ");
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

        add_component(*graph, *spqr_nodes, node1, node2, parts[0], value_str, meta);
    }

    return graph, spqr_nodes, circuit_meta;
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

contract_series :: (graph: *Graph, spqr_nodes: *[..]SPQR_Node,
                    node: int, active_edges: [2] int)
{
    e1_id := active_edges[0];
    e2_id := active_edges[1];

    e1 := graph.edges[e1_id];
    e2 := graph.edges[e2_id];

    // determine outer nodes
    other1 := ifx e1.n1 == node then e1.n2 else e1.n1;
    other2 := ifx e2.n1 == node then e2.n2 else e2.n1;

    new_node := SPQR_Node.{
        kind = .SERIES,
    };
    array_add(*new_node.children, e1.spqr_node);
    array_add(*new_node.children, e2.spqr_node);

    new_spqr_id := spqr_nodes.count;
    array_add(spqr_nodes, new_node);
    spqr_nodes.*[e1.spqr_node].parent = new_spqr_id;
    spqr_nodes.*[e2.spqr_node].parent = new_spqr_id;

    // replace e1
    graph.edges[e1_id].n1 = other1;
    graph.edges[e1_id].n2 = other2;
    graph.edges[e1_id].spqr_node = new_spqr_id;
    graph.edges[e1_id].virtual = true;

    // deactivate e2
    graph.edges[e2_id].active = false;
    // array_unordered_remove_by_value(*graph.adjacency[node], e2_id); // @TODO: do I need this?
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

contract_parallel :: (graph: *Graph, spqr_nodes: *[..]SPQR_Node,
                      e1_id: int, e2_id: int)
{
    e1 := graph.edges[e1_id];
    e2 := graph.edges[e2_id];

    new_node := SPQR_Node.{
        kind = .PARALLEL,
    };
    array_add(*new_node.children, e1.spqr_node);
    array_add(*new_node.children, e2.spqr_node);

    new_spqr_id := spqr_nodes.count;
    array_add(spqr_nodes, new_node);
    spqr_nodes.*[e1.spqr_node].parent = new_spqr_id;
    spqr_nodes.*[e2.spqr_node].parent = new_spqr_id;

    // Replace e1 with new synthetic edge
    graph.edges[e1_id].spqr_node = new_spqr_id;
    graph.edges[e1_id].virtual = true;

    // deactivate e2
    graph.edges[e2_id].active = false;
}

build_rigid_component :: (graph: *Graph, spqr_nodes: *[..]SPQR_Node) -> int
{
    rigid := SPQR_Node.{
        kind = .RIGID,
    };

    rigid_id := spqr_nodes.count;
    array_add(spqr_nodes, rigid);

    vertex_map: [..] int;
    get_vertex :: (node: int, map: *[..] int) -> int {
        for v : map.* { if v == node return it_index; }
        idx := map.count;
        array_add(map, node);
        return idx;
    }

    for edge : graph.edges {
        if !edge.active continue;

        array_add(*spqr_nodes.*[rigid_id].children, edge.spqr_node);
        spqr_nodes.*[edge.spqr_node].parent = rigid_id;

        v1 := get_vertex(edge.n1, *vertex_map);
        v2 := get_vertex(edge.n2, *vertex_map);

        array_add(*spqr_nodes.*[rigid_id].rigid_edges, .{
            child = edge.spqr_node,
            v1 = v1,
            v2 = v2,
            virtual = edge.virtual,
        });
    }

    spqr_nodes.*[rigid_id].rigid_vertices = vertex_map.count;

    return rigid_id;
}

reduce_to_spqr_tree :: (graph: *Graph, spqr_nodes: *[..]SPQR_Node) -> int
{
    while true {
        e1, e2 := find_parallel_pair(graph);
        if e1 != -1 {
            contract_parallel(graph, spqr_nodes, e1, e2);
            continue;
        }

        series_node, active_edges := find_series_node(graph);
        if series_node != -1 {
            contract_series(graph, spqr_nodes, series_node, active_edges);
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
        // assert(false, "Netlist Error: Circuit graph is not series-parallel!");
        return build_rigid_component(graph, spqr_nodes);
    }

    junction_root_idx := graph.edges[remaining].spqr_node;
    junction_root := spqr_nodes.*[graph.edges[remaining].spqr_node];
    element_root_idx := junction_root.children[1];
    first_child_idx := junction_root.children[0];
    array_add(*spqr_nodes.*[element_root_idx].children, first_child_idx);
    spqr_nodes.*[element_root_idx].parent = -1;
    spqr_nodes.*[first_child_idx].parent = element_root_idx;

    assert(junction_root_idx == spqr_nodes.count - 1);
    spqr_nodes.count -= 1;

    return element_root_idx; // return "root" node index
}

reroot :: (spqr_nodes: []SPQR_Node, old_root: int) -> int
{
    new_root := -1;
    for node : spqr_nodes {
        if node.is_root { new_root = it_index; break; }
    }

    if new_root < 0 {
        spqr_nodes[old_root].is_root = true;
        return old_root;
    }

    find_path_to_root :: (spqr_nodes: []SPQR_Node, start: int) -> [..] int {
        path: [..]int;
        current := start;

        while current != -1 {
            array_add(*path, current);
            current = spqr_nodes[current].parent;
        }

        return path;  // from leaf up to root
    }

    path := find_path_to_root(spqr_nodes, new_root);
    // print("%\n", path);

    for i : 0..path.count-2 {
        // print("%\n", spqr_nodes);
        // print_spqr_graph(spqr_nodes, new_root);

        child := path[i];
        parent := path[i+1];
        if parent == -1 break;

        // Remove child from parent's children
        array_unordered_remove_by_value(*spqr_nodes[parent].children, child, stop_after_first=true);

        // Add parent as child of current
        array_add(*spqr_nodes[child].children, parent);

        // Fix parent pointers
        spqr_nodes[parent].parent = child;
        if i + 2 < path.count {
            grandparent := path[i+2];
            spqr_nodes[child].parent  = grandparent;
        }

        // Fix rigid edges
        if spqr_nodes[parent].kind == .RIGID {
            for *edge : spqr_nodes[parent].rigid_edges {
                if edge.child == child {
                    edge.child = -1;
                    break;
                }
            }
        }
    }

    return new_root;
}

get_element_name :: (node: SPQR_Node, index: s64) -> string {
    if node.kind == .SERIES { return tprint("S%", index); }
    if node.kind == .PARALLEL { return tprint("P%", index); }
    if node.kind == .RIGID { return tprint("R%", index); }
    return node.component_id;
}

generate_wdf_string :: (nodes: []SPQR_Node, root: int, meta: []string) -> string {
    builder: String_Builder;
    print_to_builder(*builder, "circuit:\n");

    print_element :: (builder: *String_Builder, nodes: []SPQR_Node, index: int, depth := 0) {
        node := nodes[index];
        type_name := "";
        elem_name := get_element_name(node, index);
        if node.kind == .SERIES {
            type_name = "Series";
        } else if node.kind == .PARALLEL {
            type_name = "Parallel";
        } else if node.kind == .RIGID {
            type_name = "Custom";
            // @TODO: can we insert the custom element args here?
            // node.value = tprint("%, prefix=rtype_, params=R_Params, vars=R_Vars, updater=update_vars, compute=root_compute", node.children.count);
            // print("%\n", node);
        } else {
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
            } else if node.leaf == .CUSTOM {
                type_name = "Custom";
            }
        }

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

    print_to_builder(*builder, "\nmeta:\n");
    for m_str : meta {
        print_to_builder(*builder, "%\n", m_str);
    }

    return builder_to_string(*builder);
}

print_spqr_graph :: (nodes: []SPQR_Node, root: int) {

    print_element :: (nodes: []SPQR_Node, index: int, prefix: string, last: bool) {
        node := nodes[index];
        name := ifx node.kind == .LEAF then node.component_id else tprint("%", node.kind);

        // print prefix
        print("%", prefix);

        // print branch
        if last {
            print("└── ");
        } else {
            print("├── ");
        }

        color := Console_Color.WHITE;
        if node.is_input color = .RED;
        else if node.is_root color = .YELLOW;
        else if node.output == .VOLTAGE color = .GREEN;
        else if node.output == .CURRENT color = .MAGENTA;

        print_color("%\n", name, color=color);

        // compute prefix for children
        child_prefix := prefix;
        if last {
            child_prefix = tprint("%    ", prefix);
        } else {
            child_prefix = tprint("%│   ", prefix);
        }

        for i: 0..node.children.count-1 {
            child := node.children[i];
            if child < 0 continue;

            child_last := i == node.children.count - 1;

            print_element(nodes, child, child_prefix, child_last);
        }
    }

    // root printed without prefix
    root_node := nodes[root];
    name := ifx root_node.kind == .LEAF then root_node.component_id else tprint("%", root_node.kind);
    color := Console_Color.WHITE;
    if root_node.is_input color = .RED;
    else if root_node.is_root color = .YELLOW;
    print_color("%\n", name, color=color);

    for i: 0..root_node.children.count-1 {
        child := root_node.children[i];
        if child < 0 continue;

        print_element(nodes, child, "", i == root_node.children.count-1);
    }
}

find_rigid_node :: (nodes: []SPQR_Node) -> s64 {
    for node : nodes {
        if node.kind == .RIGID { return it_index; }
    }
    return -1;
}

generate_rtype_netlist_string :: (nodes: []SPQR_Node, r_node_idx: s64) -> string {
    builder: String_Builder;
    print_to_builder(*builder, "* R-Type Netlist\n");
    r_node := nodes[r_node_idx];
    // print("%\n", r_node);

    adapt_node := -1;
    in_between_idx := r_node.rigid_vertices;
    for edge : r_node.rigid_edges {
        child_idx := edge.child;
        if child_idx < 0 {
            child_idx = r_node.parent;
            adapt_node = it_index;
        }
        child_name := get_element_name(nodes[child_idx], child_idx);
        print_to_builder(*builder, "V%1 %2 %3 %4\n", it_index + 1, edge.v1 + 1, in_between_idx + 1, child_name);
        print_to_builder(*builder, "R%1 %2 %3\n", it_index + 1, edge.v2 + 1, in_between_idx + 1);
        in_between_idx += 1;
    }

    print_to_builder(*builder, ".adapt = %\n", adapt_node);
    print_to_builder(*builder, ".end\n");
    return builder_to_string(*builder);
}
