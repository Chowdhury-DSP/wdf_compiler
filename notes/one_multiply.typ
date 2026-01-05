#align(center, text(17pt)[*1-Multiply Junctions*])

== Series Junction

A 3-port WDF Series adaptor:

$
R_p = R_1 + R_2 \
b_0 = -a_1 - a_2 \
b_1 = -R_1/R_p a_0
    + R_2/R_p a_1
    - R_1/R_p a_2 \
b_2 = -R_2/R_p a_0
    - R_2/R_p a_1
    + R_1/R_p a_2
$

We can simplify this into 1-multiply form. First, we
recognize the similarity between the $b_1$ and $b_2$ terms:
$
b_1 + b_2 = -R_1/R_p a_0 - R_2/R_p a_0 \
b_1 + b_2 = -((R_1 + R_2)/R_p a_0)
$

Recall that $R_p = R_1 + R_2$, so:
$
b_1 + b_2 = -((R_1 + R_2)/(R_1 + R_2) a_0) \
b_1 + b_2 = -a_0 \
b_2 = -(a_0 + b_1)
$

So if we can figure out $b_1$ then we can compute $b_2$
with zero multiplications.

$
b_1 = -R_1/R_p a_0 + R_2/R_p a_1 - R_1/R_p a_2 \
b_1 = -R_1/R_p a_0 + R_2/R_p a_1 - R_1/R_p a_2
    + R_1/R_p a_1 - R_1/R_p a_1 \
b_1 = -R_1/R_p (a_0 + a_1 + a_2) + R_2/R_p a_1 + R_1/R_p a_1 \
b_1 = a_1 -R_1/R_p (a_0 + a_1 + a_2)
b_1 = a_1 -R_1/R_p (a_0 - b_0)
$

So the full 1-multiply series adaptor is:
$
R_p = R_1 + R_2 \
b_0 = -a_1 - a_2 \
b_1 = a_1 -R_1/R_p (a_0 - b_0) \
b_2 = -(a_0 + b_1)
$

We have a problem here that the computation
for $b_2$ is dependent on $b_1$. Also, essentially
none of the above can be computed in parallel, since
$b_1$ depends on the result of the multiply, which
in turn depends on the result of $a_0 - b_0$.

---

What happens if we leave $b_1$ alone, but work out
a different way to compute $b_2$:

$
b_2 = -R_2/R_p a_0
    - R_2/R_p a_1
    + R_1/R_p a_2 \
b_2 = -R_2/R_p a_0
    - R_2/R_p a_1
    + R_1/R_p a_2
    + R_2/R_p a_2
    - R_2/R_p a_2 \
b_2 = R_2/R_p (-a_0 - a_1 - a_2)
    + R_1/R_p a_2
    + R_2/R_p a_2 \
b_2 = a_2 + R_2/R_p (-a_0 - a_1 - a_2) \
b_2 = a_2 + R_2/R_p (b_0 - a_0) \
$

That's great, but now we have to store 2 coefficients,
$R_1/R_p$ and $R_2/R_p$. Recall that:
$
R_2/R_p = R_2/R_p + R_1/R_p - R_1/R_p \
R_2/R_p = 1 - R_1/R_p \
$

So we can re-write
$
b_2 = a_2 + R_2/R_p (-a_0 - a_1 - a_2) \
b_2 = a_2 + (1 - R_1/R_p) (-a_0 - a_1 - a_2) \
b_2 = a_2 - a_0 - a_1 - a_2 - R_1/R_p (-a_0 - a_1 - a_2) \
b_2 = -a_0 - a_1 - R_1/R_p (b_0 - a_0) \
b_2 = -a_0 - a_1 + R_1/R_p (a_0 - b_0) \
$

== Resistor-Capacitor Series

$
a_1 -> 0, a_2 -> a_c[n-1] \
R_1/R_p = R/(T/(2 C) + R) = (2 R C) / (T + 2 R C) \
b_0 = -a_1 - a_2 = -a_c[n-1] \
b_1 = -R_1/R_p (a_0 - b_0) = -R_1/R_p (a_0 + a_c[n-1]) \
b_2 = -a_0 - R_1/R_p (b_0 - a_0)
$

== Others
- Parallel junction doesn't stall as badly?
