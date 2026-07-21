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
    // rigid_vertex_global[local_idx] = global node_names index.
    // Lets code outside build_rigid_component (e.g. op-amp wiring) translate
    // global node indices into this rigid component's local vertex numbering.
    rigid_vertex_global: [..] int;

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

Graph :: struct {
    edges: [..] Edge;
    adjacency: [..][..]int;
    // pinned[node] == true means the node must survive series contraction
    // even if it has exactly two active edges. Used for nodes that op-amps
    // reference but that never become graph edges themselves.
    pinned: [..] bool;
}

// Selects which op-amp macromodel an "E"/"e" netlist component uses,
// set via a ".meta macromodel:<name>" comment token on that line.
Op_Amp_Model :: enum {
    FINITE_GAIN; // 3-parameter model: input/output impedance + open-loop gain (Ri/Ro/Ag)
    IDEAL;       // zero-parameter nullor model: V+ = V-, no input current
}

Op_Amp :: struct {
    component_id: string;
    n_pos: int;
    n_neg: int;
    n_out: int;
    n_gnd: int;
    model := Op_Amp_Model.FINITE_GAIN;
}

// A SPICE "D<name> n1 n2 <model>" line, resolved by resolve_diodes().
Diode :: struct {
    component_id: string;
    n1: int;
    n2: int;
    model: string;
}

get_node_idx :: (node_names: *[..]string, name: string) -> int {
    found, idx := array_find(node_names.*, name);
    if found return idx;

    idx = node_names.count;
    array_add(node_names, name);
    return idx;
}

add_component :: (graph: *Graph, spqr_nodes: *[..] SPQR_Node,
                  op_amps: *[..] Op_Amp, diodes: *[..] Diode, node_names: *[..]string,
                  n1: int, n2: int, value_str: string,
                  meta: []string, parts: []string) {

    get_custom_meta :: (meta: []string) -> string {
        for m_str : meta {
            if String.begins_with(m_str, "custom:") {
                return String.slice(m_str, 7, m_str.count - 7);
            }
        }
        return .{};
    }

    // Reads an op-amp's "macromodel:<name>" meta token. No token means the
    // existing 3-parameter finite-gain model, for backwards compatibility.
    get_macromodel_meta :: (meta: []string) -> Op_Amp_Model {
        for m_str : meta {
            if String.begins_with(m_str, "macromodel:") {
                name := String.slice(m_str, 11, m_str.count - 11);
                if name == {
                    case "finite-gain"; return .FINITE_GAIN;
                    case "ideal";       return .IDEAL;
                    case; assert(false, tprint("Unknown op-amp macromodel '%'", name));
                }
            }
        }
        return .FINITE_GAIN;
    }

    component_id := parts[0];
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
        case "E"; #through;
        case "e";
            op_amp := Op_Amp.{
                component_id = component_id,
                n_pos = get_node_idx(node_names, parts[3]),
                n_neg = get_node_idx(node_names, parts[4]),
                n_out = get_node_idx(node_names, parts[1]),
                n_gnd = get_node_idx(node_names, parts[2]),
                model = get_macromodel_meta(meta),
            };
            array_add(op_amps, op_amp);
            return;
        case "D"; #through;
        case "d";
            error_assert(parts.count >= 4, tprint("Diode % needs a model name: \"D<name> n+ n- <model>\"", component_id));
            if compile_failed return;
            diode := Diode.{
                component_id = component_id,
                n1 = n1,
                n2 = n2,
                model = parts[3],
            };
            array_add(diodes, diode);
            return;
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
            case "variable"; leaf.value = "";
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

// Maps SPICE "D" lines onto the matching built-in wave-domain diode
// element. A mismatched diode count/topology, or more than one node
// pair is a compile error.
resolve_diodes :: (graph: *Graph, spqr_nodes: *[..] SPQR_Node, node_names: [] string, diodes: [] Diode) {
    if diodes.count == 0 return;

    for existing : spqr_nodes.* {
        error_assert(!existing.is_root, tprint("Netlist has diodes as well as an explicit root ('%'); can't have both.", existing.component_id));
    }
    if compile_failed return;

    n1 := diodes[0].n1;
    n2 := diodes[0].n2;
    for d : diodes {
        same_pair := (d.n1 == n1 && d.n2 == n2) || (d.n1 == n2 && d.n2 == n1);
        error_assert(same_pair, "Diodes on more than one node pair aren't supported natively; use \"; root custom:...\" instead.");
    }
    if compile_failed return;

    leaf := SPQR_Node.{ kind = .LEAF, leaf = .CUSTOM, is_root = true };

    if diodes.count == 1 {
        leaf.component_id = diodes[0].component_id;
        leaf.value = "1,prefix=wdf_lib::diode::,jai_prefix=,params=Diode_Params,vars=Diode_Vars,updater=update_vars,compute=root_compute";
    } else if diodes.count == 2 {
        d0 := diodes[0];
        d1 := diodes[1];
        antiparallel := d0.n1 == d1.n2 && d0.n2 == d1.n1;
        error_assert(antiparallel && d0.model == d1.model,
                     tprint("Diodes % and % must be antiparallel (same two nodes, opposite polarity) with the same model to form a diode pair.", d0.component_id, d1.component_id));
        if compile_failed return;

        leaf.component_id = d0.component_id; // name the pair after the first diode, like hand-written "DP"
        leaf.value = "1,prefix=wdf_lib::diode_pair::,jai_prefix=,params=Diode_Pair_Params,vars=Diode_Pair_Vars,updater=update_vars,compute=root_compute";
    } else {
        error_assert(false, tprint("% diodes share nodes % and %; only one diode or one antiparallel pair is supported natively.", diodes.count, node_names[n1], node_names[n2]));
        return;
    }

    leaf_id := spqr_nodes.count;
    array_add(spqr_nodes, leaf);

    edge := Edge.{ n1 = n1, n2 = n2, spqr_node = leaf_id };
    edge_id := graph.edges.count;
    array_add(*graph.edges, edge);
    if n1 >= graph.adjacency.count || n2 >= graph.adjacency.count {
        array_resize(*graph.adjacency, max(n1, n2) + 1, initialize=true);
    }
    array_add(*graph.adjacency[n1], edge_id);
    array_add(*graph.adjacency[n2], edge_id);
}

parse_netlist :: (netlist_str: string) -> Graph, [..] SPQR_Node, [..] Op_Amp, [..] string {
    lines := String.split(netlist_str, "\n");

    graph: Graph;
    spqr_nodes : [..] SPQR_Node;
    op_amps : [..] Op_Amp;
    diodes : [..] Diode;
    circuit_meta : [..] string;
    array_reserve(*spqr_nodes, lines.count * 2);

    node_names: [..] string;
    array_reserve(*node_names, lines.count * 2);

    for line : lines {
        line_trim := String.trim(line);
        if line_trim.count == 0 { continue; } // empty line

        if String.begins_with(line_trim, ".meta") {
            array_add(*circuit_meta, String.slice(line_trim, 6, line_trim.count - 6));
        }

        if line_trim[0] == "*" || line_trim[0] == "." { continue; } // comment or heading line (".model" cards included - we only need the model *name* referenced by a "D" line, not its params)

        raw_parts := String.split(line_trim, " ");
        parts: [..] string;
        for part : raw_parts {
            if part.count > 0 { array_add(*parts, part); }
        }
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

        add_component(*graph, *spqr_nodes, *op_amps, *diodes, *node_names, node1, node2, value_str, meta, parts);
    }

    if op_amps.count > 0 {
        array_resize(*graph.pinned, node_names.count, initialize=true);
        for op_amp : op_amps {
            graph.pinned[op_amp.n_pos] = true;
            graph.pinned[op_amp.n_neg] = true;
            graph.pinned[op_amp.n_out] = true;
            graph.pinned[op_amp.n_gnd] = true;
        }
    }

    resolve_diodes(*graph, *spqr_nodes, node_names, diodes);

    return graph, spqr_nodes, op_amps, circuit_meta;
}

find_series_node :: (graph: *Graph) -> int, [2] int
{
    for adjacency: graph.adjacency {
        if it_index < graph.pinned.count && graph.pinned[it_index] continue;

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

    // e1 now touches `other2` instead of `node`. Keep adjacency in sync,
    // otherwise later degree checks (find_series_node/find_parallel_pair)
    // undercount other2's true degree by one -- silently missing this edge
    // can let a node that still has 3 real connections get contracted away
    // as if it only had 2, which later shows up as a node "isolated inside
    // the RIGID subgraph" for the R-type solver (see PLAN.md issue #1).
    array_unordered_remove_by_value(*graph.adjacency[node], e1_id, stop_after_first=true);
    array_add(*graph.adjacency[other2], e1_id);

    // deactivate e2
    graph.edges[e2_id].active = false;
    array_unordered_remove_by_value(*graph.adjacency[node], e2_id, stop_after_first=true);
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

    // The r_solver treats local vertex 0 as the MNA datum (circuit ground,
    // global node 0). Pre-seed global node 0 as local vertex 0 whenever it
    // appears in the rigid component; otherwise the first edge's n1 gets
    // index 0 and is treated as ground, which produces an incorrect MNA.
    for edge : graph.edges {
        if !edge.active continue;
        if edge.n1 == 0 || edge.n2 == 0 {
            array_add(*vertex_map, 0);
            break;
        }
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
    spqr_nodes.*[rigid_id].rigid_vertex_global = vertex_map;

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

// prefer_rigid_root (".meta root:rigid") roots the tree at the RIGID node
// instead of the leaf annotated "root" (caller must ensure no leaf also
// claims that label). In practice this is a no-op: reduce_to_spqr_tree
// creates at most one RIGID node, as its last step, absorbing every
// remaining edge -- so it's already old_root. The path-walking logic below
// still applies generally, in case that invariant ever changes.
reroot :: (spqr_nodes: []SPQR_Node, old_root: int, prefer_rigid_root := false) -> int
{
    new_root := -1;
    if prefer_rigid_root {
        for node : spqr_nodes {
            if node.kind == .RIGID { new_root = it_index; break; }
        }
        assert(new_root >= 0, "Netlist Error: '.meta root:rigid' was specified, but the circuit contains no RIGID (non-series-parallel) subgraph.");
    } else {
        for node : spqr_nodes {
            if node.is_root { new_root = it_index; break; }
        }
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

        // A Series of [bare non-root voltage source, capacitor] is an ideal
        // source directly in series with a cap -- the netlist gave the
        // source no resistance at all. Emit it as a genuine Capacitive_Voltage_Source
        // (CVs) leaf instead of descending into Series(RVs, C).
        //
        // A Series of [bare non-root voltage source, resistor] is a Thevenin
        // source. Emit it as a ResistiveVoltageSource (RVs) so the reflected
        // wave is +Vs (not the -Vs that the Series combinator would produce).
        if node.kind == .SERIES && node.children.count == 2 {
            child0 := nodes[node.children[0]];
            child1 := nodes[node.children[1]];
            vs_idx  := -1;
            cap_idx := -1;
            res_idx := -1;
            if child0.kind == .LEAF && child0.leaf == .VOLTAGE_SOURCE && !child0.is_root && child1.kind == .LEAF && child1.leaf == .CAPACITOR {
                vs_idx  = node.children[0];
                cap_idx = node.children[1];
            } else if child1.kind == .LEAF && child1.leaf == .VOLTAGE_SOURCE && !child1.is_root && child0.kind == .LEAF && child0.leaf == .CAPACITOR {
                vs_idx  = node.children[1];
                cap_idx = node.children[0];
            } else if child0.kind == .LEAF && child0.leaf == .VOLTAGE_SOURCE && !child0.is_root
                    && child1.kind == .LEAF && child1.leaf == .RESISTOR && child1.value.count > 0 {
                vs_idx  = node.children[0];
                res_idx = node.children[1];
            } else if child1.kind == .LEAF && child1.leaf == .VOLTAGE_SOURCE && !child1.is_root
                    && child0.kind == .LEAF && child0.leaf == .RESISTOR && child0.value.count > 0 {
                vs_idx  = node.children[1];
                res_idx = node.children[0];
            }
            if vs_idx >= 0 && cap_idx >= 0 {
                vs_node  := nodes[vs_idx];
                cap_node := nodes[cap_idx];
                for i: 0..depth-1  print_to_builder(builder, "    ");
                print_to_builder(builder, "CVs(%, %);\n", vs_node.component_id, cap_node.value);
                return;
            }
            if vs_idx >= 0 && res_idx >= 0 {
                vs_node  := nodes[vs_idx];
                res_node := nodes[res_idx];
                for i: 0..depth-1  print_to_builder(builder, "    ");
                print_to_builder(builder, "RVs(%, %);\n", vs_node.component_id, res_node.value);
                return;
            }
        }

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

// Plain-text SPQR tree dump for the -verbose flag. This used to go straight to
// the terminal via print_color() (raw ANSI escapes via write_string(), which
// bypasses context.logger entirely and so can't be captured by a host
// program) -- log() instead, with a bracketed tag replacing the color, so a
// statically-linked caller can capture this the same way as any other
// diagnostic output.
print_spqr_graph :: (nodes: []SPQR_Node, root: int) {

    node_tag :: (node: SPQR_Node) -> string {
        if node.is_input  return " [input]";
        if node.is_root    return " [root]";
        if node.output == .VOLTAGE  return " [V]";
        if node.output == .CURRENT  return " [I]";
        return "";
    }

    log_element :: (nodes: []SPQR_Node, index: int, prefix: string, last: bool) {
        node := nodes[index];
        name := ifx node.kind == .LEAF then node.component_id else tprint("%", node.kind);
        branch := ifx last then "└── " else "├── ";
        log("%1%2%3%4\n", prefix, branch, name, node_tag(node));

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

            log_element(nodes, child, child_prefix, child_last);
        }
    }

    // root printed without prefix
    root_node := nodes[root];
    name := ifx root_node.kind == .LEAF then root_node.component_id else tprint("%", root_node.kind);
    log("%1%2\n", name, node_tag(root_node));

    for i: 0..root_node.children.count-1 {
        child := root_node.children[i];
        if child < 0 continue;

        log_element(nodes, child, "", i == root_node.children.count-1);
    }
}

find_rigid_node :: (nodes: []SPQR_Node) -> s64 {
    for node : nodes {
        if node.kind == .RIGID { return it_index; }
    }
    return -1;
}

// Translates a global node_names index into this rigid component's local
// vertex numbering (as assigned by build_rigid_component). Op-amp nodes are
// recorded in global node space since op-amps never become graph edges and
// so never go through the SPQR reduction that assigns local indices.
global_to_rigid_vertex :: (r_node: SPQR_Node, global_idx: int) -> int {
    for v : r_node.rigid_vertex_global {
        if v == global_idx return it_index;
    }
    return -1;
}

// Converts the SPQR rigid node at r_node_idx into an R_Type_Topology.
// The adapt port is the edge whose child == -1 (the connection to the
// SPQR parent). VCVS entries are left empty; they must be filled in
// separately when the circuit contains controlled sources.
rtype_topology_from_spqr :: (nodes: []SPQR_Node, op_amps: []Op_Amp, r_node_idx: s64) -> R_Type_Topology {
    r_node := nodes[r_node_idx];
    topo: R_Type_Topology;
    topo.n_verts = r_node.rigid_vertices;

    for edge : r_node.rigid_edges {
        child_idx := edge.child;
        if child_idx < 0 {
            child_idx       = r_node.parent;
            topo.adapt_port = it_index;
        }
        name := get_element_name(nodes[child_idx], child_idx);
        array_add(*topo.ports, .{ v1 = edge.v1, v2 = edge.v2, impedance_name = name });
    }

    for op_amp : op_amps {
        n_pos := global_to_rigid_vertex(r_node, op_amp.n_pos);
        n_neg := global_to_rigid_vertex(r_node, op_amp.n_neg);
        n_out := global_to_rigid_vertex(r_node, op_amp.n_out);
        n_gnd := global_to_rigid_vertex(r_node, op_amp.n_gnd);
        assert(n_pos >= 0 && n_neg >= 0 && n_out >= 0 && n_gnd >= 0,
               tprint("Op-amp % references a node outside the rigid component", op_amp.component_id));

        if op_amp.model == {
            case .FINITE_GAIN;
                // r_solver's R_VCVS names are swapped from what they sound
                // like: its "out_n1/out_n2" pair takes the admittance
                // (1/Ri) stamp and the constraint row's Ag coefficient
                // lands on "ctrl_n1/ctrl_n2" -- i.e. "ctrl" is the terminal
                // pair that actually carries the branch current and
                // follows Ag*V_ctrl, which is the op-amp's real output. So
                // the real output pins go into ctrl_n1/ctrl_n2 and the real
                // (high-impedance) input pins go into out_n1/out_n2. This
                // also matches R_VCVS's own doc comment, which only allows
                // ctrl_n2 (not out_n2) to be grounded -- exactly where
                // n_gnd (the op-amp's output return) lands here.
                array_add(*topo.vcvs,
                          .{ out_n1 = n_pos,
                             out_n2 = n_neg,
                             ctrl_n1 = n_out,
                             ctrl_n2 = n_gnd,
                             gain_name = "Ag",
                             ri_name = "Ri",
                             ro_name = "Ro",
                           });
            case .IDEAL;
                array_add(*topo.nullors,
                          .{ in_n1 = n_pos,
                             in_n2 = n_neg,
                             out_n1 = n_out,
                             out_n2 = n_gnd,
                           });
        }
    }

    return topo;
}

generate_rtype_netlist_string :: (nodes: []SPQR_Node, op_amps: []Op_Amp, r_node_idx: s64) -> string {
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

    for op_amp : op_amps {
        n_pos := global_to_rigid_vertex(r_node, op_amp.n_pos);
        n_neg := global_to_rigid_vertex(r_node, op_amp.n_neg);
        n_out := global_to_rigid_vertex(r_node, op_amp.n_out);
        n_gnd := global_to_rigid_vertex(r_node, op_amp.n_gnd);
        assert(n_pos >= 0 && n_neg >= 0 && n_out >= 0 && n_gnd >= 0,
               tprint("Op-amp % references a node outside the rigid component", op_amp.component_id));

        if op_amp.model == {
            case .FINITE_GAIN;
                print_to_builder(*builder,
                                 "%1 %2 %3 %4 %5 Ag\n",
                                 op_amp.component_id,
                                 n_neg + 1,
                                 n_pos + 1,
                                 n_out + 1,
                                 n_gnd + 1);
            case .IDEAL;
                print_to_builder(*builder,
                                 "%1 %2 %3 %4 %5 ; macromodel:ideal\n",
                                 op_amp.component_id,
                                 n_neg + 1,
                                 n_pos + 1,
                                 n_out + 1,
                                 n_gnd + 1);
        }
    }

    print_to_builder(*builder, ".adapt = %\n", adapt_node);
    print_to_builder(*builder, ".end\n");
    return builder_to_string(*builder);
}

#scope_file
#import "Basic";
String :: #import "String";
#import "r_solver";
#import "utils";
