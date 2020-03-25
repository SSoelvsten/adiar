# --------------------------------------------------------------------------- #
# A simpler version of the Apply algorithm is the Substitute algorithm.
#
# Sorting by a node: First by v.label, then by v.uid, Top-to-bottom.
# --------------------------------------------------------------------------- #

# An OBBD G and an array of assignment tuples (label, value), where all label
# entries are unique.
def Substitute(G, A):
    # ----------------------------------------------------------------------- #
    # Initialise data structures
    # ----------------------------------------------------------------------- #
    #
    # NOTE: The low and the high only are a dummy-reference and cannot as such
    #       be used for traversing the output. That is, we fix it up during the
    #       reduction....

    Al = [ (l,v) | (l,v) in A ]
    sort (l,v) in Al by l

    # Queue of requests v to be processed when reaching v in L1.
    Q = Empty priority queue: (s) sorted by s

    # ------------------------------------------------------------------------ #
    # Process the root and create initial request(s)
    # ------------------------------------------------------------------------ #
    iL = 0
    v = L[iL]

    iAl = 0
    (al, av) = Al[iAl]

    if al == v.label:
        if av:
            if v.high is not sink:
                Q.insert(v.high)
            else:
                output v.high as minimal OBBD
                return
        else and v.low is not sink:
            if v.low is not sink:
                Q.insert(v.low)
            else:
                output v.low as minimal OBBD
                return

        (al, av) = Al[++iAl]
    else:
        output v
        if v.low is not sink:
            Q.insert(v.low)
        if v.high is not sink:
            Q.insert(v.high)

    # NOTE: Below we jump through the OBBD with the desired ordering. This does
    #       result in quite a few cache-misses, which may be circumvented by
    #       branch-prediction if we were to just scan through the list and see
    #       whether the node was requested.

    while not Q.empty():
        v_request = Q.get_next()

        v = G.V[v_request.index]

        # -------------------------------------------------------------------- #
        # Jump over to-be-substituted variables that we didn't get to see
        # -------------------------------------------------------------------- #
        while al < v.label:
            (al, av) = Al[++iAl]

        # -------------------------------------------------------------------- #
        # Output if not to be substituted
        # -------------------------------------------------------------------- #
        if al == v.label:
            if av and v.high is not sink:
                Q.insert(v.high)
            else and v.low is not sink:
                Q.insert(v.low)
                (al, av) = Al[++iAl]
        else:
            output v
            Q.insert(v.low)
            Q.insert(v.high)
