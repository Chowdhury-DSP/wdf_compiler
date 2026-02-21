#align(center, text(17pt)[*Inductor Combos*])

== Resistor + Inductor Series

Resistor:
$
R_p = R, b = 0
$

Inductor:
$
R_p = 2 L f_s, b = -a[n-1]
$

We'll define port 1 as the resistor, and port 2 as the inductor.

First, the port impedance:
$
R_p = R_1 + R_2 \
R_p = 2 L f_s + R \
R_i := R_1/R_p = R/(2 L f_s + R)
$

Recall: $b_1 = a_r$, $a_1 = b_r = 0$, $b_2 = a_l$, $a_2 = b_l$.

Using the 2-multiply junction:
$
b_0 = -a_1 - a_2 = a_l[n-1] \
b_1 = a_r = -R_i (a_0 - b_0) \
b_2 = a_l = -a_0 + R_i (a_0 - b_0) \
b_2 = a_l = -a_0 + R_i (a_0 - a_l[n-1]) \
$

$
b_0 = -b_l, a_l = -(a_0 + a_r) \
b_0 = -a_l[n-1], a_l[n] = a_l[n-1] - R_i (a_0 + a_l[n-1]) \
$

Note that we can also define a 2-multiply form for the state update:
$
a_l[n] = (1 - R_i) a_l[n-1] - R_i a_0
$
