// Build a canonical rational form for display/debugging:
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

// ============================================================
// TEST  (temporary — remove once wired into the compiler)
// ============================================================

// Runs the 2-port + VCVS test case taken from the Mathematica reference.
// Prints the symbolic S matrix; compare against the expected output below.
test_r_solver :: () {
    // Mathematica netlist (1-indexed nodes):
    //   Va 1 4 Vin   }  port 0: v1=0(datum), v2=1, impedance="Ra"
    //   Ra 2 4 Vin   }  (Mathematica nodes → Jai 0-indexed)
    //   Vb 2 5 Rfb   }  port 1: v1=1, v2=2, impedance="Rb"
    //   Rb 3 5 Rfb   }
    //   E1 1 2 3 1 Ag   VCVS: out=(0,1), ctrl=(2,0), gain=Ag, ri=Ri, ro=Ro
    //
    // MNA circuit topology (0-indexed, node 0 = datum):
    //   n_verts = 3  (nodes 0, 1, 2)
    //   port 0:  V_0 from node 0 → mid_0=3,  R_0 from node 1 → mid_0=3
    //   port 1:  V_1 from node 1 → mid_1=4,  R_1 from node 2 → mid_1=4
    //   E1:      output (0,1), control (2,0), gain=Ag, ri=Ri, ro=Ro
    //
    // Expected S-matrix (portToAdapt=0 in Mathematica → no adaptation):
    //   let denom = Ri*(Rb - Ro) + Ra*(Rb + Ri*(1 + Ag) - Ro)
    //   S[0,0]  =  (Ri*(Rb - Ro) - Ra*(Rb + Ri*(1 + Ag) - Ro)) / denom
    //   S[0,1]  =  -2*Ra*Ri / denom
    //   S[1,0]  =  -2*(1 + Ag)*Rb*Ri / denom
    //   S[1,1]  =  1 - 2*Rb*(Ra + Ri) / denom

    topo: R_Type_Topology;
    topo.n_verts    = 3;
    topo.adapt_port = -1; // no adaptation in this test

    // Port 0: Va(1→4), Ra(2→4) — v1=0(datum), v2=1, impedance="Ra"
    array_add(*topo.ports, .{ v1 = 0, v2 = 1, impedance_name = "Ra" });

    // Port 1: Vb(2→5), Rb(3→5) — v1=1, v2=2, impedance="Rb"
    array_add(*topo.ports, .{ v1 = 1, v2 = 2, impedance_name = "Rb" });

    // VCVS E1 1 2 3 1 Ag:
    //   output between Mth-node-1=0 and Mth-node-2=1
    //   control: Mth-node-3=2 (ctrl+), Mth-node-4=1→0-indexed=0 (datum, ctrl-)
    array_add(*topo.vcvs, .{
        out_n1    = 0,  out_n2  = 1,
        ctrl_n1   = 2,  ctrl_n2 = 0,
        gain_name = "Ag",
        ri_name   = "Ri",
        ro_name   = "Ro",
    });

    result := solve_rtype_scattering(topo);

    clear_symbols: [..] string;
    for p : topo.ports {
        array_add(*clear_symbols, p.impedance_name);
    }
    for v : topo.vcvs {
        array_add(*clear_symbols, v.ri_name);
        array_add(*clear_symbols, v.ro_name);
    }

    print("=== test_r_solver: 2-port + VCVS (no adaptation, canonicalized) ===\n");
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
    print("\n");
}

// Tests the same 2-port + VCVS circuit with port 0 adapted (S[0][0] = 0).
// Expected adapted impedance (from Mathematica):
//   Ra = Ri*(Rb - Ro) / (Rb + Ri*(1 + Ag) - Ro)
test_r_solver_adapted :: () {
    topo: R_Type_Topology;
    topo.n_verts    = 3;
    topo.adapt_port = 0; // adapt port 0 (Ra)

    array_add(*topo.ports, .{ v1 = 0, v2 = 1, impedance_name = "Ra" });
    array_add(*topo.ports, .{ v1 = 1, v2 = 2, impedance_name = "Rb" });
    array_add(*topo.vcvs, .{
        out_n1    = 0,  out_n2  = 1,
        ctrl_n1   = 2,  ctrl_n2 = 0,
        gain_name = "Ag",
        ri_name   = "Ri",
        ro_name   = "Ro",
    });

    result := solve_rtype_scattering(topo);

    eval_expr_numeric :: (e: Expr_ID, exprs: []Expr,
                          Ra: float64, Rb: float64, Ri: float64, Ro: float64, Ag: float64) -> float64 {
        node := exprs[e];
        if node.kind == {
            case .NULL;
                return 0.0;
            case .CONST;
                return cast(float64) node.value;
            case .SYMBOL;
                if node.name == "Ra"   return Ra;
                if node.name == "Rb"   return Rb;
                if node.name == "Ri"   return Ri;
                if node.name == "Ro"   return Ro;
                if node.name == "Ag"   return Ag;
                if node.name == "G_Ra" return 1.0 / Ra;
                print("[r_solver] Unknown symbol in numeric eval: %\n", node.name);
                assert(false);
                return 0.0;
            case .NEG;
                return -eval_expr_numeric(node.left, exprs, Ra, Rb, Ri, Ro, Ag);
            case .ADD;
                return eval_expr_numeric(node.left, exprs, Ra, Rb, Ri, Ro, Ag)
                     + eval_expr_numeric(node.right, exprs, Ra, Rb, Ri, Ro, Ag);
            case .SUB;
                return eval_expr_numeric(node.left, exprs, Ra, Rb, Ri, Ro, Ag)
                     - eval_expr_numeric(node.right, exprs, Ra, Rb, Ri, Ro, Ag);
            case .MUL;
                return eval_expr_numeric(node.left, exprs, Ra, Rb, Ri, Ro, Ag)
                     * eval_expr_numeric(node.right, exprs, Ra, Rb, Ri, Ro, Ag);
            case .DIV;
                denom := eval_expr_numeric(node.right, exprs, Ra, Rb, Ri, Ro, Ag);
                assert(denom != 0.0);
                return eval_expr_numeric(node.left, exprs, Ra, Rb, Ri, Ro, Ag) / denom;
        }
        return 0.0;
    }

    abs64 :: (x: float64) -> float64 { return ifx x < 0.0 then -x else x; }

    // Numeric sanity check for adaptation:
    // 1) adapted Ra matches reference formula
    // 2) S[0][0] evaluates ~ 0 after substitution
    Rb_num := 3.7;
    Ri_num := 2.3;
    Ro_num := 1.1;
    Ag_num := 4.2;

    Ra_expected := (Ri_num * (Rb_num - Ro_num)) / (Rb_num + Ri_num * (1.0 + Ag_num) - Ro_num);
    Ra_eval_1   := eval_expr_numeric(result.adapt_impedance, result.exprs, 1.0, Rb_num, Ri_num, Ro_num, Ag_num);
    Ra_eval_2   := eval_expr_numeric(result.adapt_impedance, result.exprs, 2.0, Rb_num, Ri_num, Ro_num, Ag_num);
    S00_eval    := eval_expr_numeric(result.S_matrix[0 * result.n_ports + 0], result.exprs,
                                     Ra_expected, Rb_num, Ri_num, Ro_num, Ag_num);

    print("=== test_r_solver: 2-port + VCVS (adapt port 0) ===\n");
    print("Ra_adapt = %\n", print_expr(result.adapt_impedance, result.exprs));
    print("Expected: Ri*(Rb - Ro) / (Rb + Ri*(1 + Ag) - Ro)\n");
    for j : 0..result.n_ports-1 {
        for k : 0..result.n_ports-1 {
            e := result.S_matrix[j * result.n_ports + k];
            s := ifx e != NO_EXPR then print_expr(e, result.exprs) else "0";
            print("S[%][%] = %\n", j, k, s);
        }
    }

    print("Numeric check @ (Rb=%, Ri=%, Ro=%, Ag=%):\n", Rb_num, Ri_num, Ro_num, Ag_num);
    print("  Ra_expected = %\n", Ra_expected);
    d_ra        := abs64(Ra_eval_1 - Ra_expected);
    d_ra_alias   := abs64(Ra_eval_1 - Ra_eval_2);
    d_s00        := abs64(S00_eval);

    print("  Ra_eval(ra=1) = %\n", Ra_eval_1);
    print("  Ra_eval(ra=2) = %\n", Ra_eval_2);
    print("  |ΔRa|         = %\n", d_ra);
    print("  |ΔRa_alias|   = %\n", d_ra_alias);
    print("  S[0][0]       = %\n", S00_eval);
    print("  |S[0][0]|     = %\n", d_s00);

    ra_ok  := d_ra < 1.0e-6 && d_ra_alias < 1.0e-6;
    s00_ok := d_s00 < 1.0e-7;
    print("  Ra check ok: %\n", ra_ok);
    print("  S00 check ok: %\n", s00_ok);
    if ra_ok && s00_ok {
        print("  (adaptation numeric checks passed)\n\n");
    } else {
        print("  (adaptation numeric checks FAILED)\n\n");
    }
}

run_r_solver_tests :: () {
    test_r_solver();
    test_r_solver_adapted();
}

main :: () {
    run_r_solver_tests();
}

#scope_file
#import "Basic";
#import "r_solver";
#import "sym_algebra";
