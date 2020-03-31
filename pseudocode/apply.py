# --------------------------------------------------------------------------- #
# [Arge96] Section 3.5
#
# Sorting by a node: First by v.label, then by v.uid, Top-to-bottom.
# --------------------------------------------------------------------------- #

def Apply(G1, G2, op):
    # ----------------------------------------------------------------------- #
    # Initialise data structures
    # ----------------------------------------------------------------------- #
    # NOTE: These data structures can be populated during the previous Reduce!
    #
    #       [Arge]: If we represent the OBDDs in terms of edges instead of
    #               vertices (where each edge “knows” the level of both source
    #               and sink) [...] The reduce algorithm can then again produce
    #               the blocking used by the (next) apply algorithm.
    #
    #       So the sorting step of L1 and L2 can be skipped if this Apply is
    #       not the initial computation!
    #
    # NOTE: Quite a few times there will be references to the children's
    #       labels. If done naively this would result in having to do an I/O
    #       together with retrieving this data. Remember, that we store the uid
    #       and the label of the children in ALL its parents.
    #
    #       That is, as long as we only retrieve the data of direct children
    #       from L1 and L2 and do not index further into the queues, then it
    #       will cost us no extra I/O's.
    #
    # NOTE: The low and the high only are a dummy-reference and cannot as such
    #       be used for traversing the output. That is, we fix it up during the
    #       reduction....

    L1 = [ v | in G1.V ]
    sort v in L1

    L2 = [ v | in G2.V ]
    sort v in L2

    # [Arge96] Uses 4 priority queues to make reading simpler. I have condensed
    # it into two queues, which I believe will be much simpler in the end.
    #
    # TODO: This should be possible to condense into a single priority queue?
    #
    # NOTE: Most likely a single queue results in fewer cache-misses, since you
    #       do not need to keep the smallest element of all four queues in the
    #       smallest cache.
    #       As long as we make sure L1 always is ahead of L2, then we can merge
    #       these into a single queue still.

    # Queue of requests (v1,v2) to be processed when reaching v1 in L1.
    # ?data is optional data, that may need to be used
    Q1 = Empty priority queue: (v1, v2, ?data) sorted by v1

    # Queue of requests (v1,v2) to be processed when reaching v2 in L2.
    # ?data is optional data, that may need to be used
    Q2 = Empty priority queue: (v1, v2, ?data) sorted by v2

    def forward_request((v1,v2), data?):
        if v1.label < v2.label:
            Q1.insert((v1, v2, data))
        else if v1.label > v2.label:
            Q2.insert((v1, v2, data))
        else 12.label == v2.label:
            # We don't know whether we get to process this at first in L1 or in
            # L2, so we will instead just forward it to both places and skip on
            # whoever gets there late.
            # TODO: Or do we?
            Q1.insert((v1, v2, data))
            Q2.insert((v1, v2, data))

    # ----------------------------------------------------------------------- #
    # Process the roots and create initial requests
    # ----------------------------------------------------------------------- #
    i1, i2 = 0
    v1 = L1[i1]
    v2 = L2[i2]

    # TODO: This logic below has to be abstracted into helper function?
    r_low = compare v1.label with v2.label
             | <    => (v1.low, v2)
             | >    => (v1, v2.low)
             | ==   => (v1.low, v2.low)

    r_high = compare v.label with w.label
             | <    => (v1.high, v2)
             | >    => (v1, v2.high)
             | ==   => (v1.high, v2.high)

    output {
        uid: (v,w),
        label: min(v.label, w.label),
        low: r_low,
        high: r_high,
    }

    forward_requests(r_low, None)
    forward_requests(r_high, None)

    # ----------------------------------------------------------------------- #
    # Process the nodes
    # ----------------------------------------------------------------------- #
    v1 = L1[++i1]
    v2 = L2[++i2]

    while i1 < L1.len or i2 < l2.len:
        # NOTE: In [Arge96] most requests are extracted into a list before
        #       being processed. But, we don't need to do any of this.
        #
        # NOTE: Peeking a queue takes in theory 0 I/O's, since the next element
        #       already is prefetched.

        # ------------------------------------------------------------------- #
        # Forward L1 or L2 to the next request.
        # ------------------------------------------------------------------- #
        # For simplicity, we assume there are no dead nodes in either G1 or G2.
        Q = None
        q1_request = Q1.peek()[0]
        q2_request = Q2.peek()[1]

        if q1_request != v1 and q2_request != v2:
            if i1+1 < L1.len and q1_request == L1[i1+1]:
                v1 = L1[++i1]
                Q = Q1
            else i2+1 < L2.len and q2_request == L2[i2+1]:
                v2 = L2[++i2]
                Q = Q2
        else if q1_request == v1:
            Q = Q1
        else q2_request == v2:
            Q = Q2

        # ------------------------------------------------------------------- #
        # Process next request for either v1 or v2
        # ------------------------------------------------------------------- #
        (v1_request, v2_request, data) = Q.get_next()

        # Skip all same requests and retrieve the data
        while Q.peek()[0] == v1_request and Q.peek()[1] == v2_request:
            (_, _, data) = Q.get_next()

        uid = (v1_request, v2_request)
        label = None
        request_low = None
        request_high = None

        if v1_request.label != v2_request.label:
            if v1 == v1_request:
                label = v1.label
                request_low = (v1.low, v2_request)
                request_high = (v1.high, v2_request)

            else v2 == v2_request:
                label = v2.label
                request_low = (v1_request, v2.low)
                request_high = (v1_request, v2.high)

        else:
            label = v1.label

            if v1 == v1_request and v2 == v2_request:
                request_low = (v1.low, v2.low)
                request_high = (v1.high, v2.high)

            else if data is not None:
                if v2 == v2_request:
                    request_low = (data.low, v2.low)
                    request_high = (data.high, v2.high)

                else v1 == v1_request:
                    request_low = (v1.low, data.low)
                    request_high = (v1.high, data.high)

            else data is None:
                if v2 == v2_request:
                    Q1.insert((v1_request,v2_request v2))
                else v1 == v1_request:
                    Q2.insert((v1_request,v2_request v1))
                continue # Since we do not need to output anything

        # ------------------------------------------------------------------- #
        # Shortcut (sink, sink) requests
        # ------------------------------------------------------------------- #
        if request_low is (sink, sink) and request_high is (sink, sink):
            output {
                uid: uid,
                label: label,
                low: op(request_low[0], request_low[1])
                high: op(request_high[0], request_high[1])
            }

        # ------------------------------------------------------------------- #
        # Output result and forward processing information to later v1s and v2s
        # ------------------------------------------------------------------- #
        else:
            output {
                uid: uid,
                label: label,
                low: request_low
                high: request_high
            }

            forward_request(request_low)
            forward_request(request_high)
