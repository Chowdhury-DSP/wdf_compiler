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

== Capacitor + Inductor Series

Capacitor:
$
R_p = T / (2 C), b = a[n-1]
$

We'll define port 1 as the capacitor, and port 2 as the inductor.

First, the port impedance:
$
R_p = R_1 + R_2 \
R_p = T/(2 C) + 2 L f_s \
R_i := R_1/R_p = (T/(2 C))/(2 L f_s + (T/(2 C)) \
R_i := R_1/R_p = T/(4 L C f_s + T) \
$

Using the 2-multiply junction:
$
b_0 = -a_1 - a_2 = -a_c[n-1] + a_l[n-1] = a_l[n-1] - a_c[n-1] \
b_1 = a_c = a_c[n-1] - R_i (a_0 - b_0) = a_c[n-1] -R_i (a_0 - a_l[n-1] + a_c[n-1]) \
b_2 = a_l = -a_0 - a_c[n-1] + R_i (a_0 - b_0) = -a_0 - a_c[n-1] - a_c = -a_0 - a_c[n-1] + R_i (a_0 - a_l[n-1] + a_c[n-1]) \
$

== Resistor + Capacitor + Inductor Series

From above:
$
b_(l c) = a_l[n-1] - a_c[n-1] \
a_c = a_c[n-1] - R_(l c) (a_(l c) - b_(l c)) \
a_l = -a_0 - a_c[n-1] + R_(l c) (a_(l c) - b_(l c)) \
$

Now assume we have a series adaptor with port 1 as a resistor and port 2 as the L+C combo.
$
b_0 = -a_1 - a_2 \
b_1 = a_1 - R_1/R_p (a_0 - b_0) \
b_2 = -a_0 - a_1 + R_1/R_p (a_0 - b_0) \
$

Substituting $b_1 = a_r$ and $a_1 = b_r = 0$:
$
b_0 = -a_2 \
b_2 = -a_0 + R_1/R_p (a_0 + a_2) \
$

Substituting $b_2 = a_(l c)$ and $a_2 = b_(l c)$:
$
b_0 = a_c[n-1] - a_l[n-1] \
a_(l c) = -a_0 + R_1/R_p (a_0 + b_0) \
a_c = a_c[n-1] - R_(l c) (a_(l c) + b_0) \
a_l = -a_0 - a_c[n-1] + R_(l c) (a_(l c) + b_0) \
$
