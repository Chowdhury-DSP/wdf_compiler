abs64 :: (x: float64) -> float64 { return ifx x < 0.0 then -x else x; }

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

    print("=== test_r_solver: 2-port + VCVS (no adaptation, canonicalized) ===\n");
    print_rtype(result, topo);
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
    print_rtype(result, topo);
    print("Expected: R_adapt = Ri*(Rb - Ro) / (Rb + Ri*(1 + Ag) - Ro)\n");

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

// Regression test for the port-stamping code assuming `v2` (the load-side
// vertex) is never the MNA datum. Real netlists violate that assumption
// whenever a component's "far" terminal is circuit ground (e.g. an op-amp's
// control/output nodes pull ground into a rigid component that already used
// vertex 0 for something else) — see netlist_converter's sk_lpf test case.
//
// Two resistors R1 (adapted) and R2 both wired node1 -> ground (v2 = datum)
// in parallel. This is the textbook WDF parallel-adaptor identity: adapting
// one port of a two-port parallel junction yields R_adapt = the *other*
// port's resistance, independent of R1.
test_r_solver_grounded_v2 :: () {
    topo: R_Type_Topology;
    topo.n_verts    = 2; // node 0 = datum, node 1 = the shared junction
    topo.adapt_port = 0;

    array_add(*topo.ports, .{ v1 = 1, v2 = 0, impedance_name = "R1" }); // adapted
    array_add(*topo.ports, .{ v1 = 1, v2 = 0, impedance_name = "R2" }); // fixed

    result := solve_rtype_scattering(topo);

    print("=== test_r_solver: grounded v2 on both ports (adapt port 0) ===\n");
    print_rtype(result, topo);
    print("Expected: R_adapt = R2\n");

    eval_expr_numeric :: (e: Expr_ID, exprs: []Expr, R2: float64) -> float64 {
        node := exprs[e];
        if node.kind == {
            case .NULL;   return 0.0;
            case .CONST;  return cast(float64) node.value;
            case .SYMBOL;
                if node.name == "R2" return R2;
                print("[r_solver] Unknown symbol in numeric eval: %\n", node.name);
                assert(false);
                return 0.0;
            case .NEG; return -eval_expr_numeric(node.left, exprs, R2);
            case .ADD; return eval_expr_numeric(node.left, exprs, R2) + eval_expr_numeric(node.right, exprs, R2);
            case .SUB; return eval_expr_numeric(node.left, exprs, R2) - eval_expr_numeric(node.right, exprs, R2);
            case .MUL; return eval_expr_numeric(node.left, exprs, R2) * eval_expr_numeric(node.right, exprs, R2);
            case .DIV;
                denom := eval_expr_numeric(node.right, exprs, R2);
                assert(denom != 0.0);
                return eval_expr_numeric(node.left, exprs, R2) / denom;
        }
        return 0.0;
    }

    R2_num := 4.7;
    Ra_eval := eval_expr_numeric(result.adapt_impedance, result.exprs, R2_num);
    d := ifx Ra_eval - R2_num < 0.0 then R2_num - Ra_eval else Ra_eval - R2_num;

    print("  R_adapt = %\n", Ra_eval);
    ok := result.adapt_port == 0 && d < 1.0e-9;
    print("  grounded-v2 check ok: %\n\n", ok);
}

// Shared by the two 3-port star tests below: three resistors R0, R1, R2 all
// wired node1 -> ground (v2 = datum), same shape as test_r_solver_grounded_v2
// but with a third port, to catch anything that only breaks once n_ports > 2
// (e.g. an off-by-one in the port loop, or the datum symmetry fix only being
// exercised pairwise).
build_star_3port_topology :: () -> R_Type_Topology {
    topo: R_Type_Topology;
    topo.n_verts = 2; // node 0 = datum, node 1 = the shared junction
    array_add(*topo.ports, .{ v1 = 1, v2 = 0, impedance_name = "R0" });
    array_add(*topo.ports, .{ v1 = 1, v2 = 0, impedance_name = "R1" });
    array_add(*topo.ports, .{ v1 = 1, v2 = 0, impedance_name = "R2" });
    return topo;
}

eval_expr_numeric_3r :: (e: Expr_ID, exprs: []Expr, R0: float64, R1: float64, R2: float64) -> float64 {
    node := exprs[e];
    if node.kind == {
        case .NULL;   return 0.0;
        case .CONST;  return cast(float64) node.value;
        case .SYMBOL;
            if node.name == "R0" return R0;
            if node.name == "R1" return R1;
            if node.name == "R2" return R2;
            print("[r_solver] Unknown symbol in numeric eval: %\n", node.name);
            assert(false);
            return 0.0;
        case .NEG; return -eval_expr_numeric_3r(node.left, exprs, R0, R1, R2);
        case .ADD; return eval_expr_numeric_3r(node.left, exprs, R0, R1, R2) + eval_expr_numeric_3r(node.right, exprs, R0, R1, R2);
        case .SUB; return eval_expr_numeric_3r(node.left, exprs, R0, R1, R2) - eval_expr_numeric_3r(node.right, exprs, R0, R1, R2);
        case .MUL; return eval_expr_numeric_3r(node.left, exprs, R0, R1, R2) * eval_expr_numeric_3r(node.right, exprs, R0, R1, R2);
        case .DIV;
            denom := eval_expr_numeric_3r(node.right, exprs, R0, R1, R2);
            assert(denom != 0.0);
            return eval_expr_numeric_3r(node.left, exprs, R0, R1, R2) / denom;
    }
    return 0.0;
}

// Verifies the full (unadapted) S matrix against Fettweis' classic N-port
// parallel-adaptor formula: S[i][j] = 2*Gj/Gsum - delta(i,j), where
// Gk = 1/Rk and Gsum = G0 + G1 + G2. Unlike the other tests here, this
// checks every entry, not just one derived impedance.
test_r_solver_star_3port_no_adapt :: () {
    topo := build_star_3port_topology();
    topo.adapt_port = -1;

    result := solve_rtype_scattering(topo);

    print("=== test_r_solver: 3-port resistive star (no adaptation) ===\n");
    print_rtype(result, topo);
    print("Expected: S[i][j] = 2*Gj/Gsum - delta(i,j)\n");

    R0: float64 = 2.0;
    R1: float64 = 4.7;
    R2: float64 = 9.1;
    G0 := 1.0 / R0; G1 := 1.0 / R1; G2 := 1.0 / R2;
    Gsum := G0 + G1 + G2;
    G := float64.[G0, G1, G2];

    ok := true;
    for i : 0..2 {
        for j : 0..2 {
            e := result.S_matrix[i * result.n_ports + j];
            actual := ifx e == NO_EXPR then 0.0 else eval_expr_numeric_3r(e, result.exprs, R0, R1, R2);
            expected := 2.0 * G[j] / Gsum - cast(float64) ifx i == j then 1 else 0;
            d := abs64(actual - expected);
            if d > 1.0e-9 {
                ok = false;
                print("  S[%][%] mismatch: got % expected %\n", i, j, actual, expected);
            }
        }
    }
    print("  3-port star (no adapt) check ok: %\n\n", ok);
}

// Same 3-port star, but with port 0 adapted. Classic N-port parallel
// adaptor identity: the adapted port's impedance equals the parallel
// combination of the *other* ports' impedances (generalizes
// test_r_solver_grounded_v2 from 2 to 3 ports).
test_r_solver_star_3port_adapted :: () {
    topo := build_star_3port_topology();
    topo.adapt_port = 0;

    result := solve_rtype_scattering(topo);

    print("=== test_r_solver: 3-port resistive star (adapt port 0) ===\n");
    print_rtype(result, topo);
    print("Expected: R_adapt = R1*R2 / (R1 + R2)\n");

    R1: float64 = 4.7;
    R2: float64 = 9.1;
    expected := (R1 * R2) / (R1 + R2);
    actual := eval_expr_numeric_3r(result.adapt_impedance, result.exprs, 0.0, R1, R2);
    d := abs64(actual - expected);

    print("  R_adapt = % (expected %)\n", actual, expected);
    ok := result.adapt_port == 0 && d < 1.0e-9;
    print("  3-port star (adapted) check ok: %\n\n", ok);
}

// Regression test for generate_rtype_header's root-RIGID codegen path (used
// when a ".meta root:rigid" netlist directive makes the R-type node the
// literal tree root, e.g. tests/bassman_tone_stack/bassman_tone_stack.net --
// see PLAN.md). A root node has adapt_port == -1 (no parent to defer an
// upward wave to), so instead of the usual state-caching incident()/
// reflected() pair, generate_rtype_header must emit a single stateless
// compute(a_in[], b_out[]) that resolves every port in one call. This test
// checks both the emitted code's shape (right function, no leftover
// adapt-port-only artifacts) and that its S matrix (the same numeric values
// compute() reads out of vars->S) still matches the reference N-port
// parallel-adaptor formula already verified in
// test_r_solver_star_3port_no_adapt.
test_r_solver_root_rtype_codegen :: () {
    topo := build_star_3port_topology();
    topo.adapt_port = -1;

    result := solve_rtype_scattering(topo);

    print("=== test_r_solver: root RIGID node codegen (no adapt port) ===\n");

    // Verify the raw S matrix *before* calling generate_rtype_header: its
    // CSE pass rewrites result.S_matrix/result.exprs in place to reference
    // extracted temporaries (_t0, _t1, ...), which eval_expr_numeric_3r
    // doesn't know how to evaluate. Same reference formula as
    // test_r_solver_star_3port_no_adapt.
    R0: float64 = 2.0;
    R1: float64 = 4.7;
    R2: float64 = 9.1;
    G0 := 1.0 / R0; G1 := 1.0 / R1; G2 := 1.0 / R2;
    Gsum := G0 + G1 + G2;
    G := float64.[G0, G1, G2];

    values_ok := true;
    for i : 0..2 {
        for j : 0..2 {
            e := result.S_matrix[i * result.n_ports + j];
            actual := ifx e == NO_EXPR then 0.0 else eval_expr_numeric_3r(e, result.exprs, R0, R1, R2);
            expected := 2.0 * G[j] / Gsum - cast(float64) ifx i == j then 1 else 0;
            d := abs64(actual - expected);
            if d > 1.0e-9 {
                values_ok = false;
                print("  S[%][%] mismatch: got % expected %\n", i, j, actual, expected);
            }
        }
    }
    print("  S-matrix values check ok: %\n", values_ok);

    header := generate_rtype_header("Root3", *result, .["R0", "R1", "R2"]);

    has_compute,    _ := String.contains(header, "static inline void compute(");
    has_state,      _ := String.contains(header, "R_State");
    has_adapt_port, _ := String.contains(header, "adapt_port");
    has_incident,   _ := String.contains(header, "static inline void incident(");
    has_reflected,  _ := String.contains(header, "static inline float reflected(");

    shape_ok := has_compute && !has_state && !has_adapt_port && !has_incident && !has_reflected;
    print("  has compute(): %  (expect true)\n", has_compute);
    print("  has R_State:   %  (expect false)\n", has_state);
    print("  has adapt_port:%  (expect false)\n", has_adapt_port);
    print("  has incident():%  (expect false)\n", has_incident);
    print("  has reflected():%  (expect false)\n", has_reflected);
    print("  codegen shape check ok: %\n", shape_ok);

    print("  root RIGID codegen check ok: %\n\n", shape_ok && values_ok);
}

run_r_solver_tests :: () {
    test_r_solver();
    test_r_solver_adapted();
    test_r_solver_grounded_v2();
    test_r_solver_star_3port_no_adapt();
    test_r_solver_star_3port_adapted();
    test_r_solver_root_rtype_codegen();
}

main :: () {
    run_r_solver_tests();
}

#scope_file
#import "Basic";
#import "r_solver";
#import "sym_algebra";
String :: #import "String";
