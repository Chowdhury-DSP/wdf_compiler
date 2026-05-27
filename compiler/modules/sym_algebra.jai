// sym_algebra -- symbolic computer algebra for Jai
//
// This module can be used to represent, simplify, and solve
// mathematical expressions sybmolically. The module can also
// be used to generate code to evaluate these expressions.
// At the moment only linear expressions are supported.
//
// ===========================================================================
// REPRESENTATION
// ===========================================================================
//
// Expressions are stored in a flat [..] Expr array owned by the caller,
// referenced by Expr_ID (an s64 index into that array).  Passing
// expressions around is cheap; the array grows as needed and IDs
// remain stable. NO_EXPR (-1) is the sentinel for "absent / zero."
//
// Build expressions with:
//   expr_symbol (name,  *exprs)     -- a named unknown
//   expr_const  (value, *exprs)     -- an integer literal
//   expr_add / expr_sub / expr_mul / expr_div
//                                   -- binary arithmetic nodes
//   expr_neg    (a,     *exprs)     -- unary negation
//   substitute  (old, new, *exprs)  -- in-place variable substitution
//
// Note that `expr_symbol` is de-duplicated, i.e. if you call `expr_symbol("x", *exprs)`
// when a symbol called "x" already exists in the `exprs` array, the function
// will just return the ID of the "x" that already exist.
//
// ===========================================================================
// SIMPLIFICATION
// ===========================================================================
//
// This module contains two methods for simplifying expressions.
//
//   fold (e, *exprs)  -- Performs constant folding, and collapses algebraic
//                        identities. For example: 0+x -> x, 1*x -> x, etc.
//
//   simplify (e, *exprs)  -- Performs sybmolic simplification of an expression,
//                            by trying to reduce the number of multiplies and divides,
//                            using common-factor cancellation and similar techniques.
//                            For best results, call after `fold`.
//
// ===========================================================================
// LINEAR SOLVER
// ===========================================================================
//
//   linearize (e, vars, *exprs)  -- Decomposes `e` into a linear combination of the
//                                   provided `vars`, plus a constant term.
//
//   solve :: (equations, vars, exprs) -- Solves a system of equations.
//
// Some notes on the solver:
//   - As of this writing, the largest system of equations that the solver
//     has been tested with had 10 equations and 10 unknowns.
//   - The solver uses Cramer's rule (cofactor / Laplace expansion) with a
//     memoized sub-determinant cache. Previous versions of this code used
//     Gaussian elimination, but the resulting "product of pivots" structure
//     resulted in floating-point underflows when attempting to compute the
//     resulting expressions with 32-bit floats.
//
// ===========================================================================
// CODE-GENERATION PASSES
// ===========================================================================
//
// Before generating code, the following methods may be useful for generating
// optimal code, using common sub-expression elimination and loop-invariant
// code motion. While is possible for the compiler to do these things for us,
// it can be a lot of work (especially for bigger math expressions), and the
// compiler may give up, or so it's nice to give the compiler a head start.
//
// For best results, run these methods in thie order before emitting code:
//
//   cse (gen_vars, *exprs) -> [..] CSE_Var
//       Common Subexpression Elimination -- extracts shared sub-expressions
//       into named temporaries (_t0, _t1, …) to avoid recomputing them.
//
//   sort_cse_topological (*cse_vars)
//       Sort temporaries so each temp is defined before it is used.
//       Run this before the LICM and recip_hoist passes.
//
//   licm (gen_vars, *cse_vars, *exprs, variant_names)
//       Loop-Invariant Code Motion -- identifies which temporaries depend
//       on loop-variant inputs and hoists the rest to an outer scope.
//
//   recip_hoist (gen_vars, *cse_vars, *exprs, variant_names)
//       Reciprocal hoisting -- when an outer-loop temporary T appears only
//       as a divisor in inner-loop expressions, replaces it with 1/T and
//       rewrites x/T as x*(1/T), turning an inner-loop division into a
//       cheaper multiply.
//
// ===========================================================================
// PRINTING
// ===========================================================================
//
//   print_expr (e, exprs) -> string  -- render an expression to a string
//

#import "Basic";

Expr_Kind :: enum {
    NULL;
    CONST;
    SYMBOL;
    NEG; // unary; operand is `left`
    ADD;
    SUB;
    MUL;
    DIV;
}

Expr_ID :: s64;

Expr :: struct {
    kind: Expr_Kind;

    // @TODO: we could union some of this stuff?

    value: s64; // For CONST
    name: string; // For SYMBOL

    // For binary ops
    left: Expr_ID;
    right: Expr_ID;
}

expr_symbol :: (id: Expr_ID, exprs: *[..] Expr) -> Expr_ID { return id; }
expr_symbol :: (name: string, exprs: *[..] Expr) -> Expr_ID {
    // @TODO: Is this slow?
    for expr, id : exprs.* {
        if expr.name == name return id;
    }

    id := exprs.count;
    array_add(exprs, Expr.{ kind = .SYMBOL, name = name });
    return id;
}

expr_neg :: (a: Expr_ID, exprs: *[..] Expr) -> Expr_ID {
    id := exprs.count;
    array_add(exprs, Expr.{ kind = .NEG, left = a });
    return id;
}

expr_neg :: (a: string, exprs: *[..] Expr) -> Expr_ID {
    return expr_neg(expr_symbol(a), exprs);
}

expr_const :: (value: s64, exprs: *[..] Expr) -> Expr_ID {
    id := exprs.count;
    array_add(exprs, Expr.{ kind = .CONST, value = value });
    return id;
}

expr_bin :: (kind: Expr_Kind, a: Expr_ID, b: Expr_ID, exprs: *[..] Expr) -> Expr_ID {
    id := exprs.count;
    array_add(exprs, Expr.{ kind = kind, left = a, right = b });
    return id;
}

expr_add :: (a: $T1, b: $T2, exprs: *[..] Expr) -> Expr_ID { return expr_bin(.ADD, expr_symbol(a, exprs), expr_symbol(b, exprs), exprs); }
expr_sub :: (a: $T1, b: $T2, exprs: *[..] Expr) -> Expr_ID { return expr_bin(.SUB, expr_symbol(a, exprs), expr_symbol(b, exprs), exprs); }
expr_mul :: (a: $T1, b: $T2, exprs: *[..] Expr) -> Expr_ID { return expr_bin(.MUL, expr_symbol(a, exprs), expr_symbol(b, exprs), exprs); }
expr_div :: (a: $T1, b: $T2, exprs: *[..] Expr) -> Expr_ID { return expr_bin(.DIV, expr_symbol(a, exprs), expr_symbol(b, exprs), exprs); }

substitute :: (old_expr: Expr_ID, new_expr: Expr_ID, exprs: *[..] Expr) -> Expr_ID {
    if exprs.*[new_expr].kind == .SYMBOL {
        exprs.*[old_expr] = .{ kind = .NULL };
        for *expr : exprs.* {
            if expr.left == old_expr  expr.left = new_expr;
            if expr.right == old_expr expr.right = new_expr;
        }
        return new_expr;
    }

    exprs.*[old_expr] = exprs.*[new_expr];
    return new_expr;
}

//==================================================

// Sentinel meaning "zero / absent expression"
NO_EXPR : Expr_ID : -1;

// Represents:  sum_i(coeffs[i] * vars[i])  +  constant
// where coeffs is parallel to the `vars` array passed to linearize().
// NO_EXPR in any slot means that term is structurally zero.
Linear :: struct {
    coeffs:   [..] Expr_ID;
    constant: Expr_ID;
}

linear_is_constant :: (l: Linear) -> bool {
    for l.coeffs  if it != NO_EXPR  return false;
    return true;
}

// Combine two optional expression IDs with ADD or SUB, treating NO_EXPR as zero.
// For SUB with `a` absent: produces (0 - b), i.e. negation.
combine :: (a: Expr_ID, b: Expr_ID, op: Expr_Kind, exprs: *[..] Expr) -> Expr_ID {
    if a == NO_EXPR && b == NO_EXPR  return NO_EXPR;
    if a == NO_EXPR {
        if op == .ADD  return b;
        assert(op == .SUB);
        return expr_neg(b, exprs);
    }
    if b == NO_EXPR  return a; // a +/- 0 = a
    return expr_bin(op, a, b, exprs);
}

// Decompose `e` into a linear combination of `vars`.
// Each entry in the returned Linear.coeffs is parallel to `vars`.
// Asserts if the expression is not linear in the given variables.
linearize :: (e: Expr_ID, vars: [] Expr_ID, exprs: *[..] Expr) -> Linear {
    result: Linear;
    for i: 0..vars.count-1  array_add(*result.coeffs, NO_EXPR);
    result.constant = NO_EXPR;

    // Copy the node before recursing: array_add inside recursive calls may
    // reallocate, but Expr is a value type so this local copy stays valid.
    expr := exprs.*[e];

    if expr.kind == {
        case .CONST;
            result.constant = e;

        case .SYMBOL;
            for i: 0..vars.count-1 {
                if vars[i] == e {
                    result.coeffs[i] = expr_const(1, exprs);
                    return result;
                }
            }
            result.constant = e; // a parameter symbol, not one of the vars we care about

        case .NEG;
            la := linearize(expr.left, vars, exprs);
            for i: 0..vars.count-1
                result.coeffs[i] = combine(NO_EXPR, la.coeffs[i], .SUB, exprs);
            result.constant = combine(NO_EXPR, la.constant, .SUB, exprs);

        case .ADD; #through;
        case .SUB;
            la := linearize(expr.left,  vars, exprs);
            lb := linearize(expr.right, vars, exprs);
            for i: 0..vars.count-1
                result.coeffs[i] = combine(la.coeffs[i], lb.coeffs[i], expr.kind, exprs);
            result.constant = combine(la.constant, lb.constant, expr.kind, exprs);

        case .MUL;
            la := linearize(expr.left,  vars, exprs);
            lb := linearize(expr.right, vars, exprs);
            la_const := linear_is_constant(la);
            lb_const := linear_is_constant(lb);
            assert(la_const || lb_const, "Expression is not linear in the given variables!");
            scalar   := ifx la_const then la.constant else lb.constant;
            var_part := ifx la_const then lb           else la;
            if scalar != NO_EXPR {
                for i: 0..vars.count-1 {
                    if var_part.coeffs[i] != NO_EXPR
                        result.coeffs[i] = expr_mul(scalar, var_part.coeffs[i], exprs);
                }
                if var_part.constant != NO_EXPR
                    result.constant = expr_mul(scalar, var_part.constant, exprs);
            }

        case .DIV;
            la := linearize(expr.left,  vars, exprs);
            lb := linearize(expr.right, vars, exprs);
            assert(linear_is_constant(lb), "Expression is not linear! (non-constant divisor)");
            divisor := lb.constant;
            if divisor != NO_EXPR {
                for i: 0..vars.count-1 {
                    if la.coeffs[i] != NO_EXPR
                        result.coeffs[i] = expr_div(la.coeffs[i], divisor, exprs);
                }
                if la.constant != NO_EXPR
                    result.constant = expr_div(la.constant, divisor, exprs);
            }
    }

    return result;
}

//==================================================
// Fold constants and algebraic identities in place, bottom-up.
// Children are fully folded before the parent is checked, so one pass suffices.
fold :: (e: Expr_ID, exprs: *[..] Expr) {
    expr := exprs.*[e];
    if expr.kind == {
        case .CONST;  return;
        case .SYMBOL; return;

        case .NEG;
            fold(expr.left, exprs);
            child := exprs.*[expr.left];
            if child.kind == .CONST {
                exprs.*[e] = .{ kind = .CONST, value = -child.value }; // -C     → CONST(-C)
            } else if child.kind == .NEG {
                exprs.*[e] = exprs.*[child.left];                       // -(-x)  → x
            // NOTE: -(a+b) is intentionally NOT distributed to (-a)+(-b).
            // Keeping it factored enables (-a)/(-b)->a/b and (-a)-b->-(a+b) to fire.
            } else if child.kind == .SUB {
                exprs.*[e] = .{ kind = .SUB, left = child.right, right = child.left }; // -(a-b) → b-a
            }

        case; // ADD, SUB, MUL, DIV
            fold(expr.left,  exprs);
            fold(expr.right, exprs);
            left  := exprs.*[expr.left];
            right := exprs.*[expr.right];

            // Evaluate arithmetic on two literal constants
            if left.kind == .CONST && right.kind == .CONST {
                result: s64;
                if expr.kind == {
                    case .ADD; result = left.value + right.value;
                    case .SUB; result = left.value - right.value;
                    case .MUL; result = left.value * right.value;
                    case .DIV;
                        if right.value != 0 && left.value % right.value == 0
                            result = left.value / right.value;
                        else return; // not exactly divisible; leave as-is
                }
                exprs.*[e] = .{ kind = .CONST, value = result };
                return;
            }

            if expr.kind == .ADD {
                if left.kind  == .CONST && left.value  == 0 { exprs.*[e] = right; return; } // 0 + x → x
                if right.kind == .CONST && right.value == 0 { exprs.*[e] = left;  return; } // x + 0 → x
                // (-a) + (-b) → -(a+b) : factor out shared negation
                if left.kind == .NEG && right.kind == .NEG {
                    inner := expr_bin(.ADD, left.left, right.left, exprs);
                    fold(inner, exprs);
                    exprs.*[e] = .{ kind = .NEG, left = inner };
                    return;
                }
                // a + (-b) → a - b : normalize to SUB form
                if right.kind == .NEG {
                    exprs.*[e] = .{ kind = .SUB, left = expr.left, right = right.left };
                    fold(e, exprs);
                    return;
                }
                // (-a) + b → b - a : normalize so the NEG term is on the right
                if left.kind == .NEG {
                    exprs.*[e] = .{ kind = .SUB, left = expr.right, right = left.left };
                    fold(e, exprs);
                    return;
                }
            }
            if expr.kind == .SUB {
                if right.kind == .CONST && right.value == 0 { exprs.*[e] = left;  return; } // x - 0 → x
                if left.kind  == .CONST && left.value  == 0 { exprs.*[e] = .{ kind = .NEG, left = expr.right }; return; } // 0 - x → -x
                // a - (-b) → a + b : eliminate double sign
                if right.kind == .NEG {
                    exprs.*[e] = .{ kind = .ADD, left = expr.left, right = right.left };
                    fold(e, exprs);
                    return;
                }
                // (-a) - b → -(a+b) : factor out negation from left operand
                if left.kind == .NEG {
                    inner := expr_bin(.ADD, left.left, expr.right, exprs);
                    fold(inner, exprs);
                    exprs.*[e] = .{ kind = .NEG, left = inner };
                    return;
                }
            }
            if expr.kind == .MUL {
                if left.kind  == .CONST && left.value  ==  0 { exprs.*[e] = left;  return; } // 0 * x → 0
                if right.kind == .CONST && right.value ==  0 { exprs.*[e] = right; return; } // x * 0 → 0
                if left.kind  == .CONST && left.value  ==  1 { exprs.*[e] = right; return; } // 1 * x → x
                if right.kind == .CONST && right.value ==  1 { exprs.*[e] = left;  return; } // x * 1 → x
                if left.kind  == .CONST && left.value  == -1 { exprs.*[e] = .{ kind = .NEG, left = expr.right }; return; } // -1 * x → -x
                if right.kind == .CONST && right.value == -1 { exprs.*[e] = .{ kind = .NEG, left = expr.left  }; return; } // x * -1 → -x
                // Pull NEG out of MUL: a*(-b) → -(a*b), (-a)*b → -(a*b)
                // Re-fold afterwards to collapse any resulting NEG(NEG(...))
                if right.kind == .NEG {
                    inner := expr_mul(expr.left, right.left, exprs);
                    fold(inner, exprs);
                    exprs.*[e] = .{ kind = .NEG, left = inner };
                    fold(e, exprs);
                    return;
                }
                if left.kind == .NEG {
                    inner := expr_mul(left.left, expr.right, exprs);
                    fold(inner, exprs);
                    exprs.*[e] = .{ kind = .NEG, left = inner };
                    fold(e, exprs);
                    return;
                }
            }
            if expr.kind == .DIV {
                if right.kind == .CONST && right.value == 1 { exprs.*[e] = left; return; } // x / 1 → x
                // (-a) / (-b) → a / b : cancel double negation (check this before single-NEG cases)
                if left.kind == .NEG && right.kind == .NEG {
                    exprs.*[e] = .{ kind = .DIV, left = left.left, right = right.left };
                    fold(e, exprs);
                    return;
                }
                // (-a) / b → -(a/b) : move negation outside the division
                if left.kind == .NEG {
                    inner := expr_bin(.DIV, left.left, expr.right, exprs);
                    fold(inner, exprs);
                    exprs.*[e] = .{ kind = .NEG, left = inner };
                    return;
                }
                // a / (-b) → -(a/b) : move negation outside the division
                if right.kind == .NEG {
                    inner := expr_bin(.DIV, expr.left, right.left, exprs);
                    fold(inner, exprs);
                    exprs.*[e] = .{ kind = .NEG, left = inner };
                    return;
                }
            }
    }
}

// ===========================================================================
// Cramer's rule solver using memoized cofactor expansion.
//
// Replaces the previous fraction-free Gauss-Jordan implementation.
//
// Motivation: Gauss-Jordan builds each diagonal entry as a product of
// intermediate pivot values.  For typical audio-circuit conductances
// (~1e-3 to 1e-5) an 8×8 system produces pivot products in the range
// 1e-45 to 1e-60 — at or below float32's minimum denormal (~1.4e-45).
// Those denominators underflow to zero in float32, causing NaN/Inf output.
//
// Cramer's rule computes det(A) via cofactor (Laplace) expansion, which
// expresses it as a *sum* of products rather than a *product* of pivots.
// For the same circuit the determinant evaluates to ~6e-16 — well within
// float32's normal range.  All n solutions share the identical det(A)
// denominator, so CSE extracts it once and recip_hoist turns it into a
// single precomputed reciprocal, matching the structure produced by the
// old Mathematica pipeline.
// ===========================================================================

// Count the number of set bits in a 64-bit integer.
popcount_u64 :: (x: u64) -> int {
    n := 0;
    v := x;
    while v != 0 { n += 1; v &= v - 1; }
    return n;
}

// Index of the lowest set bit (0-based).  Precondition: x != 0.
ctz_u64 :: (x: u64) -> int {
    n := 0;
    v := x;
    while (v & 1) == 0 { n += 1; v >>= 1; }
    return n;
}

// Memoization cache entry: the determinant of the sub-matrix identified
// by the given row / column bitmasks within the full n×n matrix.
Det_Cache_Entry :: struct {
    rows : u64;
    cols : u64;
    value: Expr_ID;
}

// Symbolically compute the determinant of the sub-matrix of M (n×n,
// row-major) whose rows and columns are indicated by the bitmasks.
// M[i*n+j] == NO_EXPR is treated as a structural zero (skips that term).
// Results are memoized in `cache` to avoid re-expanding identical minors.
symbolic_det :: (M: [] Expr_ID, n: int,
                 rows: u64, cols: u64,
                 cache: *[..] Det_Cache_Entry,
                 exprs: *[..] Expr) -> Expr_ID {

    // Cache lookup.
    for e: cache.*  if e.rows == rows && e.cols == cols  return e.value;

    k := popcount_u64(rows);
    assert(k == popcount_u64(cols));

    det: Expr_ID;

    if k == 0 {
        // Empty sub-matrix: determinant = 1 (multiplicative identity).
        det = expr_const(1, exprs);

    } else if k == 1 {
        r   := ctz_u64(rows);
        c   := ctz_u64(cols);
        det = M[r * n + c];
        if det == NO_EXPR  det = expr_const(0, exprs);

    } else {
        // Cofactor expansion along the lowest-numbered remaining row.
        r0       := ctz_u64(rows);
        sub_rows := rows ^ (cast(u64)1 << r0);

        det      = NO_EXPR;
        col_pos  := 0;          // position of cj within the ordered cols set
        remaining := cols;
        while remaining != 0 {
            cj        := ctz_u64(remaining);
            remaining &= remaining - 1;   // clear lowest set bit

            entry := M[r0 * n + cj];
            if entry == NO_EXPR { col_pos += 1; continue; }   // structural zero

            sub_cols := cols ^ (cast(u64)1 << cj);
            minor    := symbolic_det(M, n, sub_rows, sub_cols, cache, exprs);

            // Skip if the minor is the constant zero.
            minor_zero := exprs.*[minor].kind == .CONST && exprs.*[minor].value == 0;
            if !minor_zero {
                term := expr_mul(entry, minor, exprs);
                fold(term, exprs);
                sign_plus := (col_pos % 2) == 0;
                det = ifx sign_plus then combine(det, term, .ADD, exprs)
                                    else combine(det, term, .SUB, exprs);
            }
            col_pos += 1;
        }
        if det == NO_EXPR  det = expr_const(0, exprs);
    }

    array_add(cache, .{ rows = rows, cols = cols, value = det });
    return det;
}

// Solve a square linear system A*x = b using Cramer's rule.
// Every solution x[i] = det(A_i) / det(A), where A_i is A with column i
// replaced by b.  Because all solutions share the *same* det(A), the CSE
// pass will extract it as one shared temporary and recip_hoist will turn
// it into a single precomputed reciprocal — identical in structure to the
// single `temp17 = 1/det` produced by the old Mathematica pipeline.
solve :: (equations: [] Expr_ID, solve_for: [] Expr_ID, exprs: *[..] Expr) -> [..] Expr_ID {
    n := equations.count;
    assert(n == solve_for.count, "Need one equation per unknown!");
    assert(n <= 62, "System too large for bitmask Cramer solver (n > 62)");
    print("Solving %1x%1 system\n", n);

    // -----------------------------------------------------------------------
    // Step 1: Extract coefficient matrix A and RHS vector b.
    //   A[i*n + j] = coefficient of solve_for[j] in equation i
    //   b[i]       = -constant term  (so A*x = b)
    // -----------------------------------------------------------------------
    A: [..] Expr_ID;
    b: [..] Expr_ID;

    for i: 0..n-1 {
        lin := linearize(equations[i], solve_for, exprs);
        for j: 0..n-1 {
            a_ij := lin.coeffs[j];
            if a_ij != NO_EXPR  fold(a_ij, exprs);
            array_add(*A, a_ij);
        }
        rhs_i := ifx lin.constant != NO_EXPR then expr_neg(lin.constant, exprs) else NO_EXPR;
        if rhs_i != NO_EXPR  fold(rhs_i, exprs);
        array_add(*b, rhs_i);
    }

    all_mask : u64 = (cast(u64)1 << n) - 1;

    // -----------------------------------------------------------------------
    // Step 2: Compute det(A) — the shared denominator for every solution.
    // Because symbolic_det uses cofactor expansion, det(A) is expressed as
    // a *sum* of products of conductances, keeping values in float32 range.
    // -----------------------------------------------------------------------
    det_A_cache: [..] Det_Cache_Entry;
    det_A := symbolic_det(A, n, all_mask, all_mask, *det_A_cache, exprs);
    assert(!(exprs.*[det_A].kind == .CONST && exprs.*[det_A].value == 0),
           "System matrix is singular!");
    fold(det_A, exprs);

    // -----------------------------------------------------------------------
    // Step 3: For each variable x[i], build A_i and compute det(A_i).
    // We seed the A_i cache with all sub-matrix results from det(A) that do
    // NOT involve column i — those sub-matrices are identical in A and A_i,
    // so the same Expr_IDs can be reused.  This enables the later CSE pass
    // to recognise shared cofactors across all Cramer determinants.
    // -----------------------------------------------------------------------
    result: [..] Expr_ID;

    for i: 0..n-1 {
        // Build A_i: same as A but with column i replaced by b.
        A_i: [..] Expr_ID;
        for row: 0..n-1 {
            for col: 0..n-1 {
                array_add(*A_i, ifx col == i then b[row] else A[row*n + col]);
            }
        }

        // Pre-populate cache with invariant sub-determinants from det(A).
        det_Ai_cache: [..] Det_Cache_Entry;
        col_i_bit := cast(u64)1 << i;
        for e: det_A_cache {
            if (e.cols & col_i_bit) == 0   // sub-matrix doesn't touch col i
                array_add(*det_Ai_cache, e);
        }

        det_Ai := symbolic_det(A_i, n, all_mask, all_mask, *det_Ai_cache, exprs);
        fold(det_Ai, exprs);

        // Numerically zero solution — return directly without a division.
        if exprs.*[det_Ai].kind == .CONST && exprs.*[det_Ai].value == 0 {
            array_add(*result, det_Ai);
            continue;
        }

        sol := expr_div(det_Ai, det_A, exprs);
        fold(sol, exprs);
        array_add(*result, sol);
    }

    return result;
}

//==================================================
// Simplification helpers

// Euclidean GCD.
gcd :: (a: s64, b: s64) -> s64 {
    a = ifx a < 0 then -a else a;
    b = ifx b < 0 then -b else b;
    while b != 0 { t := b;  b = a % b;  a = t; }
    return a;
}

// Deep structural equality: are two sub-trees the same formula?
exprs_equal :: (a: Expr_ID, b: Expr_ID, exprs: [] Expr) -> bool {
    if a == b  return true;
    ea := exprs[a];
    eb := exprs[b];
    if ea.kind != eb.kind  return false;
    if ea.kind == .CONST   return ea.value == eb.value;
    if ea.kind == .SYMBOL  return ea.name == eb.name;
    if ea.kind == .NEG     return exprs_equal(ea.left, eb.left, exprs);
    return exprs_equal(ea.left, eb.left, exprs) && exprs_equal(ea.right, eb.right, exprs);
}

// Rational product form:  (int_num / int_denom) * ∏ sym_num / ∏ sym_denom
// Invariant: int_denom >= 1,  gcd(|int_num|, int_denom) == 1.
Flat :: struct {
    int_num:   s64;
    int_denom: s64;
    sym_num:   [..] Expr_ID;
    sym_denom: [..] Expr_ID;
}

// Decompose a MUL/DIV/NEG/CONST/SYMBOL sub-tree into Flat product form.
// ADD/SUB nodes are treated as opaque atomic symbolic factors.
flatten :: (e: Expr_ID, exprs: [] Expr) -> Flat {
    f: Flat;
    f.int_num   = 1;
    f.int_denom = 1;
    expr := exprs[e];
    if expr.kind == {
        case .NULL;   f.int_num = 0;
        case .CONST;  f.int_num = expr.value;
        case .SYMBOL; array_add(*f.sym_num, e);

        case .NEG;
            inner := flatten(expr.left, exprs);
            f.int_num   = -inner.int_num;
            f.int_denom =  inner.int_denom;
            f.sym_num   =  inner.sym_num;
            f.sym_denom =  inner.sym_denom;

        case .MUL;
            a := flatten(expr.left,  exprs);
            b := flatten(expr.right, exprs);
            f.int_num   = a.int_num   * b.int_num;
            f.int_denom = a.int_denom * b.int_denom;
            for s: a.sym_num   array_add(*f.sym_num,   s);
            for s: b.sym_num   array_add(*f.sym_num,   s);
            for s: a.sym_denom array_add(*f.sym_denom, s);
            for s: b.sym_denom array_add(*f.sym_denom, s);

        case .DIV;
            a := flatten(expr.left,  exprs);
            b := flatten(expr.right, exprs);
            // (a.n/a.d) / (b.n/b.d)  =  (a.n * b.d) / (a.d * b.n)
            f.int_num   = a.int_num   * b.int_denom;
            f.int_denom = a.int_denom * b.int_num;
            for s: a.sym_num   array_add(*f.sym_num,   s);
            for s: b.sym_denom array_add(*f.sym_num,   s);
            for s: a.sym_denom array_add(*f.sym_denom, s);
            for s: b.sym_num   array_add(*f.sym_denom, s);

        case;  // ADD, SUB: treat as one opaque symbolic factor
            array_add(*f.sym_num, e);
    }
    // Normalize: keep int_denom > 0 and reduce by GCD.
    if f.int_denom < 0 { f.int_num = -f.int_num;  f.int_denom = -f.int_denom; }
    g := gcd(ifx f.int_num < 0 then -f.int_num else f.int_num, f.int_denom);
    if g > 1 { f.int_num /= g;  f.int_denom /= g; }
    return f;
}

// Left-associative product:  coeff * syms[0] * syms[1] * ...
// Returns expr_const(1, exprs) when coeff == 1 and syms is empty.
build_product :: (coeff: s64, syms: [] Expr_ID, exprs: *[..] Expr) -> Expr_ID {
    result := NO_EXPR;
    if coeff != 1 || syms.count == 0  result = expr_const(coeff, exprs);
    for s: syms {
        if result == NO_EXPR  result = s;
        else                  result = expr_bin(.MUL, result, s, exprs);
    }
    if result == NO_EXPR  result = expr_const(1, exprs);
    return result;
}

// Reconstruct an expression from a Flat.  Caller should fold() afterwards if needed.
rebuild_flat :: (f: Flat, exprs: *[..] Expr) -> Expr_ID {
    if f.int_num == 0  return expr_const(0, exprs);
    negative  := f.int_num < 0;
    abs_coeff := ifx negative then -f.int_num else f.int_num;
    num_expr  := build_product(abs_coeff, f.sym_num, exprs);
    result: Expr_ID;
    if f.int_denom == 1 && f.sym_denom.count == 0 {
        result = num_expr;
    } else {
        denom_expr := build_product(f.int_denom, f.sym_denom, exprs);
        result = expr_bin(.DIV, num_expr, denom_expr, exprs);
    }
    if negative  result = expr_neg(result, exprs);
    return result;
}

// Try to exactly divide expression `num` by `factor`.
//
// For ADD / SUB / NEG nodes: recurses and requires every branch to succeed.
// For leaf / product nodes:  uses Flat arithmetic to verify exact divisibility.
//
// Returns (true, result_id) on success, (false, NO_EXPR) on failure.
// On failure the expression array may contain a few unreachable scratch nodes;
// those are harmless and will not be printed.
try_cancel :: (num: Expr_ID, factor: Expr_ID, exprs: *[..] Expr) -> (bool, Expr_ID) {
    expr := exprs.*[num];

    if expr.kind == .NEG {
        ok, inner := try_cancel(expr.left, factor, exprs);
        if ok  return true, expr_neg(inner, exprs);
        return false, NO_EXPR;
    }

    if expr.kind == .ADD || expr.kind == .SUB {
        ok_l, res_l := try_cancel(expr.left,  factor, exprs);
        if !ok_l  return false, NO_EXPR;
        ok_r, res_r := try_cancel(expr.right, factor, exprs);
        if !ok_r  return false, NO_EXPR;
        return true, expr_bin(expr.kind, res_l, res_r, exprs);
    }

    // Leaf / product node: use Flat arithmetic.
    nf := flatten(num,    exprs.*);
    df := flatten(factor, exprs.*);

    // Integer part: (nf.n / nf.d) / (df.n / df.d) = (nf.n * df.d) / (nf.d * df.n).
    // Must reduce to an exact integer (reduced denominator == 1).
    new_n := nf.int_num   * df.int_denom;
    new_d := nf.int_denom * df.int_num;
    if new_d == 0  return false, NO_EXPR;
    if new_d < 0 { new_n = -new_n;  new_d = -new_d; }
    {
        g := gcd(ifx new_n < 0 then -new_n else new_n, new_d);
        if g > 0 { new_n /= g;  new_d /= g; }
    }
    if new_d != 1  return false, NO_EXPR;  // not exact

    // Symbolic part:
    //   combined_n = nf.sym_num   ∪ df.sym_denom
    //   combined_d = nf.sym_denom ∪ df.sym_num
    // Cross-cancel matching pairs (one entry from combined_d with one from combined_n).
    // Success condition: every factor that came from df.sym_num must be cancelled.
    combined_n: [..] Expr_ID;
    combined_d: [..] Expr_ID;
    for s: nf.sym_num    array_add(*combined_n, s);
    for s: df.sym_denom  array_add(*combined_n, s);
    for s: nf.sym_denom  array_add(*combined_d, s);
    for s: df.sym_num    array_add(*combined_d, s);

    used_n: [..] bool;
    used_d: [..] bool;
    for _: 0..combined_n.count-1  array_add(*used_n, false);
    for _: 0..combined_d.count-1  array_add(*used_d, false);

    for i: 0..combined_d.count-1 {
        if used_d[i]  continue;
        for j: 0..combined_n.count-1 {
            if used_n[j]  continue;
            if exprs_equal(combined_d[i], combined_n[j], exprs.*) {
                used_d[i] = true;
                used_n[j] = true;
                break;
            }
        }
    }

    // Every factor from df.sym_num (the last df.sym_num.count entries of combined_d)
    // must have been cancelled.
    for i: nf.sym_denom.count .. combined_d.count - 1 {
        if !used_d[i]  return false, NO_EXPR;
    }

    result_f: Flat;
    result_f.int_num   = new_n;
    result_f.int_denom = 1;
    for i: 0..combined_n.count-1  if !used_n[i]  array_add(*result_f.sym_num,   combined_n[i]);
    for i: 0..combined_d.count-1  if !used_d[i]  array_add(*result_f.sym_denom, combined_d[i]);
    return true, rebuild_flat(result_f, exprs);
}

// Simplify a DIV node in-place by cancelling common symbolic and integer factors
// between numerator and denominator.
simplify_div :: (e: Expr_ID, exprs: *[..] Expr) {
    expr := exprs.*[e];
    num  := expr.left;
    den  := expr.right;

    // a / a  →  1
    if exprs_equal(num, den, exprs.*) {
        exprs.*[e] = .{ kind = .CONST, value = 1 };
        return;
    }

    den_flat := flatten(den, exprs.*);
    if den_flat.int_num == 0  return;  // degenerate denominator; leave as-is

    cur_num := num;

    // Cancel symbolic denominator factors from the numerator one-by-one.
    remaining_sym: [..] Expr_ID;
    for sym: den_flat.sym_num {
        ok, new_num := try_cancel(cur_num, sym, exprs);
        if ok  cur_num = new_num;
        else   array_add(*remaining_sym, sym);
    }

    // Try to cancel the integer coefficient of the denominator (exact division).
    remaining_int := den_flat.int_num;  // > 0 after fold normalises sign
    if den_flat.int_denom == 1 && remaining_int > 1 {
        int_factor := expr_const(remaining_int, exprs);
        ok, new_num := try_cancel(cur_num, int_factor, exprs);
        if ok { cur_num = new_num;  remaining_int = 1; }
    }

    // Partial integer GCD: if the numerator is a plain CONST, reduce by GCD.
    if den_flat.int_denom == 1 && remaining_int > 1 {
        cur_expr := exprs.*[cur_num];
        if cur_expr.kind == .CONST && cur_expr.value != 0 {
            abs_val := ifx cur_expr.value < 0 then -cur_expr.value else cur_expr.value;
            g := gcd(abs_val, remaining_int);
            if g > 1 {
                cur_num       = expr_const(cur_expr.value / g, exprs);
                remaining_int /= g;
            }
        }
    }

    // If the denominator itself had a denominator (den_flat.sym_denom / den_flat.int_denom),
    // those factors flip to the numerator:  num / (A/B)  =  num * B / A.
    for sym: den_flat.sym_denom {
        cur_num = expr_bin(.MUL, cur_num, sym, exprs);
    }
    if den_flat.int_denom != 1 {
        cur_num = expr_bin(.MUL, cur_num, expr_const(den_flat.int_denom, exprs), exprs);
    }

    // Has the denominator fully cancelled?
    if remaining_sym.count == 0 && remaining_int == 1 {
        exprs.*[e] = exprs.*[cur_num];
        fold(e, exprs);
        return;
    }

    new_den := build_product(remaining_int, remaining_sym, exprs);
    exprs.*[e] = .{ kind = .DIV, left = cur_num, right = new_den };
    fold(e, exprs);
}

// If a MUL has a DIV child, lift it into a single top-level DIV so that
// simplify_div can see the full numerator and denominator.
//   (a/b) * c  →  (a*c) / b
//    a * (b/c) →  (a*b) / c
simplify_mul :: (e: Expr_ID, exprs: *[..] Expr) {
    expr  := exprs.*[e];
    left  := exprs.*[expr.left];
    right := exprs.*[expr.right];
    if left.kind == .DIV {
        new_num := expr_bin(.MUL, left.left, expr.right, exprs);
        exprs.*[e] = .{ kind = .DIV, left = new_num, right = left.right };
        simplify_div(e, exprs);
    } else if right.kind == .DIV {
        new_num := expr_bin(.MUL, expr.left, right.left, exprs);
        exprs.*[e] = .{ kind = .DIV, left = new_num, right = right.right };
        simplify_div(e, exprs);
    }
}

// Recursive bottom-up pass: simplify children first, then the current node.
simplify_node :: (e: Expr_ID, exprs: *[..] Expr) {
    expr := exprs.*[e];
    if expr.kind == .CONST || expr.kind == .SYMBOL || expr.kind == .NULL  return;
    if expr.kind == .NEG {
        simplify_node(expr.left, exprs);
    } else {
        simplify_node(expr.left,  exprs);
        simplify_node(expr.right, exprs);
    }
    expr = exprs.*[e];  // re-read: a child may have changed its own kind
    if expr.kind == .DIV  simplify_div(e, exprs);
    if expr.kind == .MUL  simplify_mul(e, exprs);
}

//==================================================
simplify :: (e: Expr_ID, exprs: *[..] Expr) -> Expr_ID {
    fold(e, exprs);             // constant-fold and normalise signs
    simplify_node(e, exprs);    // cancel common factors (MUL/DIV lifting)
    fold(e, exprs);             // clean up any new constant sub-expressions
    simplify_node(e, exprs);    // second pass: catches opportunities exposed above
    fold(e, exprs);
    return e;
}


//==================================================
print_expr :: (e: Expr_ID, exprs: [] Expr) -> string {
    expr_to_builder :: (builder: *String_Builder, e: Expr_ID, exprs: [] Expr) {
        if exprs[e].kind == {
            case .CONST;
                print_to_builder(builder, "%", exprs[e].value);

            case .SYMBOL;
                append(builder, exprs[e].name);

            case .NEG;
                append(builder, "(-");
                expr_to_builder(builder, exprs[e].left, exprs);
                append(builder, ")");

            case;
                append(builder, "(");
                expr_to_builder(builder, exprs[e].left, exprs);
                if exprs[e].kind == {
                    case .ADD; append(builder, " + ");
                    case .SUB; append(builder, " - ");
                    case .MUL; append(builder, " * ");
                    case .DIV; append(builder, " / ");
                }
                expr_to_builder(builder, exprs[e].right, exprs);
                append(builder, ")");
        }
    }

    builder: String_Builder;
    expr_to_builder(*builder, e, exprs);
    return builder_to_string(*builder);
}

//==================================================
CSE_Var :: struct {
    name:    string;
    expr_id: Expr_ID;
}

cse_expr_size :: (e: Expr_ID, exprs: [] Expr) -> int {
    ex := exprs[e];
    if ex.kind == .CONST || ex.kind == .SYMBOL || ex.kind == .NULL  return 0;
    if ex.kind == .NEG  return 1 + cse_expr_size(ex.left, exprs);
    return 1 + cse_expr_size(ex.left, exprs) + cse_expr_size(ex.right, exprs);
}

cse_count_refs :: (e: Expr_ID, exprs: [] Expr,
                   ids: *[..] Expr_ID, counts: *[..] int) {
    ex := exprs[e];
    if ex.kind == .CONST || ex.kind == .SYMBOL || ex.kind == .NULL  return;
    for i: 0..ids.count-1 {
        if ids.*[i] == e {
            counts.*[i] += 1;
            return;
        }
    }
    array_add(ids,    e);
    array_add(counts, 1);
    if ex.kind == .NEG {
        cse_count_refs(ex.left, exprs, ids, counts);
    } else {
        cse_count_refs(ex.left,  exprs, ids, counts);
        cse_count_refs(ex.right, exprs, ids, counts);
    }
}

cse_pass :: (gen_var_ids: [] Expr_ID, cse_vars: *[..] CSE_Var,
             exprs: *[..] Expr, counter: *int) -> bool {
    ids:    [..] Expr_ID;
    counts: [..] int;
    for v  : gen_var_ids  cse_count_refs(v,          exprs.*, *ids, *counts);
    for cv : cse_vars.*   cse_count_refs(cv.expr_id, exprs.*, *ids, *counts);
    if ids.count == 0  return false;

    // Union-find: merge structurally-equal nodes into one equivalence class.
    parent: [..] int;
    for i: 0..ids.count-1  array_add(*parent, i);
    for i: 0..ids.count-1 {
        if parent[i] != i  continue;
        for j: i+1..ids.count-1 {
            if parent[j] != j    continue;
            if ids[i] == ids[j]  continue;
            if exprs_equal(ids[i], ids[j], exprs.*) {
                parent[j]   = i;
                counts[i]  += counts[j];
            }
        }
    }

    made_progress := false;
    for i: 0..ids.count-1 {
        if parent[i] != i                        continue;  // not the class representative
        if counts[i] < 2                         continue;  // not shared enough to CSE
        if cse_expr_size(ids[i], exprs.*) == 0   continue;  // trivial (leaf) node

        temp_name := tprint("_t%", counter.*);
        counter.* += 1;

        ex       := exprs.*[ids[i]];
        saved_id := ifx ex.kind == .NEG
                    then expr_neg(ex.left, exprs)
                    else expr_bin(ex.kind, ex.left, ex.right, exprs);
        array_add(cse_vars, .{ name = temp_name, expr_id = saved_id });

        temp_sym_id := expr_symbol(temp_name, exprs);
        exprs.*[ids[i]] = exprs.*[temp_sym_id];
        for j: 0..ids.count-1 {
            if parent[j] == i && j != i
                exprs.*[ids[j]] = exprs.*[temp_sym_id];
        }
        made_progress = true;
    }
    return made_progress;
}

expr_is_loop_variant :: (e: Expr_ID, exprs: [] Expr,
                         base_variant_names: [] string,
                         cse_variant_names:  [] string) -> bool {
    ex := exprs[e];
    if ex.kind == .CONST || ex.kind == .NULL  return false;
    if ex.kind == .SYMBOL {
        for n : base_variant_names  if ex.name == n  return true;
        for n : cse_variant_names   if ex.name == n  return true;
        return false;
    }
    if ex.kind == .NEG {
        return expr_is_loop_variant(ex.left, exprs, base_variant_names, cse_variant_names);
    }
    return expr_is_loop_variant(ex.left,  exprs, base_variant_names, cse_variant_names)
        || expr_is_loop_variant(ex.right, exprs, base_variant_names, cse_variant_names);
}

cse :: (gen_var_ids: [] Expr_ID, exprs: *[..] Expr) -> [..] CSE_Var {
    cse_vars: [..] CSE_Var;
    counter: int = 0;
    MAX_PASSES :: 20;
    for 0..MAX_PASSES-1 {
        if !cse_pass(gen_var_ids, *cse_vars, exprs, *counter)  break;
    }
    return cse_vars;
}


// Sort cse_vars in-place into topological order: each temp appears after all
// other temps whose names it references.  A single forward pass over cse_vars
// is then sufficient to propagate loop-variant-ness and to emit declarations
// in the correct dependency order.
sort_cse_topological :: (cse_vars: *[..] CSE_Var, exprs: [] Expr) {
    sorted:  [..] CSE_Var;
    emitted: [..] bool;
    for _ : 0..cse_vars.count-1  array_add(*emitted, false);

    remaining := cse_vars.count;
    while remaining > 0 {
        made_progress := false;
        for i : 0..cse_vars.count-1 {
            if emitted[i]  continue;
            deps: [..] string;
            collect_symbol_names(cse_vars.*[i].expr_id, exprs, *deps);
            ready := true;
            for dep : deps {
                for j : 0..cse_vars.count-1 {
                    if j != i && cse_vars.*[j].name == dep && !emitted[j] {
                        ready = false;
                        break;
                    }
                }
                if !ready  break;
            }
            if ready {
                emitted[i] = true;
                remaining -= 1;
                made_progress = true;
                array_add(*sorted, cse_vars.*[i]);
            }
        }
        if !made_progress  break;  // Cycle guard.
    }
    cse_vars.* = sorted;
}
// Walk a loop-variant expression and hoist any loop-invariant sub-expressions
// into new_vars as new CSE temporaries (even if they appear only once).
licm_walk :: (e: Expr_ID, exprs: *[..] Expr,
              new_vars: *[..] CSE_Var, counter: *int,
              variant_base: [] string, variant_cse_names: [] string) -> bool {
    ex := (exprs.*)[e];
    if ex.kind == .CONST || ex.kind == .SYMBOL || ex.kind == .NULL  return false;

    made_progress := false;
    n_children := ifx ex.kind == .NEG then 1 else 2;
    for ci : 0..n_children-1 {
        child_id := ifx ci == 0 then ex.left else ex.right;
        ch := (exprs.*)[child_id];
        if ch.kind == .CONST || ch.kind == .SYMBOL || ch.kind == .NULL  continue;
        if !expr_is_loop_variant(child_id, exprs.*, variant_base, variant_cse_names) {
            // Loop-invariant compound sub-expression inside a loop-variant context -- hoist it.
            temp_name := tprint("_t%", counter.*);  counter.* += 1;
            saved_id  := ifx ch.kind == .NEG
                         then expr_neg(ch.left, exprs)
                         else expr_bin(ch.kind, ch.left, ch.right, exprs);
            array_add(new_vars, .{ name = temp_name, expr_id = saved_id });
            (exprs.*)[child_id] = (exprs.*)[expr_symbol(temp_name, exprs)];
            made_progress = true;
        } else if licm_walk(child_id, exprs, new_vars, counter, variant_base, variant_cse_names) {
            made_progress = true;
        }
    }

    return made_progress;
}

// Loop-Invariant Code Motion pass: extract loop-invariant sub-expressions from
// loop-variant expressions, even when they appear only once.  Newly extracted
// temporaries are appended to cse_vars; codegen will hoist them outside the
// per-sample loop because they classify as loop-invariant.
licm :: (gen_var_ids: [] Expr_ID, cse_vars: *[..] CSE_Var, exprs: *[..] Expr,
         variant_base: [] string) {
    // Propagate variant-ness through the existing (topologically ordered) CSE temps
    // so that expr_is_loop_variant correctly handles temps that depend on other temps.
    variant_cse_names: [..] string;
    for cv : cse_vars.* {
        if expr_is_loop_variant(cv.expr_id, exprs.*, variant_base, variant_cse_names)
            array_add(*variant_cse_names, cv.name);
    }

    new_vars: [..] CSE_Var;
    counter := cse_vars.count;   // Name LICM temps after existing CSE temps to avoid clashes.

    MAX_PASSES :: 10;
    for 0..MAX_PASSES-1 {
        made_progress := false;

        for gv_id : gen_var_ids {
            if expr_is_loop_variant(gv_id, exprs.*, variant_base, variant_cse_names) {
                if licm_walk(gv_id, exprs, *new_vars, *counter, variant_base, variant_cse_names)
                    made_progress = true;
            }
        }

        for cv : cse_vars.* {
            if expr_is_loop_variant(cv.expr_id, exprs.*, variant_base, variant_cse_names) {
                if licm_walk(cv.expr_id, exprs, *new_vars, *counter, variant_base, variant_cse_names)
                    made_progress = true;
            }
        }

        if !made_progress  break;
    }

    // Append newly extracted loop-invariant temporaries to cse_vars.
    for nv : new_vars  array_add(cse_vars, nv);
}


// Collect all SYMBOL names referenced anywhere in expression e (recursively).
collect_symbol_names :: (e: Expr_ID, exprs: [] Expr, names: *[..] string) {
    ex := exprs[e];
    if ex.kind == .SYMBOL {
        array_add_if_unique(names, ex.name);
        return;
    }
    if ex.kind == .CONST || ex.kind == .NULL  return;
    if ex.kind == .NEG {
        collect_symbol_names(ex.left, exprs, names);
        return;
    }
    collect_symbol_names(ex.left,  exprs, names);
    collect_symbol_names(ex.right, exprs, names);
}


// Count how many times a named symbol appears as a *direct* divisor
// (the immediate right child of a DIV node) vs. all other positions,
// in the expression tree rooted at e.
count_sym_uses_by_name :: (e: Expr_ID, sym_name: string, exprs: [] Expr,
                           div_count: *int, other_count: *int) {
    ex := exprs[e];
    if ex.kind == .CONST || ex.kind == .NULL  return;
    if ex.kind == .SYMBOL {
        if ex.name == sym_name  other_count.* += 1;
        return;
    }
    if ex.kind == .NEG {
        count_sym_uses_by_name(ex.left, sym_name, exprs, div_count, other_count);
        return;
    }
    if ex.kind == .DIV {
        // Treat the right (divisor) child specially.
        right_ex := exprs[ex.right];
        if right_ex.kind == .SYMBOL && right_ex.name == sym_name {
            div_count.* += 1;
        } else {
            count_sym_uses_by_name(ex.right, sym_name, exprs, div_count, other_count);
        }
        count_sym_uses_by_name(ex.left, sym_name, exprs, div_count, other_count);
        return;
    }
    count_sym_uses_by_name(ex.left,  sym_name, exprs, div_count, other_count);
    count_sym_uses_by_name(ex.right, sym_name, exprs, div_count, other_count);
}

// In expression tree e, replace every DIV(A, sym_name) with MUL(A, recip_sym_id).
replace_div_by_name :: (e: Expr_ID, sym_name: string, recip_sym_id: Expr_ID, exprs: *[..] Expr) {
    ex := (exprs.*)[e];
    if ex.kind == .CONST || ex.kind == .SYMBOL || ex.kind == .NULL  return;
    if ex.kind == .NEG {
        replace_div_by_name(ex.left, sym_name, recip_sym_id, exprs);
        return;
    }
    if ex.kind == .DIV {
        right_ex := (exprs.*)[ex.right];
        if right_ex.kind == .SYMBOL && right_ex.name == sym_name {
            (exprs.*)[e].kind  = .MUL;
            (exprs.*)[e].right = recip_sym_id;
            replace_div_by_name(ex.left, sym_name, recip_sym_id, exprs);
        } else {
            replace_div_by_name(ex.left,  sym_name, recip_sym_id, exprs);
            replace_div_by_name(ex.right, sym_name, recip_sym_id, exprs);
        }
        return;
    }
    replace_div_by_name(ex.left,  sym_name, recip_sym_id, exprs);
    replace_div_by_name(ex.right, sym_name, recip_sym_id, exprs);
}

// Pre-compute reciprocals of loop-invariant expressions that appear as divisors
// in the inner sample loop, so every inner-loop divide becomes a multiply.
//
// For each outer-loop CSE temp _tN used as a direct divisor (x / _tN):
//   - If _tN is used ONLY as a divisor in the inner loop AND is not referenced by
//     any other outer-loop expression: flip _tN in place to (1 / original) and
//     replace every inner-loop (x / _tN) with (x * _tN).
//   - Otherwise: add a new outer-loop temp (1 / _tN) and replace inner-loop
//     (x / _tN) with (x * new_temp).
recip_hoist :: (gen_var_ids: [] Expr_ID, cse_vars: *[..] CSE_Var, exprs: *[..] Expr,
                variant_base: [] string) {
    // Classify each CSE temp as inner (loop-variant) or outer (loop-invariant).
    variant_cse_names: [..] string;
    for cv : cse_vars.* {
        if expr_is_loop_variant(cv.expr_id, exprs.*, variant_base, variant_cse_names)
            array_add(*variant_cse_names, cv.name);
    }

    // Build a usage-info record for every outer-loop CSE temp.
    Div_Info :: struct {
        name:        string;
        cse_idx:     int;   // Position in cse_vars (for in-place flip).
        inner_div:   int;   // Times used as direct divisor in inner-loop expressions.
        inner_other: int;   // Times used in non-divisor position in inner-loop expressions.
        outer_refs:  int;   // Times referenced (any position) by other outer-loop expressions.
    }
    infos: [..] Div_Info;
    for cv, idx : cse_vars.* {
        if !array_find(variant_cse_names, cv.name)
            array_add(*infos, .{ name = cv.name, cse_idx = idx });
    }
    if infos.count == 0  return;

    // Count inner-loop uses.
    for i : 0..infos.count-1 {
        for gv_id : gen_var_ids
            count_sym_uses_by_name(gv_id, infos[i].name, exprs.*, *infos[i].inner_div, *infos[i].inner_other);
        for cv : cse_vars.* {
            if array_find(variant_cse_names, cv.name)
                count_sym_uses_by_name(cv.expr_id, infos[i].name, exprs.*, *infos[i].inner_div, *infos[i].inner_other);
        }
    }

    // Count references in *other* outer-loop CSE expressions (guards the in-place flip).
    for i : 0..infos.count-1 {
        for cv : cse_vars.* {
            if cv.name == infos[i].name  continue;
            if !array_find(variant_cse_names, cv.name) {
                d, o := 0, 0;
                count_sym_uses_by_name(cv.expr_id, infos[i].name, exprs.*, *d, *o);
                infos[i].outer_refs += d + o;
            }
        }
    }

    counter := cse_vars.count;

    for info : infos {
        if info.inner_div == 0  continue;

        sym_id := expr_symbol(info.name, exprs);

        if info.inner_other == 0 && info.outer_refs == 0 {
            // Safe to flip: temp is used only as a divisor and nothing else depends on it.
            one_id := expr_const(1, exprs);
            cse_vars.*[info.cse_idx].expr_id =
                expr_bin(.DIV, one_id, cse_vars.*[info.cse_idx].expr_id, exprs);
            for gv_id : gen_var_ids  replace_div_by_name(gv_id, info.name, sym_id, exprs);
            for cv : cse_vars.* {
                if array_find(variant_cse_names, cv.name)
                    replace_div_by_name(cv.expr_id, info.name, sym_id, exprs);
            }
        } else {
            // Temp is used elsewhere too; add a dedicated reciprocal.
            recip_name := tprint("_t%", counter);
            counter += 1;
            one_id := expr_const(1, exprs);
            recip_expr_id := expr_bin(.DIV, one_id, sym_id, exprs);
            array_add(cse_vars, .{ name = recip_name, expr_id = recip_expr_id });

            recip_sym_id := expr_symbol(recip_name, exprs);
            for gv_id : gen_var_ids  replace_div_by_name(gv_id, info.name, recip_sym_id, exprs);
            for cv : cse_vars.* {
                if array_find(variant_cse_names, cv.name)
                    replace_div_by_name(cv.expr_id, info.name, recip_sym_id, exprs);
            }
        }
    }
}
