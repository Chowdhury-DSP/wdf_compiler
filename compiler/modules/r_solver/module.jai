// r_solver -- symbolic scattering-matrix solver for WDF R-type adaptors.
//
// Given the topology of an R-type (rigid) SPQR node, this module
// builds a Modified Nodal Analysis (MNA) system symbolically and uses
// Cramer's rule (via sym_algebra.solve) to compute the WDF scattering
// matrix S.
//
// ===========================================================================
// CIRCUIT MODEL
// ===========================================================================
//
// Each WDF port k is modelled by:
//   V_k  (voltage source): from v1_k to mid_k  (mid_k = n_verts + k)
//   R_k  (resistor):       from v2_k to mid_k  (impedance = port.impedance_name)
//
// Node 0 is always the MNA datum (ground).  v1_k may be 0.
// Optional VCVS elements may be present in the internal circuit.
//
// MNA variable layout (after removing datum row/col 0):
//   cols  0 .. n_nodes-2              : node voltage unknowns v_1..v_{n_nodes-1}
//   cols  n_nodes-1 .. n_nodes+n_ports-2 : port current unknowns i_0..i_{n_ports-1}
//   cols  n_nodes+n_ports-1 .. dim-1  : VCVS branch unknowns ie_0..ie_{n_extras-1}
//
//   dim = n_nodes - 1 + n_ports + n_extras
//       = n_verts + 2*n_ports + n_extras - 1
//
// ===========================================================================
// SCATTERING MATRIX FORMULA
// ===========================================================================
//
// Weinberg's formula:  S = I + 2 * Rp * H * Xinv * V
//
//   Rp   = diag(R_0, …, R_{n-1})  port-impedance diagonal
//   V    = column selector: picks the port-current columns of Xinv
//   H    = row    selector: picks the port-current rows    of Xinv
//
// Equivalently, for each port excitation k:
//   solve  X * x_k = e_{port_current_k}
//   S[j,k] = delta(j,k)  +  2 * R_j * x_k[ port_current_j ]
//
// All Cramer solutions share det(X) as denominator, so CSE + recip_hoist
// will later hoist 1/det(X) as a single outer-loop temporary.
//
// ===========================================================================
// ADAPT PORT
// ===========================================================================
//
// If topo.adapt_port >= 0, the diagonal entry S[a][a] is set to zero by
// solving for that port's wave impedance Za.  The closed-form Za is stored
// in result.adapt_impedance, then substituted back into all S entries.
//

// ============================================================
// TOPOLOGY INPUT TYPES
// ============================================================

// One WDF port in the R-type adaptor MNA circuit.
//   V_k : from v1 to mid_k  (mid_k node index = n_verts + k)
//   R_k : from v2 to mid_k  (resistance symbol = impedance_name)
// Node 0 is the datum; v1 may be 0 (voltage source drives from ground).
R_Port :: struct {
    v1:             int;    // source-side vertex (0-indexed; 0 = datum/ground)
    v2:             int;    // load-side vertex   (0-indexed; always non-datum)
    impedance_name: string; // symbol name for the port wave impedance
}

// A voltage-controlled voltage source inside the R-type circuit.
//
// MNA stamp (indices 0-based, datum = node 0):
//   Input-side admittance  1/ri  between out_n1 and out_n2.
//   Extra-variable row:
//     -gain*(V_out1 - V_out2) + (V_ctrl1 - V_ctrl2) + ro*ie = 0
//   Extra-variable column:  +1 at ctrl_n1, -1 at ctrl_n2.
//
// out_n1 and ctrl_n2 may be the datum (0).
R_VCVS :: struct {
    out_n1:    int;    // output-side  positive node (0-indexed; may be datum)
    out_n2:    int;    // output-side  negative node (0-indexed)
    ctrl_n1:   int;    // control-side positive node (0-indexed)
    ctrl_n2:   int;    // control-side negative node (0-indexed; may be datum)
    gain_name: string; // VCVS voltage-gain symbol
    ri_name:   string; // input-side (output-side) impedance symbol
    ro_name:   string; // output-impedance symbol in the extra row
}

// Complete MNA topology for one R-type adaptor.
//   nodes  0..n_verts-1             : original circuit vertices (0 = datum)
//   nodes  n_verts..n_verts+nports-1: per-port intermediate nodes
R_Type_Topology :: struct {
    n_verts:    int;
    ports:      [..] R_Port;
    vcvs:       [..] R_VCVS;
    adapt_port: int = -1; // port to make reflection-free; -1 = none
}

// ============================================================
// RESULT TYPE
// ============================================================

R_Solver_Result :: struct {
    // n_ports × n_ports scattering matrix, row-major.
    // S_matrix[i * n_ports + j] is the Expr_ID for S_{ij}.
    // All IDs reference `exprs`.
    S_matrix:             [..] Expr_ID;
    n_ports:              int;

    // Symbolic impedance name for each port (parallel to topo.ports).
    port_impedance_names: [..] string;

    // Reflection-free port index, or -1 if no adaptation was performed.
    adapt_port:           int     = -1;
    // Closed-form expression for the adapted port's wave impedance.
    // Valid only when adapt_port >= 0.
    adapt_impedance:      Expr_ID = -1; // NO_EXPR

    // Expression pool referenced by all Expr_IDs above.
    exprs: [..] Expr;
}

// ============================================================
// SOLVER
// ============================================================

solve_rtype_scattering :: (topo: R_Type_Topology) -> R_Solver_Result {
    result: R_Solver_Result;
    exprs := *result.exprs;

    n_ports  := topo.ports.count;
    n_extras := topo.vcvs.count;

    // ---------------------------------------------------------------
    // Condensed MNA: analytically eliminate intermediate nodes (mid_k)
    // and port current unknowns (i_k) from the system.
    //
    // For each port k the voltage source gives  v_mid = v_v1 - V_k
    // and KCL at mid_k gives  i_k = G_k*(v_v1 - v_v2) - G_k*V_k.
    // Substituting into the KCL equations at v1/v2 leaves a condensed
    // system where port k is replaced by a conductance G_k between v1_k
    // and v2_k, plus a current-source injection G_k at (v2_k, v1_k) for
    // the excitation whose port is k.
    //
    // Port current recovery:
    //   i_j = G_j*(v_v1_j - v_v2_j) - G_j*delta(j, exc)
    //
    // Scattering matrix (R_j * G_j = 1 cancels):
    //   S[j, exc] = delta(j,exc) + 2*R_j*i_j
    //             = -delta(j,exc) + 2*(v_v1_j - v_v2_j)
    //
    // Condensed dim = n_verts - 1 + n_extras  (e.g. bassman: 19 -> 5).
    // ---------------------------------------------------------------
    dim_r        := topo.n_verts - 1 + n_extras;
    extra_base_r := topo.n_verts - 1;   // VCVS current unknowns start here

    result.n_ports    = n_ports;
    result.adapt_port = topo.adapt_port;
    for port : topo.ports  array_add(*result.port_impedance_names, port.impedance_name);

    one     := expr_const( 1, exprs);
    neg_one := expr_neg(one, exprs);

    // Accumulate a symbolic term into matrix X at (row, col).
    xadd :: (X: *[..]Expr_ID, row: int, col: int, dim: int,
             term: Expr_ID, exprs: *[..]Expr) {
        idx := row * dim + col;
        cur := X.*[idx];
        X.*[idx] = ifx cur == NO_EXPR then term
                                        else expr_bin(.ADD, cur, term, exprs);
    }

    // Replaces every symbol node with a matching name by `new_expr`.
    // Needed because fold/simplify can duplicate symbol nodes into new Expr_IDs,
    // so ID-based substitute(old_id, ...) may miss aliases.
    substitute_symbol_name :: (symbol_name: string, new_expr: Expr_ID, exprs: *[..] Expr) {
        for i : 0..exprs.*.count-1 {
            e := exprs.*[i];
            if e.kind == .SYMBOL && e.name == symbol_name {
                substitute(cast(Expr_ID) i, new_expr, exprs);
            }
        }
    }

    // Compute det(X_r with column col_idx replaced by b), seeding the cache
    // from cache_src with sub-minors that do not involve col_idx.
    det_cramer :: (X_r: [] Expr_ID, dim_r: int, b: [] Expr_ID, col_idx: int,
                  mask: u64, cache_src: [] Det_Cache_Entry,
                  exprs: *[..] Expr) -> Expr_ID {
        col_bit := cast(u64)1 << col_idx;
        X_col: [..] Expr_ID;
        for row : 0..dim_r-1 {
            for c : 0..dim_r-1 {
                array_add(*X_col, ifx c == col_idx then b[row] else X_r[row*dim_r+c]);
            }
        }
        cache: [..] Det_Cache_Entry;
        for e: cache_src  if (e.cols & col_bit) == 0  array_add(*cache, e);
        d := symbolic_det(X_col, dim_r, mask, mask, *cache, exprs);
        fold(d, exprs);
        return d;
    }

    // ---------------------------------------------------------------
    // 1. Build condensed MNA matrix X_r  (dim_r x dim_r).
    //
    //    Port k:  stamps G_k conductance between v1_k and v2_k.
    //             For adapt port, G_k = symbol G_{name}; else G_k = 1/R_{name}.
    //    VCVS:    same stamp as the full MNA, using original node indices.
    // ---------------------------------------------------------------
    G_k_syms: [..] Expr_ID;   // conductance expression (symbol or 1/R) per port

    X_r: [..] Expr_ID;
    array_resize(*X_r, dim_r * dim_r);
    for i : 0..X_r.count-1  X_r[i] = NO_EXPR;

    for k : 0..n_ports-1 {
        port  := topo.ports[k];
        v2c_r := port.v2 - 1;   // v2 is always non-datum
        v1c_r := port.v1 - 1;   // -1 when v1 = datum (node 0)

        G_k: Expr_ID;
        if k == topo.adapt_port {
            G_k = expr_symbol(tprint("G_%", port.impedance_name), exprs);
        } else {
            G_k = expr_div(one, expr_symbol(port.impedance_name, exprs), exprs);
        }
        array_add(*G_k_syms, G_k);
        neg_Gk := expr_neg(G_k, exprs);

        // Conductance G_k between v1 and v2.
        // If v1 = datum the port is grounded at v1, so only the v2 diagonal entry.
        xadd(*X_r, v2c_r, v2c_r, dim_r, G_k, exprs);
        if v1c_r >= 0 {
            xadd(*X_r, v1c_r, v1c_r, dim_r, G_k,    exprs);
            xadd(*X_r, v2c_r, v1c_r, dim_r, neg_Gk, exprs);
            xadd(*X_r, v1c_r, v2c_r, dim_r, neg_Gk, exprs);
        }
    }

    // VCVS elements reference original node indices, which are valid in the
    // condensed system.  Only extra_base_r and dim_r differ from the full MNA.
    for j : 0..n_extras-1 {
        vcvs  := topo.vcvs[j];
        exc_r := extra_base_r + j;

        Ri_sym := expr_symbol(vcvs.ri_name,   exprs);
        Ro_sym := expr_symbol(vcvs.ro_name,   exprs);
        Ag_sym := expr_symbol(vcvs.gain_name, exprs);
        Gi     := expr_div(one, Ri_sym, exprs);
        neg_Gi := expr_neg(Gi,     exprs);
        neg_Ag := expr_neg(Ag_sym, exprs);

        o1v := vcvs.out_n1 != 0;
        o2v := vcvs.out_n2 != 0;
        o1  := vcvs.out_n1 - 1;
        o2  := vcvs.out_n2 - 1;
        if o1v  xadd(*X_r, o1, o1, dim_r, Gi,     exprs);
        if o2v  xadd(*X_r, o2, o2, dim_r, Gi,     exprs);
        if o1v && o2v {
            xadd(*X_r, o1, o2, dim_r, neg_Gi, exprs);
            xadd(*X_r, o2, o1, dim_r, neg_Gi, exprs);
        }

        if o1v  xadd(*X_r, exc_r, o1, dim_r, neg_Ag, exprs);
        if o2v  xadd(*X_r, exc_r, o2, dim_r, Ag_sym, exprs);

        c1v := vcvs.ctrl_n1 != 0;
        c2v := vcvs.ctrl_n2 != 0;
        c1  := vcvs.ctrl_n1 - 1;
        c2  := vcvs.ctrl_n2 - 1;
        if c1v { xadd(*X_r, exc_r, c1, dim_r, one,     exprs);  xadd(*X_r, c1, exc_r, dim_r, one,     exprs); }
        if c2v { xadd(*X_r, exc_r, c2, dim_r, neg_one, exprs);  xadd(*X_r, c2, exc_r, dim_r, neg_one, exprs); }
        xadd(*X_r, exc_r, exc_r, dim_r, Ro_sym, exprs);
    }

    // ---------------------------------------------------------------
    // 2. Fold condensed entries and compute det(X_r) once.
    // ---------------------------------------------------------------
    for i : 0..dim_r-1 {
        for j : 0..dim_r-1 {
            xij := X_r[i * dim_r + j];
            if xij != NO_EXPR  fold(xij, exprs);
        }
    }

    assert(dim_r <= 62, "Condensed MNA too large for bitmask Cramer solver (dim_r > 62)");
    all_mask_r : u64 = (cast(u64)1 << dim_r) - 1;
    print("Computing det of condensed %1x%1 system\n", dim_r);
    det_Xr_cache: [..] Det_Cache_Entry;
    det_Xr := symbolic_det(X_r, dim_r, all_mask_r, all_mask_r, *det_Xr_cache, exprs);
    assert(!(exprs.*[det_Xr].kind == .CONST && exprs.*[det_Xr].value == 0),
           "Condensed MNA is singular!");
    fold(det_Xr, exprs);

    // ---------------------------------------------------------------
    // 3. For each excitation k: compute S[j,k] for all ports j.
    //
    //    RHS for excitation k:
    //      b_k[v2c_k] = -G_k,   b_k[v1c_k] = +G_k  (if v1_k != datum)
    //
    //    Cramer solution components:
    //      x[v2c_j] = det(X_r with col v2c_j <- b_k) / det(X_r)
    //      x[v1c_j] = det(X_r with col v1c_j <- b_k) / det(X_r)
    //                 (= 0 if v1_j = datum)
    //
    //    S[j,k] = -delta(j,k) + 2*(x[v1c_j] - x[v2c_j])
    //
    //    For the adapt port: save raw Cramer numerator dets so the
    //    adaptation equation can be formed without a rational denominator.
    // ---------------------------------------------------------------
    array_resize(*result.S_matrix, n_ports * n_ports);

    // Raw Cramer numerator dets for the adapt case (saved for step 4).
    adapt_det_v1: Expr_ID = NO_EXPR;   // det(X_r col v1_a <- b_a); NO_EXPR if v1_a = datum
    adapt_det_v2: Expr_ID = NO_EXPR;   // det(X_r col v2_a <- b_a)

    for k : 0..n_ports-1 {
        port_k := topo.ports[k];
        v2c_k  := port_k.v2 - 1;
        v1c_k  := port_k.v1 - 1;
        G_k    := G_k_syms[k];

        // Build RHS b_k (dim_r entries, structural zeros = NO_EXPR).
        b_k: [..] Expr_ID;
        for i : 0..dim_r-1  array_add(*b_k, NO_EXPR);
        b_k[v2c_k] = expr_neg(G_k, exprs);
        if v1c_k >= 0  b_k[v1c_k] = G_k;

        for j : 0..n_ports-1 {
            port_j := topo.ports[j];
            v2c_j  := port_j.v2 - 1;
            v1c_j  := port_j.v1 - 1;

            // Cramer numerator for v2_j.
            det_v2j := det_cramer(X_r, dim_r, b_k, v2c_j, all_mask_r, det_Xr_cache, exprs);

            // Cramer numerator for v1_j (skip if v1_j = datum or same node as v2_j).
            det_v1j: Expr_ID = NO_EXPR;
            if v1c_j >= 0 && v1c_j != v2c_j {
                det_v1j = det_cramer(X_r, dim_r, b_k, v1c_j, all_mask_r, det_Xr_cache, exprs);
            }

            // Save raw dets for the adapt step.
            if k == topo.adapt_port && j == k {
                adapt_det_v2 = det_v2j;
                adapt_det_v1 = det_v1j;  // NO_EXPR when v1_a = datum
            }

            // x[v2c_j] = det_v2j / det_Xr.
            x_v2j: Expr_ID;
            if exprs.*[det_v2j].kind == .CONST && exprs.*[det_v2j].value == 0 {
                x_v2j = det_v2j;
            } else {
                x_v2j = expr_div(det_v2j, det_Xr, exprs);
                fold(x_v2j, exprs);
            }

            // x[v1c_j] = det_v1j / det_Xr  (or NO_EXPR = 0 if v1_j = datum).
            x_v1j: Expr_ID = NO_EXPR;
            if det_v1j != NO_EXPR {
                if exprs.*[det_v1j].kind == .CONST && exprs.*[det_v1j].value == 0 {
                    x_v1j = det_v1j;
                } else {
                    x_v1j = expr_div(det_v1j, det_Xr, exprs);
                    fold(x_v1j, exprs);
                }
            }

            // v_diff = v_v1_j - v_v2_j  (= -x_v2j when v1_j = datum).
            // x_v2j is always defined (v2_j is never datum), so v_diff != NO_EXPR.
            v_diff := combine(x_v1j, x_v2j, .SUB, exprs);

            // S[j,k] = -delta(j,k) + 2 * v_diff
            two  := expr_const(2, exprs);
            term := expr_mul(two, v_diff, exprs);
            fold(term, exprs);
            neg_delta := ifx j == k then neg_one else NO_EXPR;
            s_jk := combine(neg_delta, term, .ADD, exprs);
            if s_jk != NO_EXPR  s_jk = simplify(s_jk, exprs);
            result.S_matrix[j * n_ports + k] = s_jk;
        }
    }

    // ---------------------------------------------------------------
    // 4. Adapt port: solve S[a,a] = 0  for G_a, then recover R_a = 1/G_a.
    //
    //    S[a,a] = -1 + 2*(det_v1 - det_v2)/det_Xr = 0
    //    Cleared form (linear in G_a):
    //      2*(det_v1 - det_v2) - det_Xr = 0
    //
    //    det_Xr and each Cramer numerator are individually linear in G_a,
    //    so this equation is linear and solve() handles it directly.
    // ---------------------------------------------------------------
    if topo.adapt_port >= 0 {
        a       := topo.adapt_port;
        ga_name := tprint("G_%", topo.ports[a].impedance_name);
        Ga      := expr_symbol(ga_name, exprs);
        two     := expr_const(2, exprs);

        assert(adapt_det_v2 != NO_EXPR);

        // Numerator of v_diff_aa: (det_v1 - det_v2), or -det_v2 if v1_a = datum.
        adapt_num := combine(adapt_det_v1, adapt_det_v2, .SUB, exprs);

        // Cleared adaptation equation: 2*adapt_num - det_Xr = 0.
        adapt_eq_g := simplify(
            expr_bin(.SUB, expr_mul(two, adapt_num, exprs), det_Xr, exprs), exprs);

        adapt_solutions_g := solve(.[adapt_eq_g], .[Ga], exprs);
        adapt_g := simplify(adapt_solutions_g[0], exprs);

        // Degenerate check: if G_a solved to the constant 0 the adapt port's
        // v2 node is electrically isolated inside the RIGID subgraph (all its
        // connections were contracted into series/parallel children).  In that
        // case the adaptation impedance is infinite and the port cannot be made
        // reflection-free.  Mark adapt_port = -1 so callers can skip codegen.
        if exprs.*[adapt_g].kind == .CONST && exprs.*[adapt_g].value == 0 {
            print("Warning: adapt port % ('%') of RIGID node is degenerate (isolated node). Adaptation skipped.\n",
                  a, topo.ports[a].impedance_name);
            result.adapt_port = -1;
            // Still substitute G_a = 0 so S entries for non-adapt ports are clean.
            substitute_symbol_name(ga_name, adapt_g, exprs);
            for i : 0..n_ports*n_ports-1 {
                e := result.S_matrix[i];
                if e == NO_EXPR  continue;
                fold(e, exprs);
                result.S_matrix[i] = simplify(e, exprs);
            }
            return result;
        }
        result.adapt_impedance = simplify(expr_div(one, adapt_g, exprs), exprs);

        // Substitute the solved G_a back into all S entries.
        substitute_symbol_name(ga_name, adapt_g, exprs);
        for i : 0..n_ports*n_ports-1 {
            e := result.S_matrix[i];
            if e == NO_EXPR  continue;
            fold(e, exprs);
            result.S_matrix[i] = simplify(e, exprs);
        }
    }

    return result;
}

// ===========================================================================
// CODE GENERATION  (generate_rtype_header)
// ===========================================================================
// Converts a solved R_Solver_Result into a self-contained C++ sidecar header.
// ordered_child_names must list port impedance names in the same order as
// node.children (the order wdf_compiler passes R/G pairs to update_vars).

generate_rtype_header :: (
    node_name:           string,
    result:              *R_Solver_Result,
    ordered_child_names: [] string
) -> string {

    builder:    String_Builder;
    exprs     := *result.exprs;
    n_ports   := result.n_ports;
    adapt     := result.adapt_port;
    n_children := ordered_child_names.count;

    // child_to_port[k] = port index (in result.port_impedance_names) for child k.
    child_to_port: [..] int;
    for k : 0..n_children-1 {
        name_k := ordered_child_names[k];
        found  := false;
        for j : 0..n_ports-1 {
            if result.port_impedance_names[j] == name_k {
                array_add(*child_to_port, j);
                found = true;
                break;
            }
        }
        assert(found, tprint("generate_rtype_header: child '%' not in port_impedance_names", name_k));
    }

    // Gather non-zero S entries + adapt_impedance into gen_var_ids for CSE.
    // expr_gv_idx[i] = index in gen_var_ids for S_matrix[i], or -1 (zero).
    expr_gv_idx: [..] int;
    gen_var_ids: [..] Expr_ID;

    for i : 0..n_ports*n_ports-1 {
        e := result.S_matrix[i];
        if e == NO_EXPR {
            array_add(*expr_gv_idx, -1);
        } else {
            array_add(*expr_gv_idx, gen_var_ids.count);
            array_add(*gen_var_ids, e);
        }
    }

    adapt_gv_idx := -1;
    if adapt >= 0 && result.adapt_impedance != NO_EXPR {
        adapt_gv_idx = gen_var_ids.count;
        array_add(*gen_var_ids, result.adapt_impedance);
    }

    // CSE over all S entries + adapt expression together.
    cse_vars: [..] CSE_Var;
    if gen_var_ids.count > 0 {
        cse_vars = cse(gen_var_ids, exprs);
        sort_cse_topological(*cse_vars, exprs.*);
    }

    // File preamble.
    print_to_builder(*builder, "// Auto-generated by wdf_compiler -- do not edit manually.\n");
    print_to_builder(*builder, "#pragma once\n\n");
    print_to_builder(*builder, "namespace rtype_% {\n\n", node_name);
    print_to_builder(*builder, "static constexpr int n_ports    = %;\n", n_ports);
    print_to_builder(*builder, "static constexpr int n_children = %;\n", n_children);
    if adapt >= 0 {
        print_to_builder(*builder, "static constexpr int adapt_port = %;\n", adapt);
    }
    print_to_builder(*builder, "\n");

    // Structs.
    print_to_builder(*builder, "struct R_Params {};\n\n");
    print_to_builder(*builder, "struct R_Vars {\n");
    print_to_builder(*builder, "    float S[n_ports * n_ports]{};\n");
    if adapt >= 0 {
        print_to_builder(*builder, "    float R_adapt{};\n");
    }
    print_to_builder(*builder, "};\n\n");
    if adapt >= 0 {
        print_to_builder(*builder, "struct R_State {\n");
        print_to_builder(*builder, "    float b_up{};\n");
        print_to_builder(*builder, "};\n\n");
    }

    // update_vars.
    print_to_builder(*builder, "static inline float update_vars(\n");
    print_to_builder(*builder, "    R_Vars* vars,\n");
    print_to_builder(*builder, "    const R_Params* /*params*/");
    for name : ordered_child_names {
        print_to_builder(*builder, ",\n    float %, float /*G_%*/", name, name);
    }
    print_to_builder(*builder, "\n) {\n");
    for cv : cse_vars {
        print_to_builder(*builder, "    const float % = %;\n",
            cv.name, print_expr(cv.expr_id, exprs.*));
    }
    if cse_vars.count > 0  print_to_builder(*builder, "\n");
    for i : 0..n_ports*n_ports-1 {
        row := i / n_ports;
        col := i % n_ports;
        gv  := expr_gv_idx[i];
        if gv < 0 {
            print_to_builder(*builder, "    vars->S[% * n_ports + %] = 0.0f;\n", row, col);
        } else {
            print_to_builder(*builder, "    vars->S[% * n_ports + %] = %;\n",
                row, col, print_expr(gen_var_ids[gv], exprs.*));
        }
    }
    print_to_builder(*builder, "\n");
    if adapt >= 0 && adapt_gv_idx >= 0 {
        print_to_builder(*builder, "    const float R_adapt_val = %;\n",
            print_expr(gen_var_ids[adapt_gv_idx], exprs.*));
        print_to_builder(*builder, "    vars->R_adapt = R_adapt_val;\n");
        print_to_builder(*builder, "    return R_adapt_val;\n");
    } else {
        print_to_builder(*builder, "    return 0.0f;\n");
    }
    print_to_builder(*builder, "}\n\n");

    if adapt >= 0 {
        // reflected: b = sum_k  S[adapt][port_k] * a_in[k]
        print_to_builder(*builder, "static inline float reflected(\n");
        print_to_builder(*builder, "    const R_Vars* vars,\n");
        print_to_builder(*builder, "    const R_State* /*state*/,\n");
        print_to_builder(*builder, "    const float* a_in  // [n_children]\n");
        print_to_builder(*builder, ") {\n");
        print_to_builder(*builder, "    float b = 0.0f;\n");
        for k : 0..n_children-1 {
            port_k := child_to_port[k];
            print_to_builder(*builder, "    b += vars->S[% * n_ports + %] * a_in[%];\n",
                adapt, port_k, k);
        }
        print_to_builder(*builder, "    return b;\n");
        print_to_builder(*builder, "}\n\n");

        // incident: b_out[k] = S[port_k][adapt]*a_up + sum_k2 S[port_k][port_k2]*a_in[k2]
        print_to_builder(*builder, "static inline void incident(\n");
        print_to_builder(*builder, "    const R_Vars* vars,\n");
        print_to_builder(*builder, "    R_State* /*state*/,\n");
        print_to_builder(*builder, "    float a_up,\n");
        print_to_builder(*builder, "    const float* a_in,  // [n_children]\n");
        print_to_builder(*builder, "    float* b_out        // [n_children]\n");
        print_to_builder(*builder, ") {\n");
        for k : 0..n_children-1 {
            port_k := child_to_port[k];
            print_to_builder(*builder, "    b_out[%] = vars->S[% * n_ports + %] * a_up",
                k, port_k, adapt);
            for k2 : 0..n_children-1 {
                port_k2 := child_to_port[k2];
                print_to_builder(*builder, "\n              + vars->S[% * n_ports + %] * a_in[%]",
                    port_k, port_k2, k2);
            }
            print_to_builder(*builder, ";\n");
        }
        print_to_builder(*builder, "}\n\n");
    }

    print_to_builder(*builder, "} // namespace rtype_%\n", node_name);
    return builder_to_string(*builder);
}

#scope_file
#import "Basic";
#import "sym_algebra";
