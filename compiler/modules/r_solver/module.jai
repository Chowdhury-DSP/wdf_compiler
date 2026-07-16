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
// Node 0 is the datum; either v1 or v2 may be 0 (grounded on that side).
R_Port :: struct {
    v1:             int;    // source-side vertex (0-indexed; 0 = datum/ground)
    v2:             int;    // load-side vertex   (0-indexed; 0 = datum/ground)
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

// An ideal op-amp (a nullor), the zero-parameter macromodel: infinite
// input impedance, zero output impedance, infinite open-loop gain.
// Modelled directly as a nullor rather than as a limiting case of R_VCVS,
// to avoid the numerical ill-conditioning that near-infinite/near-zero
// symbol values would otherwise introduce.
//
// MNA stamp (indices 0-based, datum = node 0):
//   Extra-variable row:    V(in_n1) - V(in_n2) = 0        (nullator: no
//                          constraint on in_n1/in_n2 current, so neither
//                          appears in any KCL row)
//   Extra-variable column: +1 at out_n1, -1 at out_n2      (norator: the
//                          extra variable is a free current, injected at
//                          the output pins, that makes the row true)
//
// in_n1/in_n2 may be the datum (0); out_n1/out_n2 may be the datum (0).
R_Nullor :: struct {
    in_n1:  int; // input positive node (0-indexed; may be datum)
    in_n2:  int; // input negative node (0-indexed; may be datum)
    out_n1: int; // output positive node (0-indexed; may be datum)
    out_n2: int; // output negative node (0-indexed; may be datum)
}

// Complete MNA topology for one R-type adaptor.
//   nodes  0..n_verts-1             : original circuit vertices (0 = datum)
//   nodes  n_verts..n_verts+nports-1: per-port intermediate nodes
R_Type_Topology :: struct {
    n_verts:    int;
    ports:      [..] R_Port;
    vcvs:       [..] R_VCVS;
    nullors:    [..] R_Nullor;
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
    n_extras := topo.vcvs.count + topo.nullors.count;

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
        v2c_r := port.v2 - 1;   // -1 when v2 = datum (node 0)
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
        // Either side may be the datum (grounded); skip terms that touch it.
        if v2c_r >= 0  xadd(*X_r, v2c_r, v2c_r, dim_r, G_k, exprs);
        if v1c_r >= 0 {
            xadd(*X_r, v1c_r, v1c_r, dim_r, G_k, exprs);
            if v2c_r >= 0 {
                xadd(*X_r, v2c_r, v1c_r, dim_r, neg_Gk, exprs);
                xadd(*X_r, v1c_r, v2c_r, dim_r, neg_Gk, exprs);
            }
        }
    }

    // VCVS elements reference original node indices, which are valid in the
    // condensed system.  Only extra_base_r and dim_r differ from the full MNA.
    for j : 0..topo.vcvs.count-1 {
        vcvs  := topo.vcvs[j];
        exc_r := extra_base_r + j;

        print("Adding VCVS % % %\n", vcvs.ri_name, vcvs.ro_name, vcvs.gain_name);
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

    // Nullors (ideal op-amps) reference original node indices too, same as
    // VCVS above. No admittance/gain/output-impedance symbols are involved:
    // the extra-variable row only constrains in_n1/in_n2 (nullator), and the
    // extra-variable column only injects current at out_n1/out_n2 (norator)
    // -- unlike R_VCVS's ctrl_n1/ctrl_n2, which do double duty in both the
    // row and the column, in_n1/in_n2 and out_n1/out_n2 are fully decoupled.
    for j : 0..topo.nullors.count-1 {
        nullor := topo.nullors[j];
        exc_r  := extra_base_r + topo.vcvs.count + j;

        i1v := nullor.in_n1 != 0;
        i2v := nullor.in_n2 != 0;
        i1  := nullor.in_n1 - 1;
        i2  := nullor.in_n2 - 1;
        if i1v  xadd(*X_r, exc_r, i1, dim_r, one,     exprs);
        if i2v  xadd(*X_r, exc_r, i2, dim_r, neg_one, exprs);

        o1v := nullor.out_n1 != 0;
        o2v := nullor.out_n2 != 0;
        o1  := nullor.out_n1 - 1;
        o2  := nullor.out_n2 - 1;
        if o1v  xadd(*X_r, o1, exc_r, dim_r, one,     exprs);
        if o2v  xadd(*X_r, o2, exc_r, dim_r, neg_one, exprs);
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
        if v2c_k >= 0  b_k[v2c_k] = expr_neg(G_k, exprs);
        if v1c_k >= 0  b_k[v1c_k] = G_k;

        for j : 0..n_ports-1 {
            port_j := topo.ports[j];
            v2c_j  := port_j.v2 - 1;
            v1c_j  := port_j.v1 - 1;

            // Cramer numerator for v2_j (skip if v2_j = datum).
            det_v2j: Expr_ID = NO_EXPR;
            if v2c_j >= 0 {
                det_v2j = det_cramer(X_r, dim_r, b_k, v2c_j, all_mask_r, det_Xr_cache, exprs);
            }

            // Cramer numerator for v1_j (skip if v1_j = datum or same node as v2_j).
            det_v1j: Expr_ID = NO_EXPR;
            if v1c_j >= 0 && v1c_j != v2c_j {
                det_v1j = det_cramer(X_r, dim_r, b_k, v1c_j, all_mask_r, det_Xr_cache, exprs);
            }

            // Save raw dets for the adapt step.
            if k == topo.adapt_port && j == k {
                adapt_det_v2 = det_v2j;  // NO_EXPR when v2_a = datum
                adapt_det_v1 = det_v1j;  // NO_EXPR when v1_a = datum
            }

            // x[v2c_j] = det_v2j / det_Xr  (or NO_EXPR = 0 if v2_j = datum).
            x_v2j: Expr_ID = NO_EXPR;
            if det_v2j != NO_EXPR {
                if exprs.*[det_v2j].kind == .CONST && exprs.*[det_v2j].value == 0 {
                    x_v2j = det_v2j;
                } else {
                    x_v2j = expr_div(det_v2j, det_Xr, exprs);
                    fold(x_v2j, exprs);
                }
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

            // v_diff = v_v1_j - v_v2_j  (treats a datum-side term as 0).
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

        // v1_a and v2_a are distinct nodes, so at most one of them is the datum.
        assert(adapt_det_v1 != NO_EXPR || adapt_det_v2 != NO_EXPR);

        // Numerator of v_diff_aa: (det_v1 - det_v2), or -det_v2 if v1_a = datum,
        // or det_v1 if v2_a = datum.
        adapt_num := combine(adapt_det_v1, adapt_det_v2, .SUB, exprs);

        // Cleared adaptation equation: 2*adapt_num - det_Xr = 0.
        //
        // rational_simplify (not plain simplify) matters here: det_Xr and
        // adapt_num are each built from many cofactor-expansion sub-terms
        // that only cancel once fully expanded and collected. Left as
        // pattern-rewritten-but-uncollected, the generated code computes a
        // small true coefficient as the difference of much larger
        // intermediate terms -- catastrophic cancellation that can round to
        // an outright spurious 0.0 in float32 (seen in practice as a
        // divide-by-zero downstream, since this coefficient is the shared
        // denominator for G_a and everything substituted from it).
        adapt_eq_g := rational_simplify(
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
        // print("Subsistuting %\n", ga_name);
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
// Converts a solved R_Solver_Result into a self-contained sidecar header
// (C++, Jai, or C -- see `lang`).
// ordered_child_names must list port impedance names in the same order as
// node.children (the order wdf_compiler passes R/G pairs to update_vars).

generate_rtype_header :: (
    node_name:           string,
    result:              *R_Solver_Result,
    ordered_child_names: [] string,
    lang:                string
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

    // local_port[i] = physical port index for helper-matmul local slot i.
    // Local slots 0..n_children-1 are the children (a_in order, zero offset
    // -- matches child_to_port exactly); local slot n_children (only present
    // when adapted) is the up/adapt port. Root nodes have no up-port slot,
    // so n_local == n_children == n_ports there.
    local_port: [..] int;
    for k : 0..n_children-1  array_add(*local_port, child_to_port[k]);
    if adapt >= 0  array_add(*local_port, adapt);

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

    // Find symbols referenced by the generated expressions that are *not*
    // one of the WDF child ports (e.g. a VCVS's Ri/Ro/Ag). These have no
    // other source of a value, so they need to become user-configurable
    // R_Params fields. Collected before CSE, since cse() may rewrite
    // gen_var_ids in place to reference extracted temporaries.
    collect_symbols :: (id: Expr_ID, exprs: []Expr, out: *[..]string) {
        if id == NO_EXPR return;
        node := exprs[id];
        if node.kind == .SYMBOL {
            found, _ := array_find(out.*, node.name);
            if !found  array_add(out, node.name);
            return;
        }
        if node.kind == .NEG { collect_symbols(node.left, exprs, out); return; }
        if node.kind == .CONST || node.kind == .NULL return;
        collect_symbols(node.left,  exprs, out);
        collect_symbols(node.right, exprs, out);
    }
    extra_symbols: [..] string;
    for id : gen_var_ids  collect_symbols(id, exprs.*, *extra_symbols);
    for name : ordered_child_names {
        array_unordered_remove_by_value(*extra_symbols, name, stop_after_first=true);
    }

    // CSE over all S entries + adapt expression together.
    cse_vars: [..] CSE_Var;
    if gen_var_ids.count > 0 {
        cse_vars = cse(gen_var_ids, exprs);
        sort_cse_topological(*cse_vars, exprs.*);
    }

    is_jai := lang == "jai";
    is_c   := lang == "c";
    // The wdf_lib SIMD matmul helpers only exist for C++; every other
    // target language falls back to the manual-loop codegen below.
    use_helpers := lang == "cpp";

    // Jai has no namespaces and #load merges a file's declarations straight
    // into the caller's scope, so (unlike C++, which wraps everything
    // in `namespace rtype_<node_name>`) every top-level Jai/C symbol here
    // needs a node-unique prefix. This must match the
    // jai_prefix=rtype_<name>_ / c_prefix=rtype_<name>_ convention wired
    // into the Custom(...) token in wdf_compiler.jai.
    sym_prefix := ifx (is_jai || is_c) then tprint("rtype_%_", node_name) else "";
    qualify :: (prefix: string, base: string) -> string {
        if prefix.count == 0  return base;
        return tprint("%1%2", prefix, base);
    }
    // C struct types are referenced via their tag (`struct Name`), unlike
    // C++ which allows the bare name; used to prefix pointer/type usages.
    struct_kw := ifx is_c then "struct " else "";

    n_ports_name        := qualify(sym_prefix, "n_ports");
    n_children_name      := qualify(sym_prefix, "n_children");
    n_ports_padded_name  := qualify(sym_prefix, "n_ports_padded");
    adapt_port_name      := qualify(sym_prefix, "adapt_port");
    r_params_name        := qualify(sym_prefix, "R_Params");
    r_vars_name          := qualify(sym_prefix, "R_Vars");
    update_vars_name     := qualify(sym_prefix, "update_vars");
    reflected_name       := qualify(sym_prefix, "reflected");
    incident_name        := qualify(sym_prefix, "incident");
    compute_name         := qualify(sym_prefix, "compute");

    // `vars` (and, in Jai, `params`) are accessed through `.` there and `->`
    // in C++; sharing one operator string lets the loops below stay
    // language-agnostic instead of forking into parallel cpp/jai copies.
    mref     := ifx is_jai then "." else "->";
    vars_ref := tprint("vars%", mref);
    S_ref    := tprint("%S", vars_ref);
    zero_lit := ifx is_jai then "0.0" else "0.0f";

    decl_const_int :: (builder: *String_Builder, is_jai: bool, is_c: bool, name: string, value: string) {
        if is_jai       print_to_builder(builder, "%1 :: %2;\n", name, value);
        else if is_c     print_to_builder(builder, "#define %1 (%2)\n", name, value);
        else              print_to_builder(builder, "static constexpr int %1 = %2;\n", name, value);
    }
    struct_open :: (builder: *String_Builder, is_jai: bool, name: string) {
        if is_jai  print_to_builder(builder, "%1 :: struct {\n", name);
        else       print_to_builder(builder, "struct % {\n", name);
    }
    struct_close :: (builder: *String_Builder, is_jai: bool) {
        if is_jai  print_to_builder(builder, "}\n\n");
        else       print_to_builder(builder, "};\n\n");
    }
    // default_str == "" means "value-initialize / zero-init only" (no
    // explicit value): Jai zero-inits by default, C++ needs an explicit {}.
    // C does not support either.
    struct_field :: (builder: *String_Builder, is_jai: bool, is_c: bool, name: string, type_str: string, default_str: string) {
        if is_jai {
            if default_str.count == 0  print_to_builder(builder, "    %1: %2;\n", name, type_str);
            else                       print_to_builder(builder, "    %1: %2 = %3;\n", name, type_str, default_str);
        } else if is_c {
            print_to_builder(builder, "    %2 %1;\n", name, type_str);
        } else {
            if default_str.count == 0  print_to_builder(builder, "    %2 %1{};\n", name, type_str);
            else                       print_to_builder(builder, "    %2 %1 = %3;\n", name, type_str, default_str);
        }
    }
    array_field :: (builder: *String_Builder, is_jai: bool, is_c: bool, name: string, elem_type: string, size_expr: string, cpp_alignas: string) {
        if is_jai {
            print_to_builder(builder, "    %1: [%2] %3;\n", name, size_expr, elem_type);
        } else if is_c {
            print_to_builder(builder, "    %1 %2[%3];\n", elem_type, name, size_expr);
        } else if cpp_alignas.count > 0 {
            print_to_builder(builder, "    alignas (%1) %2 %3[%4]{};\n", cpp_alignas, elem_type, name, size_expr);
        } else {
            print_to_builder(builder, "    %1 %2[%3]{};\n", elem_type, name, size_expr);
        }
    }
    // Declares a typed immutable-looking local (`const float x = ...;` in
    // C++, `x: float = ...;` in Jai) -- used for both CSE temporaries and
    // unpacked R_Params fields.
    decl_local_float :: (builder: *String_Builder, is_jai: bool, name: string, expr_str: string) {
        if is_jai  print_to_builder(builder, "    %1: float = %2;\n", name, expr_str);
        else       print_to_builder(builder, "    const float %1 = %2;\n", name, expr_str);
    }
    decl_mut_float :: (builder: *String_Builder, is_jai: bool, name: string, init_expr: string) {
        if is_jai  print_to_builder(builder, "    %1 := %2;\n", name, init_expr);
        else       print_to_builder(builder, "    float %1 = %2;\n", name, init_expr);
    }
    float_default :: (value: string, is_jai: bool) -> string {
        return ifx is_jai then value else tprint("%1f", value);
    }

    // File preamble.
    print_to_builder(*builder, "// Auto-generated by wdf_compiler -- do not edit manually.\n");
    if is_jai {
        print_to_builder(*builder, "\n");
    } else {
        print_to_builder(*builder, "#pragma once\n\n");
        if use_helpers  print_to_builder(*builder, "#include <wdf_lib_rtype_helpers.h>\n\n");
        if !is_c  print_to_builder(*builder, "namespace rtype_% {\n\n", node_name);
    }
    decl_const_int(*builder, is_jai, is_c, n_ports_name, tprint("%", n_ports));
    decl_const_int(*builder, is_jai, is_c, n_children_name, tprint("%", n_children));
    if use_helpers {
        // Column stride for vars->S, padded to the SIMD width so
        // wdf_lib::aligned_matmul can load/store full vectors.
        decl_const_int(*builder, is_jai, is_c, n_ports_padded_name, tprint("wdf_lib::pad_to_multiple (%, 4)", n_ports_name));
    }
    if adapt >= 0 {
        // use_helpers reindexes the up-port to local slot n_children (see
        // local_port[] above), so adapt_port must reflect that local index
        // there instead of the solver's physical port index.
        adapt_port_val := ifx use_helpers then n_children else adapt;
        decl_const_int(*builder, is_jai, is_c, adapt_port_name, tprint("%", adapt_port_val));
    }
    print_to_builder(*builder, "\n");

    // Structs.
    struct_open(*builder, is_jai, r_params_name);
    for name : extra_symbols {
        struct_field(*builder, is_jai, is_c, name, "float", float_default("0.0", is_jai));
    }
    struct_close(*builder, is_jai);

    struct_open(*builder, is_jai, r_vars_name);
    if use_helpers {
        array_field(*builder, is_jai, is_c, "S", "float", tprint("% * %", n_ports_name, n_ports_padded_name), "16");
    } else {
        array_field(*builder, is_jai, is_c, "S", "float", tprint("% * %", n_ports_name, n_ports_name), "");
    }
    if adapt >= 0  struct_field(*builder, is_jai, is_c, "R_adapt", "float", "");
    struct_close(*builder, is_jai);

    // update_vars.
    if is_jai {
        print_to_builder(*builder, "%1 :: (vars: *%2, params: %3", update_vars_name, r_vars_name, r_params_name);
        for name : ordered_child_names  print_to_builder(*builder, ", %1: float, G_%1: float", name);
        print_to_builder(*builder, ") -> float {\n");
    } else {
        print_to_builder(*builder, "static inline float %(\n", update_vars_name);
        print_to_builder(*builder, "    %1%2* vars,\n", struct_kw, r_vars_name);
        if extra_symbols.count > 0 {
            print_to_builder(*builder, "    const %1%2* params", struct_kw, r_params_name);
        } else {
            if is_c  print_to_builder(*builder, "    const %1%2* params", struct_kw, r_params_name);
               else  print_to_builder(*builder, "    const %1%2* /*params*/", struct_kw, r_params_name);
        }
        if is_c {
            for name : ordered_child_names  print_to_builder(*builder, ",\n    float %, float G_%", name, name);
        } else {
            for name : ordered_child_names  print_to_builder(*builder, ",\n    float %, float /*G_%*/", name, name);
        }
        print_to_builder(*builder, "\n) {\n");
    }
    for name : extra_symbols  decl_local_float(*builder, is_jai, name, tprint("params%1%2", mref, name));
    if extra_symbols.count > 0  print_to_builder(*builder, "\n");
    for cv : cse_vars  decl_local_float(*builder, is_jai, cv.name, print_expr(cv.expr_id, exprs.*));
    if cse_vars.count > 0  print_to_builder(*builder, "\n");
    if use_helpers {
        // wdf_lib matmul helpers expect S stored transposed relative to the
        // physical [output_row][input_col] convention: S[input][output].
        // local_port[] maps each local (helper) slot to its physical port,
        // used identically for the row (input) and column (output) role.
        for row_local : 0..n_ports-1 {
            phys_col := local_port[row_local]; // input port for this row
            for col_local : 0..n_ports-1 {
                phys_row := local_port[col_local]; // output port for this col
                gv := expr_gv_idx[phys_row * n_ports + phys_col];
                if gv < 0 {
                    print_to_builder(*builder, "    vars->S[% * n_ports_padded + %] = 0.0f;\n", row_local, col_local);
                } else {
                    print_to_builder(*builder, "    vars->S[% * n_ports_padded + %] = %;\n",
                        row_local, col_local, print_expr(gen_var_ids[gv], exprs.*));
                }
            }
        }
    } else {
        for i : 0..n_ports*n_ports-1 {
            row := i / n_ports;
            col := i % n_ports;
            gv  := expr_gv_idx[i];
            if gv < 0 {
                print_to_builder(*builder, "    %[% * % + %] = %;\n", S_ref, row, n_ports_name, col, zero_lit);
            } else {
                print_to_builder(*builder, "    %[% * % + %] = %;\n",
                    S_ref, row, n_ports_name, col, print_expr(gen_var_ids[gv], exprs.*));
            }
        }
    }
    print_to_builder(*builder, "\n");
    if adapt >= 0 && adapt_gv_idx >= 0 {
        decl_local_float(*builder, is_jai, "R_adapt_val", print_expr(gen_var_ids[adapt_gv_idx], exprs.*));
        print_to_builder(*builder, "    %1R_adapt = R_adapt_val;\n", vars_ref);
        print_to_builder(*builder, "    return R_adapt_val;\n");
    } else {
        print_to_builder(*builder, "    return %;\n", zero_lit);
    }
    print_to_builder(*builder, "}\n\n");

    if adapt >= 0 {
        // reflected: b = sum_k  S[adapt][port_k] * a_in[k]
        if is_jai {
            print_to_builder(*builder, "%1 :: (vars: *%2, a_in: *float) -> float {\n", reflected_name, r_vars_name);
        } else {
            print_to_builder(*builder, "static inline float %(\n", reflected_name);
            print_to_builder(*builder, "    const %1%2* vars,\n", struct_kw, r_vars_name);
            print_to_builder(*builder, "    const float* a_in  // [n_children]\n");
            print_to_builder(*builder, ") {\n");
        }
        if use_helpers {
            // adapt_port is the local slot of the up/adapt port (see
            // local_port[] above); a_in already matches local slots
            // 0..n_children-1 with zero offset, so this is a direct
            // single-output matmul.
            print_to_builder(*builder,
                "    return wdf_lib::single_output_matmul<n_children, n_ports_padded>(vars->S, a_in, adapt_port);\n");
        } else {
            decl_mut_float(*builder, is_jai, "b", zero_lit);
            for k : 0..n_children-1 {
                port_k := child_to_port[k];
                print_to_builder(*builder, "    b += %[% * % + %] * a_in[%];\n",
                    S_ref, adapt, n_ports_name, port_k, k);
            }
            print_to_builder(*builder, "    return b;\n");
        }
        print_to_builder(*builder, "}\n\n");

        // incident: b_out[k] = S[port_k][adapt]*a_up + sum_k2 S[port_k][port_k2]*a_in[k2]
        if is_jai {
            print_to_builder(*builder, "%1 :: (vars: *%2, a_up: float, a_in: *float, b_out: *float) {\n", incident_name, r_vars_name);
        } else {
            print_to_builder(*builder, "static inline void %(\n", incident_name);
            print_to_builder(*builder, "    const %1%2* vars,\n", struct_kw, r_vars_name);
            print_to_builder(*builder, "    float a_up,\n");
            print_to_builder(*builder, "    const float* a_in,  // [n_children]\n");
            print_to_builder(*builder, "    float* b_out        // [n_children]\n");
            print_to_builder(*builder, ") {\n");
        }
        if use_helpers {
            print_to_builder(*builder, "    float a_full[n_ports];\n");
            for k : 0..n_children-1 {
                print_to_builder(*builder, "    a_full[%] = a_in[%];\n", k, k);
            }
            print_to_builder(*builder, "    a_full[adapt_port] = a_up;\n");
            print_to_builder(*builder, "    alignas (16) float b_full[n_ports_padded];\n");
            print_to_builder(*builder,
                "    wdf_lib::aligned_matmul<n_ports, n_ports_padded>(vars->S, a_full, b_full);\n");
            for k : 0..n_children-1 {
                print_to_builder(*builder, "    b_out[%] = b_full[%];\n", k, k);
            }
        } else {
            for k : 0..n_children-1 {
                port_k := child_to_port[k];
                print_to_builder(*builder, "    b_out[%] = %[% * % + %] * a_up",
                    k, S_ref, port_k, n_ports_name, adapt);
                for k2 : 0..n_children-1 {
                    port_k2 := child_to_port[k2];
                    print_to_builder(*builder, "\n              + %[% * % + %] * a_in[%]",
                        S_ref, port_k, n_ports_name, port_k2, k2);
                }
                print_to_builder(*builder, ";\n");
            }
        }
        print_to_builder(*builder, "}\n\n");
    } else {
        // Root RIGID node (no adapt port, no parent to defer to): a single
        // compute() step gathers every child's b (as a_in), applies the full
        // n_ports x n_ports S matrix, and scatters the results back out as
        // every child's a (b_out) -- matching how the hand-written
        // bassman_tone_stack.wdf's root Custom(...) element uses a single
        // "compute=" callback instead of separate incident=/reflected=.
        if is_jai {
            print_to_builder(*builder, "%1 :: (vars: *%2, a_in: *float, b_out: *float) {\n", compute_name, r_vars_name);
        } else {
            print_to_builder(*builder, "static inline void %(\n", compute_name);
            print_to_builder(*builder, "    const %1%2* vars,\n", struct_kw, r_vars_name);
            print_to_builder(*builder, "    const float* a_in,  // [n_children]\n");
            print_to_builder(*builder, "    float* b_out        // [n_children]\n");
            print_to_builder(*builder, ") {\n");
        }
        if use_helpers {
            // b_out is a caller-owned buffer sized to exactly n_children
            // (== n_ports here) elements -- aligned_matmul's SIMD stores
            // write in groups of 4 up to n_ports_padded, which would
            // overrun it directly, so stage into a local padded/aligned
            // buffer and copy out the real entries.
            print_to_builder(*builder, "    alignas (16) float b_full[n_ports_padded];\n");
            print_to_builder(*builder,
                "    wdf_lib::aligned_matmul<n_ports, n_ports_padded>(vars->S, a_in, b_full);\n");
            for k : 0..n_children-1 {
                print_to_builder(*builder, "    b_out[%] = b_full[%];\n", k, k);
            }
        } else {
            for k : 0..n_children-1 {
                port_k := child_to_port[k];
                print_to_builder(*builder, "    b_out[%] = %[% * % + %] * a_in[0]",
                    k, S_ref, port_k, n_ports_name, child_to_port[0]);
                for k2 : 1..n_children-1 {
                    port_k2 := child_to_port[k2];
                    print_to_builder(*builder, "\n              + %[% * % + %] * a_in[%]",
                        S_ref, port_k, n_ports_name, port_k2, k2);
                }
                print_to_builder(*builder, ";\n");
            }
        }
        print_to_builder(*builder, "}\n\n");
    }

    if !is_jai && !is_c  print_to_builder(*builder, "} // namespace rtype_%\n", node_name);
    return builder_to_string(*builder);
}

// ===========================================================================
// PRINT HELPERS
// ===========================================================================
// We can maybe move these methods somewhere else, since most things don't need them?
// // Build a canonical rational form for display/debugging:
//   e -> N / D
// then clear reciprocal symbols (1/Rx factors) by multiplying N and D
// by each requested symbol until no direct reciprocal remains.
canonicalize_for_print :: (e: Expr_ID, clear_symbols: [] string, exprs: *[..] Expr) -> Expr_ID {
    one := expr_const(1, exprs);

    Rat :: struct {
        num: Expr_ID;
        den: Expr_ID;
    }

    to_rational :: (x: Expr_ID, one: Expr_ID, exprs: *[..] Expr) -> Rat {
        node := exprs.*[x];
        if node.kind == {
            case .NULL;
                return .{ num = expr_const(0, exprs), den = one };

            case .CONST; #through;
            case .SYMBOL;
                return .{ num = x, den = one };

            case .NEG;
                r := to_rational(node.left, one, exprs);
                return .{ num = expr_neg(r.num, exprs), den = r.den };

            case .ADD; #through;
            case .SUB;
                l := to_rational(node.left,  one, exprs);
                r := to_rational(node.right, one, exprs);
                l_scaled := expr_mul(l.num, r.den, exprs);
                r_scaled := expr_mul(r.num, l.den, exprs);
                return .{
                    num = expr_bin(node.kind, l_scaled, r_scaled, exprs),
                    den = expr_mul(l.den, r.den, exprs),
                };

            case .MUL;
                l := to_rational(node.left,  one, exprs);
                r := to_rational(node.right, one, exprs);
                return .{
                    num = expr_mul(l.num, r.num, exprs),
                    den = expr_mul(l.den, r.den, exprs),
                };

            case .DIV;
                l := to_rational(node.left,  one, exprs);
                r := to_rational(node.right, one, exprs);
                return .{
                    num = expr_mul(l.num, r.den, exprs),
                    den = expr_mul(l.den, r.num, exprs),
                };
        }

        return .{ num = x, den = one };
    }

    has_direct_reciprocal :: (x: Expr_ID, symbol_name: string, exprs: *[..] Expr) -> bool {
        node := exprs.*[x];
        if node.kind == .DIV {
            right := exprs.*[node.right];
            if right.kind == .SYMBOL && right.name == symbol_name  return true;
        }
        if node.kind == .CONST || node.kind == .SYMBOL || node.kind == .NULL  return false;
        if node.kind == .NEG  return has_direct_reciprocal(node.left, symbol_name, exprs);
        return has_direct_reciprocal(node.left, symbol_name, exprs)
            || has_direct_reciprocal(node.right, symbol_name, exprs);
    }

    rat := to_rational(e, one, exprs);
    num := simplify(rat.num, exprs);
    den := simplify(rat.den, exprs);

    // Cancel common symbolic factors from numerator and denominator.
    for name : clear_symbols {
        sym := expr_symbol(name, exprs);
        for _: 0..4 {
            ok_n, new_num := try_cancel(num, sym, exprs);
            ok_d, new_den := try_cancel(den, sym, exprs);
            if ok_n && ok_d {
                num = simplify(new_num, exprs);
                den = simplify(new_den, exprs);
            } else {
                break;
            }
        }
    }

    // Cancel small common integer factors.
    for k : 2..9 {
        k_expr := expr_const(k, exprs);
        for _: 0..4 {
            ok_n, new_num := try_cancel(num, k_expr, exprs);
            ok_d, new_den := try_cancel(den, k_expr, exprs);
            if ok_n && ok_d {
                num = simplify(new_num, exprs);
                den = simplify(new_den, exprs);
            } else {
                break;
            }
        }
    }

    // Clear direct reciprocal factors 1/Rx.
    for name : clear_symbols {
        sym := expr_symbol(name, exprs);
        for _: 0..4 {
            has_rec := has_direct_reciprocal(num, name, exprs)
                    || has_direct_reciprocal(den, name, exprs);
            if !has_rec  break;

            num = simplify(expr_mul(num, sym, exprs), exprs);
            den = simplify(expr_mul(den, sym, exprs), exprs);
        }
    }

    den_node := exprs.*[den];
    if den_node.kind == .NEG {
        num = simplify(expr_neg(num, exprs), exprs);
        den = den_node.left;
    }

    return simplify(expr_div(num, den, exprs), exprs);
}

print_rtype :: (result: R_Solver_Result, topo: R_Type_Topology) {
    clear_symbols: [..] string;
    for p : topo.ports {
        array_add(*clear_symbols, p.impedance_name);
    }
    for v : topo.vcvs {
        array_add(*clear_symbols, v.ri_name);
        array_add(*clear_symbols, v.ro_name);
    }

    for j : 0..result.n_ports-1 {
        for k : 0..result.n_ports-1 {
            e := result.S_matrix[j * result.n_ports + k];
            s := "0";
            if e != NO_EXPR {
                e_canon := canonicalize_for_print(e, clear_symbols, *result.exprs);
                s = print_expr(e_canon, result.exprs);
            }
            print("S[%][%] = %\n", j, k, s);
        }
    }
    if result.adapt_port >= 0 {
        r_canon := canonicalize_for_print(result.adapt_impedance, clear_symbols, *result.exprs);
        print("R_adapt = %\n", print_expr(r_canon, result.exprs));
    }
}

#scope_file
#import "Basic";
#import "wdf_sym_algebra";
