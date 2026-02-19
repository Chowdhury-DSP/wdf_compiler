#align(center, text(17pt)[*Unterminated Combo Junctions*])

== Ideal Voltage Source in Series with Capacitor

Ideal voltage source:
$
b_s[n] = 2 v_s[n] + a_s[n]
$

Capacitor:
$
b_c[n] = a_c[n-1]
$

Assume we have a series adaptor with Port 0 as the voltage source, and Port 1 as the Capacitor. Then we can define the following: $a_0 = b_s, b_0 = a_s, a_1 = b_c, b_1 = a_c, R_1 = R_c = T/(2 C)$. Basically, wewant to compute $b_2$, given $a_2$. From the series adaptor equations, we know:

$
b_2[n] = R_c/(R_c + R_2) a_2[n] - R_2/(R_c + R_2) (a_0[n] + a_1[n])
$

From the voltage source equation, we can replace $a_0[n] = 2 v_s[n] - b_0[n]$:
$
b_2[n] = R_c/(R_c + R_2) a_2[n] - R_2/(R_c + R_2) (2 v_s[n] - b_0[n] + a_1[n])
$

From the series adaptor equation, recall that $b_0 = -(a_1 + a_2)$. This substitution gives us:
$
b_2[n] = R_c/(R_c + R_2) a_2[n] - R_2/(R_c + R_2) (2 v_s[n] + 2 a_1[n] + a_2[n])
$

Finally, substituting the capacitor equation, we get:
$
b_2[n] = R_c/(R_c + R_2) a_2[n] - R_2/(R_c + R_2) (2 v_s[n] + 2 b_1[n-1] + a_2[n])
$

Thus, we also need to compute $b_1[n]$ as part of the system state. From the series adaptor equations, we have:
$
b_1[n] = -b_2[n] - a_0[n]
$

Again, we can substitute in the voltage source equation for $a_0$:
$
b_1[n] = -b_2[n] - (2 v_s[n] - b_0[n]) \
b_1[n] = -b_2[n] - (2 v_s[n] + a_1[n] + a_2[n])
$

And again substituting in the capacitor equation for $a_1$, we find:
$
b_1[n] = -b_2[n] - (2 v_s[n] + b_1[n-1] + a_2[n])
$

We can simplify the computation by computing the full element as follows:
$
a_t[n] = 2 (v_s[n] + b_1[n-1]) + a_2[n] \
b_2[n] = R_c/(R_c + R_2) a_2[n] - R_2/(R_c + R_2) a_t[n] \
b_1[n] += -b_2[n] - a_t[n] \
$

== Ideal Voltage Source in Series with Capacitor in Parallel with Resistor

From above:
$
a_t[n] = 2 (v_s[n] + z_(cvs)[n-1]) + a_(cvs)[n] \
b_(cvs)[n] = R_c/(R_c + R_2) a_(cvs)[n] - R_2/(R_c + R_2) a_t[n] \
z_(cvs)[n] += -b_(cvs)[n] - a_t[n] \
$

Recall the 3-port parallel adaptor:
$
G_p = G_1 + G_2 \
b_0 = G_1/G_p a_1 + G_2/G_p a_2 \
b_1 = a_0 - G_2/G_p a_1 + G_2/G_p a_2 \
b_2 = a_0 + G_2/G_p a_1 - G_2/G_p a_2
$

Now assume port 0 is the unterminated CVs, and port 1 is a resistor.
Then we can define the following: $a_0 = b_(cvs), b_0 = a_(cvs), a_1 = 0, b_1 = a_(res), G_1 = 1/R$.
Basically, wewant to compute $b_2$, given $a_2$.

Computing input:
$
b_0 = a_(cvs) = G_1/G_p a_1 + G_2/G_p a_2 \
a_(cvs) = G_2/G_p a_2 \
$

$
b_2 = a_0 + G_2/G_p a_1 - G_2/G_p a_2 \
b_2 = b_(cvs) - G_2/G_p a_2 \
b_2 = b_(cvs) - a_(cvs) \
$
