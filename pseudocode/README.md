# Pseudocode
These files provide a somewhat easier to understand depiction of the algorithm
in detail. They are only _python_ files for the syntax highlighting; they not
are supposed to be compilable and possible to run. The algorithms in [Arge96]
are:

- [X] `Reduce`: Given an OBBD (outputted by `Apply` and `Substitute`) creates a
                reduced (i.e. minimal) OBBD.

- [X] `Apply`: Given two OBBDs _O₁, O₂_ and a boolean 2-ary function _f_ create
               a to-be-reduced OBBD that is _f(O₁, O₂)_.

These have then been extended with:

- [X] `Eval`: Given a reduced OBBD, evaluate it given an assignment to _x_

- [X] `Substitute`: Given an OBBD and labels _i₁, i₂, ..._ with values _v₁, v₂,
                    ..._ creates a to-be-reduced OBBD, where a variable with the
                    given labels are substituted for the specific value given.

- [ ] `Relprod`: Given two OBBDs `F` and `G` evaluates whether _∃x : F(x) ∧ G(x)_.
                 Return _x_, if this is the case.

## Notes
From the pseudocode we notice the following things:

- Since the `Apply` and `Substitute` functions manipulate and output the OBBD
  top-to-bottom, then the children references are only to the desired child
  later in the stream. There is no guarantee as to where these children are in
  the output stream. To be able to efficiently evaluate this outputted OBBD, we
  need to reduce it first, thereby fixing up the references.
