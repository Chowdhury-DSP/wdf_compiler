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
    n_nodes  := topo.n_verts + n_ports; // original + intermediate nodes

    // Total unknowns after removing datum row/col
    dim := n_nodes - 1 + n_ports + n_extras;

    result.n_ports    = n_ports;
    result.adapt_port = topo.adapt_port;
    for port : topo.ports  array_add(*result.port_impedance_names, port.impedance_name);

    // port_R_syms[k] = the Expr_ID to use as R_k in the S-matrix formula.
    // For non-adapt ports this is the impedance symbol; for the adapt port
    // it is 1/G_a (so that S[a,a] remains rational in G_a, not Ra).
    port_R_syms: [..] Expr_ID;

    // ---------------------------------------------------------------
    // 1. Create symbolic unknowns
    // ---------------------------------------------------------------
    //   unknowns[n-1]                  = node voltage v_n  (n = 1..n_nodes-1)
    //   unknowns[port_base + k]        = port current  i_k  (k = 0..n_ports-1)
    //   unknowns[extra_base + j]       = VCVS current  ie_j (j = 0..n_extras-1)
    unknowns: [..] Expr_ID;
    for n : 1..n_nodes-1  array_add(*unknowns, expr_symbol(tprint("_v%", n), exprs));
    for k : 0..n_ports-1  array_add(*unknowns, expr_symbol(tprint("_i%", k), exprs));
    for j : 0..n_extras-1 array_add(*unknowns, expr_symbol(tprint("_ie%", j), exprs));
    assert(unknowns.count == dim);

    port_base  := n_nodes - 1;           // unknowns[port_base + k]  = i_k
    extra_base := n_nodes - 1 + n_ports; // unknowns[extra_base + j] = ie_j

    // ---------------------------------------------------------------
    // 2. Build MNA matrix X  (dim × dim, row-major, entries are Expr_IDs)
    // ---------------------------------------------------------------
    X: [..] Expr_ID;
    array_resize(*X, dim * dim);
    for i : 0..X.count-1  X[i] = NO_EXPR;

    one     := expr_const( 1, exprs);
    neg_one := expr_neg(one, exprs);

    // Accumulate a symbolic term into X[row, col].
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

    // Stamp each WDF port.
    // For the adapt port we introduce a direct conductance symbol G_{name}
    // (rather than the expression 1/R_{name}).  This keeps the adapt
    // equation  S[a,a] = 0  linear in G_a so that sym_algebra.solve works.
    // The output adapt_impedance is then expressed as R_a = 1/G_a_solution.
    for k : 0..n_ports-1 {
        port    := topo.ports[k];
        mid_col := (topo.n_verts + k) - 1; // node_to_col for intermediate node
        pc      := port_base + k;           // port-current column

        G_k: Expr_ID;
        R_k: Expr_ID; // used in S-matrix formula (= 1/G_k for adapt, = R_sym otherwise)
        if k == topo.adapt_port {
            // Treat conductance as a first-class symbol to keep things linear.
            G_k = expr_symbol(tprint("G_%", port.impedance_name), exprs);
            R_k = expr_div(one, G_k, exprs);
        } else {
            R_sym := expr_symbol(port.impedance_name, exprs);
            G_k   = expr_div(one, R_sym, exprs);
            R_k   = R_sym;
        }
        neg_Gk := expr_neg(G_k, exprs);
        v2c    := port.v2 - 1;  // v2 is always non-datum

        // Resistor R_k between v2 and mid_k
        xadd(*X, v2c,    v2c,    dim, G_k,    exprs);
        xadd(*X, mid_col,mid_col,dim, G_k,    exprs);
        xadd(*X, v2c,    mid_col,dim, neg_Gk, exprs);
        xadd(*X, mid_col,v2c,    dim, neg_Gk, exprs);

        // Voltage source V_k from v1 to mid_k
        xadd(*X, pc,     mid_col, dim, neg_one, exprs);
        xadd(*X, mid_col,pc,      dim, neg_one, exprs);
        if port.v1 != 0 {
            v1c := port.v1 - 1;
            xadd(*X, pc,  v1c, dim, one, exprs);
            xadd(*X, v1c, pc,  dim, one, exprs);
        }

        // Store R_k for use in the S-matrix formula below.
        // We use a side array indexed by port number.
        array_add(*port_R_syms, R_k);
    }

    // Stamp each VCVS element
    for j : 0..n_extras-1 {
        vcvs   := topo.vcvs[j];
        exc    := extra_base + j; // extra-variable column

        Ri_sym := expr_symbol(vcvs.ri_name,   exprs);
        Ro_sym := expr_symbol(vcvs.ro_name,   exprs);
        Ag_sym := expr_symbol(vcvs.gain_name, exprs);
        Gi     := expr_div(one, Ri_sym, exprs);
        neg_Gi := expr_neg(Gi,     exprs);
        neg_Ag := expr_neg(Ag_sym, exprs);

        // Input admittance 1/Ri between out_n1 and out_n2
        o1v := vcvs.out_n1 != 0;
        o2v := vcvs.out_n2 != 0;
        o1  := vcvs.out_n1 - 1;
        o2  := vcvs.out_n2 - 1;
        if o1v  xadd(*X, o1, o1, dim, Gi,     exprs);
        if o2v  xadd(*X, o2, o2, dim, Gi,     exprs);
        if o1v && o2v {
            xadd(*X, o1, o2, dim, neg_Gi, exprs);
            xadd(*X, o2, o1, dim, neg_Gi, exprs);
        }

        // VCVS stamps in the extra row/col:
        //   extra_row: -gain*V_o1 + gain*V_o2 + V_c1 - V_c2 + Ro*ie = 0
        //   extra_col: +1 at ctrl_n1, -1 at ctrl_n2
        if o1v  xadd(*X, exc, o1, dim, neg_Ag, exprs);
        if o2v  xadd(*X, exc, o2, dim, Ag_sym, exprs);

        c1v := vcvs.ctrl_n1 != 0;
        c2v := vcvs.ctrl_n2 != 0;
        c1  := vcvs.ctrl_n1 - 1;
        c2  := vcvs.ctrl_n2 - 1;
        if c1v { xadd(*X, exc, c1, dim, one,     exprs);  xadd(*X, c1, exc, dim, one,     exprs); }
        if c2v { xadd(*X, exc, c2, dim, neg_one, exprs);  xadd(*X, c2, exc, dim, neg_one, exprs); }
        xadd(*X, exc, exc, dim, Ro_sym, exprs);
    }

    // ---------------------------------------------------------------
    // 3. Pre-compute one symbolic row expression per MNA row
    //    row_exprs[i] = sum_j  X[i,j] * unknowns[j]
    //
    // NOTE: do NOT call fold on individual terms here.  fold may apply the
    // identity MUL(1, unknown) → unknown by copying the SYMBOL struct to a
    // new slot, giving it a different Expr_ID than the canonical unknown.
    // linearize compares by Expr_ID, so it would misclassify that alias as
    // a constant parameter rather than an unknown variable.
    // ---------------------------------------------------------------
    row_exprs: [..] Expr_ID;
    for i : 0..dim-1 {
        sum := NO_EXPR;
        for j : 0..dim-1 {
            xij := X[i * dim + j];
            if xij == NO_EXPR  continue;
            term := expr_mul(xij, unknowns[j], exprs);
            sum = combine(sum, term, .ADD, exprs);
        }
        array_add(*row_exprs, sum);
    }

    // ---------------------------------------------------------------
    // 4. For each port excitation k, solve X·x = e_{port_k},
    //    then extract column k of the S matrix.
    // ---------------------------------------------------------------
    array_resize(*result.S_matrix, n_ports * n_ports);

    // Reuse this for adaptation instead of solving the adapt excitation twice.
    adapt_xaa: Expr_ID = NO_EXPR;

    for k : 0..n_ports-1 {
        pc_k := port_base + k; // index of the port-k current unknown

        // equations[i] = row_exprs[i] - delta(i, pc_k) = 0
        equations: [..] Expr_ID;
        for i : 0..dim-1 {
            rhs := ifx i == pc_k then one else NO_EXPR;
            eq  := combine(row_exprs[i], rhs, .SUB, exprs);
            if eq == NO_EXPR  eq = expr_const(0, exprs);
            array_add(*equations, eq);
        }

        solutions := solve(equations, unknowns, exprs);

        if k == topo.adapt_port {
            adapt_xaa = solutions[port_base + k];
        }

        // S[j,k] = delta(j,k)  +  2 * R_j * solutions[port_base + j]
        for j : 0..n_ports-1 {
            xinv_jk := solutions[port_base + j];

            R_j  := port_R_syms[j];  // 1/G_a for adapt port, R_sym for others
            two  := expr_const(2, exprs);
            term := expr_mul(expr_mul(two, R_j, exprs), xinv_jk, exprs);
            fold(term, exprs);

            delta := ifx j == k then one else NO_EXPR;
            s_jk  := combine(delta, term, .ADD, exprs);
            if s_jk != NO_EXPR {
                s_jk = simplify(s_jk, exprs);
            }
            result.S_matrix[j * n_ports + k] = s_jk;
        }
    }

    // ---------------------------------------------------------------
    // 5. Adapt port: solve S[a][a] = 0 for G_a, then convert to R_a.
    //
    // For the adapt port, the MNA is stamped with conductance G_a and the
    // S formula uses R_a = 1/G_a. Let x = Xinv(port_current_a, port_current_a).
    //
    //   S[a,a] = 1 + 2 * (1/G_a) * x
    //
    // If x = N/D, then S[a,a] = 0 is equivalent to:
    //   G_a * D + 2 * N = 0
    // which is linear in G_a and avoids non-constant divisors in linearize().
    // ---------------------------------------------------------------
    if topo.adapt_port >= 0 {
        a       := topo.adapt_port;
        ga_name := tprint("G_%", topo.ports[a].impedance_name);
        Ga      := expr_symbol(ga_name, exprs);

        contains_symbol_name :: (e: Expr_ID, symbol_name: string, exprs: *[..] Expr) -> bool {
            node := exprs.*[e];
            if node.kind == .SYMBOL  return node.name == symbol_name;
            if node.kind == .CONST || node.kind == .NULL  return false;
            if node.kind == .NEG  return contains_symbol_name(node.left, symbol_name, exprs);
            return contains_symbol_name(node.left, symbol_name, exprs)
                || contains_symbol_name(node.right, symbol_name, exprs);
        }

        // Try to divide expression by one factor of `symbol_name` structurally.
        // This is stricter than generic algebraic cancellation and is only used
        // for the adaptation equation where we expect a global G_a factor.
        divide_out_symbol_once :: (e: Expr_ID, symbol_name: string, exprs: *[..] Expr) -> (bool, Expr_ID) {
            node := exprs.*[e];
            if node.kind == {
                case .SYMBOL;
                    if node.name == symbol_name  return true, expr_const(1, exprs);
                    return false, NO_EXPR;

                case .CONST; #through;
                case .NULL;
                    return false, NO_EXPR;

                case .NEG;
                    ok, q := divide_out_symbol_once(node.left, symbol_name, exprs);
                    if !ok  return false, NO_EXPR;
                    return true, expr_neg(q, exprs);

                case .ADD; #through;
                case .SUB;
                    ok_l, q_l := divide_out_symbol_once(node.left,  symbol_name, exprs);
                    if !ok_l  return false, NO_EXPR;
                    ok_r, q_r := divide_out_symbol_once(node.right, symbol_name, exprs);
                    if !ok_r  return false, NO_EXPR;
                    return true, expr_bin(node.kind, q_l, q_r, exprs);

                case .MUL;
                    l_has := contains_symbol_name(node.left,  symbol_name, exprs);
                    r_has := contains_symbol_name(node.right, symbol_name, exprs);

                    if l_has {
                        ok_l, q_l := divide_out_symbol_once(node.left, symbol_name, exprs);
                        if ok_l  return true, expr_mul(q_l, node.right, exprs);
                    }
                    if r_has {
                        ok_r, q_r := divide_out_symbol_once(node.right, symbol_name, exprs);
                        if ok_r  return true, expr_mul(node.left, q_r, exprs);
                    }
                    return false, NO_EXPR;

                case .DIV;
                    // We only divide factors out of numerators.
                    if contains_symbol_name(node.right, symbol_name, exprs)
                        return false, NO_EXPR;
                    ok_n, q_n := divide_out_symbol_once(node.left, symbol_name, exprs);
                    if !ok_n  return false, NO_EXPR;
                    return true, expr_div(q_n, node.right, exprs);
            }

            return false, NO_EXPR;
        }

        // Reuse x_aa from the existing per-port solve loop above.
        x_aa := adapt_xaa;
        assert(x_aa != NO_EXPR);

        two := expr_const(2, exprs);
        adapt_eq_g: Expr_ID;

        x_node := exprs.*[x_aa];
        if x_node.kind == .DIV {
            num := simplify(x_node.left,  exprs);
            den := simplify(x_node.right, exprs);

            // Simplify x by removing common G_a factors in numerator/denominator.
            for _: 0..4 {
                ok_n, new_num := try_cancel(num, Ga, exprs);
                ok_d, new_den := try_cancel(den, Ga, exprs);
                if ok_n && ok_d {
                    num = new_num;
                    den = new_den;
                } else {
                    break;
                }
            }

            // Prefer cancelling the outer 1/G_a factor from S[a,a] = 1 + 2*(1/G_a)*x.
            // If x = G_a * N / D then this becomes 1 + 2*N/D, avoiding quadratic terms.
            ok_outer, num_no_g := try_cancel(num, Ga, exprs);
            if ok_outer {
                adapt_eq_g = expr_bin(.ADD, den, expr_mul(two, num_no_g, exprs), exprs);
            } else {
                // Fallback: clear the outer division by multiplying through by G_a.
                gd := expr_mul(Ga, den, exprs);
                tn := expr_mul(two, num, exprs);
                adapt_eq_g = expr_bin(.ADD, gd, tn, exprs);
            }
        } else {
            // x has no explicit top-level denominator.
            ok_outer, x_no_g := try_cancel(x_aa, Ga, exprs);
            if ok_outer {
                adapt_eq_g = expr_bin(.ADD, one, expr_mul(two, x_no_g, exprs), exprs);
            } else {
                adapt_eq_g = expr_bin(.ADD, Ga, expr_mul(two, x_aa, exprs), exprs);
            }
        }

        adapt_eq_g = simplify(adapt_eq_g, exprs);

        // Remove one global G_a factor before calling solve().
        // This discards the trivial root G_a=0 and leaves the physical affine equation.
        ok_div, reduced_eq := divide_out_symbol_once(adapt_eq_g, ga_name, exprs);
        if ok_div {
            adapt_eq_g = simplify(reduced_eq, exprs);
        }

        adapt_solutions_g := solve(.[adapt_eq_g], .[Ga], exprs);
        adapt_g := simplify(adapt_solutions_g[0], exprs);

        // Report adapted resistance R_a = 1 / G_a.
        result.adapt_impedance = simplify(expr_div(one, adapt_g, exprs), exprs);

        // Substitute solved G_a back into S and simplify.
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

#scope_file
#import "Basic";
#import "sym_algebra";
