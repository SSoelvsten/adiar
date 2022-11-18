#ifndef ADIAR_INTERNAL_DATA_TYPES_REQUEST_H
#define ADIAR_INTERNAL_DATA_TYPES_REQUEST_H

#include<adiar/internal/data_types/node.h>
#include<adiar/internal/data_types/tuple.h>

namespace adiar
{
  // TODO (template): node type, with_source

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Request struct for the top-down sweep time-forwarding algorithms.
  ///
  /// \tparam cardinality   The cardinality of the algorithm, e.g. it is 1 for
  ///                       `bdd_restrict`, 2 for `bdd_apply` and 3 for
  ///                       `bdd_ite`.
  ///
  /// \tparam nodes_carried Number of children being forwarded with the request.
  ///                       This is used when `cardinality` is greater than 1
  ///                       and a per-level priority queue forwards the children
  ///                       of `target.fst()` to `target.snd()` and so on.
  //////////////////////////////////////////////////////////////////////////////
  template<uint8_t cardinality = 1, uint8_t nodes_carried = 0>
  class request
  {
  public:
    static_assert(0 < cardinality,
                  "Request type is not designed for 0-ary algorithms.");

    static_assert(nodes_carried < cardinality,
                  "'nodes_carried' ought not to hold more than the 'cardinality' of the algorithm");

    typedef node::label_t label_t;

    typedef node::ptr_t ptr_t;

    typedef tuple<ptr_t, cardinality> target_t;

    /* ============================== VARIABLES ============================= */
  public:
    target_t target;

    node::children_t node_carry[nodes_carried];

    /* ============================ CONSTRUCTORS ============================ */
  public:
    // Provide 'default' constructors to ensure it being a 'POD' inside of TPIE.
    request() = default;
    request(const request &r) = default;
    ~request() = default;

    /* ======================== DERIVATED INFORMATION ======================= */
    request(const target_t &t,
            const node::children_t (& nc) [nodes_carried])
      : target(t), node_carry(nc)
    { }
  };

  //////////////////////////////////////////////////////////////////////////////
  // Priority queue functions
  template<class request_t>
  struct request_label
  {
    static inline typename request_t::label_t
    label_of(const request_t &r)
    {
      return r.target.fst().label();
    }
  };

  template<class request_t>
  struct request_fst_lt
  {
    // TODO (DEBUG): sort by source, if it exists
    inline bool operator()(const request_t &a, const request_t &b)
    {
      return tuple_fst_lt<typename request_t::target_t>()(a.target, b.target);
    }
  };

  template<class request_t>
  struct request_snd_lt
  {
    // TODO (DEBUG): sort by source, if it exists
    inline bool operator()(const request_t &a, const request_t &b)
    {
      return tuple_snd_lt<typename request_t::target_t>()(a.target, b.target);
    }
  };
}

#endif // ADIAR_INTERNAL_DATA_TYPES_REQUEST_H
