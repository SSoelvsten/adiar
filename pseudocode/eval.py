# ---------------------------------------------------------------------------- #
# The OBBD nodes looks as follows
# ---------------------------------------------------------------------------- #
Struct node {
    uid: int,
    label: int,
    low: {
        index: int,
        label: int
    },
    high: {
        index: int,
        label: int
    }
}

# ---------------------------------------------------------------------------- #
# The evaluation function of an OBBD
# ---------------------------------------------------------------------------- #
# The output of the Reduce is of the form
#
# [               ch2         ch1               v3, v2, root  ]
#                  \___________|_________________________/
#
# So we know we have the root at O[O.len - 1], and each node contains the index
# for the where to find its children.
#
# This can be achieved by the following recursive procedure.

fun eval(x, O, v = O.len - 1):
   if v is a sink:
      return v
   else:
      assignment = x[v.label]
      if assignment:
          eval(x, O, v.high)
      else:
          eval(x, O, v.low)

# If you do not want to rely on tail recursion, you can also do the following
fun eval(x, O):
    v = O[O.len - 1]

    while v is not sink:
        assignment = x[node.label]
        if assignment:
            v = O[v.high.index]
        else:
            v = O[v.low.index]

    return v
