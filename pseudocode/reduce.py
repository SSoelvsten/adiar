# --------------------------------------------------------------------------- #
# [Arge96] Section 2.4
#
# Sorting of a node: First by v.label, then by v.uid, Bottom-to-top.
#                    Sinks before nodes.
# --------------------------------------------------------------------------- #

def Reduce(G):
    # ----------------------------------------------------------------------- #
    # Initialise data structures
    # ----------------------------------------------------------------------- #
    # NOTE: These data structures can be populated during an Apply!
    #
    #       [Arge96]: If we represent the OBDDs in terms of edges instead of
    #                 vertices (where each edge “knows” the level of both
    #                 source and sink) and block them in the way they are used
    #                 by the apply algorithm, it can be realized that our apply
    #                 algorithm automatically produces the blocking used by the
    #                 (following) reduce algorithm.
    #
    #       So the sorting step of L1 can be skipped if this Reduce is not the
    #       initial computation!
    #
    #       TODO: Can L2 be populated during the previous Apply algorithm?
    #
    # NOTE: The uid of the output should instead be the index into the output
    #       array, that is the lowest nodes will be 0 and the root will end at
    #       the output size N'.
    #
    #       Followup: Is this blocking ok traversal. But you do traverse it often.

    L1 = [ v | in G.V ]
    sort v in L1

    L2 = [(s, s.low, low), (s, s.high, high) | s in G.V]
    sort (s,t,a) in L2 by t

    Q = Empty priority queue: (s,t,a) sorted by s


    # ----------------------------------------------------------------------- #
    # Process the sinks ?
    # ----------------------------------------------------------------------- #
    # We don't! The label of the children are needed inside a node for
    # I/O efficient sorting of L2, so we might as well also just write
    # down that v.low or v.high goes to a sink!
    #
    # NOTE: This is a common I/O trick to have each node contain all relevant
    #       information of the children, to make certain you know whether it is
    #       a good idea to actually spend I/O on visiting it.

    # ----------------------------------------------------------------------- #
    # Process the nodes level-wise
    # ----------------------------------------------------------------------- #
    i2 = 0
    (s, t, a) = L2[i2]

    # Create requests for level: n
    while t is sink:
        Q.insert((s, t, a))
        (s, t, a) = L2[++i2]

    # Levels: n down to 2 (the root is handled separately)
    i1 = 0
    v = L1[i1]

    for j == n-1 down to 2:
        Lj1 = []
        i2b = i2 # If you do the OPTIONAL below

        # ------------------------------------------------------------------- #
        # Scan the j'th layer in L1 and merge with children's result from Q
        # ------------------------------------------------------------------- #
        while v.label == j:
            (_, t1, a1) = Q.deleteMin()
            (_, t2, a2) = Q.deleteMin()

            # --------------------------------------------------------------- #
            # OPTIONAL: You can apply Reduction Rule 1 early and keep Lj1 smaller.
            #           In the analysis the sorting at each level is the asymptotic
            #           bottle-neck.
            if t1 == t2:
                while t <= v
                   if t == v:
                       Q.insert((s, t1, a))
                   (s, t, a) = L2[++i2]
                v = L1[++i1]
                continue
            # --------------------------------------------------------------- #

            if a1 == low: # i.e. a2 == high
                Lj1.append((v, t1, t2))
            else: # i.e. a2 == low and a1 == high
                Lj1.append((v, t2, t1))
            v = L1[++i1]
        i2 = i2b # If you did the OPTIONAL above

        Sort Lj1 with respect to t1 then t2
        Lj2 = [] # Array of length |lj1| with final node to be forwarded
                 # (old, new), s.t. old |-> new later

        # ------------------------------------------------------------------- #
        # Apply reduction rules on Lj and output result
        # ------------------------------------------------------------------- #
        i3 == 0
        while i3 <= Lj1.len:
            (w, w_low, w_high) = Lj1[i3]

            # Reduction rule 1:
            if w_low.uid == w_high.uid:
                Lj2.append((w, w_low))
                i3++
                continue

            # w definitely has to be outputted, so let us fix up the references
            w_ = { label: w.label, low: w_low, high: w_high }
            Lj2.append((w,w_))

            # NOTE: If we do not care about the ordering within the layer, we
            #       we can already output w here...

            i3++

            # Reduction rule 2:
            while i3 < Lj1.len
                (o, o_low, o_high) = Lj[i3]
                if w_low == o_low and w_high == o_high:
                    Lj2.append((o,w_))
                    i3++
                else:
                    break


        # ------------------------------------------------------------------- #
        # Forward processing information to layers j-1 .. 1
        # ------------------------------------------------------------------- #
        Sort Lj2 with respect to old

        for (old,new) in Lj2:
            if new is made from old:
                output new

            while t == old:
                Q.insert((s, new, a))
                (s, t, a) = L2[++i2]


    # ----------------------------------------------------------------------- #
    # Output the root node
    # ----------------------------------------------------------------------- #
    (_, t1, a1) = Q.deleteMin()
    (_, t2, a2) = Q.deleteMin()
    output v with v.a1 = t1, v.a2 = t2
