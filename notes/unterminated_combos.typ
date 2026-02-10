#align(center, text(17pt)[*Unterminated Combo Junctions*])

== Ideal Voltage Source in Series with Capacitor

Starting with Kirchoff's Equation, in the Laplace domain:

$
1/2 C s (A_0(s) + B_0(s)) - E(s) = 1/(2 R_0) (A_0(s) - B_0(s))
$

We can apply the bilinear transform:
$
s -> 2/T (1-z^(-1))/(1+z^(-1))
$

And solve fr $b$
$
b[n] = -1/(T + 2 R_0 C) ((T - 2 R_0 C) b[n-1]
- (T - 2 R_0 C) a[n] - (T + 2 R_0 C) a[n-1]
- 2 R_0 T e[n] - 2 R_0 T e[n-1]
) \
= (T - 2 R_0 C)/(T + 2 R_0 C) (a[n] - b[n-1]) + a[n-1]
+ (2 R_0 T)/(T + 2 R_0 C) (e[n] + e[n-1])
\
$
