#ifndef ADIAR_INTERNAL_DATA_TYPES_REQUEST_H
#define ADIAR_INTERNAL_DATA_TYPES_REQUEST_H

#include<adiar/internal/data_types/node.h>
#include<adiar/internal/data_types/tuple.h>

namespace adiar
{
  // TODO (ADD (64-bit)):
  // TODO (10+ TiB Decision Diagrams):
  // TODO (MDD):
  // TODO (QMDD):
  //   Template with the type of the node.

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
  template<uint8_t cardinality, uint8_t nodes_carried>
  class request
  {
  public:
    static_assert(0 < cardinality,
                  "Request type is not designed for 0-ary algorithms.");

    static_assert(nodes_carried < cardinality,
                  "'nodes_carried' ought not to hold more than the 'cardinality' of the algorithm");

    typedef node::label_t label_t;
    typedef node::ptr_t ptr_t;

    /* ============================== VARIABLES ============================= */
  public:
    typedef tuple<ptr_t, cardinality> target_t;
    target_t target;

    typedef node::children_t children_t;
    children_t node_carry[nodes_carried];

    /* ============================ CONSTRUCTORS ============================ */
  public:
    // Provide 'default' constructors to ensure it being a 'POD' inside of TPIE.
    request() = default;
    request(const request &r) = default;
    ~request() = default;

  public:
    // Provide 'non-default' constructors to make it easy to use outside of TPIE.
    request(const target_t &t,
            const children_t (& nc) [nodes_carried])
      : target(t), node_carry(nc)
    { }

    /* ======================== DERIVATED INFORMATION ======================= */

    // TODO: how much data is 'actually' carried? within the priority queues
    // this of course ought to be exactly `nodes_carried`, but within the
    // algorithms, we want to reuse this same class with `NIL` as placeholders.
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

  // TODO: turn it into only having the cardinality
  template<class request_t>
  struct request_fst_lt
  {
    inline bool operator()(const request_t &a, const request_t &b)
    {
      return tuple_fst_lt<typename request_t::target_t>()(a.target, b.target);
    }
  };

  template<class request_t>
  struct request_snd_lt
  {
    inline bool operator()(const request_t &a, const request_t &b)
    {
      return tuple_snd_lt<typename request_t::target_t>()(a.target, b.target);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Extension of the request struct to carry extra data. That data can
  ///        also impact the ordering.
  ///
  /// \sa request
  //////////////////////////////////////////////////////////////////////////////
  template<uint8_t cardinality, uint8_t nodes_carried, typename data_t>
  class request_data : public request<cardinality, nodes_carried>, public data_t
  {
    /* ============================ CONSTRUCTORS ============================ */
  public:
    // Provide 'default' constructors to ensure it being a 'POD' inside of TPIE.
    request_data() = default;
    request_data(const request_data &r) = default;
    ~request_data() = default;

  public:
    // Provide 'non-default' constructors to make it easy to use outside of TPIE.
    request_data(const typename request<cardinality, nodes_carried>::target_t &t,
                 const typename request<cardinality, nodes_carried>::children_t (& nc) [nodes_carried],
                 const data_t &d)
      : request<cardinality, nodes_carried>(t, nc), data_t(d)
    { }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Class to carry the parent of a recursion within `request_data`.
  //////////////////////////////////////////////////////////////////////////////
  class with_parent
  {
  public:
    node::ptr_t source;
  };
}

#endif // ADIAR_INTERNAL_DATA_TYPES_REQUEST_H
