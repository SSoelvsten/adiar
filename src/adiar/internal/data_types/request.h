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
  // TODO: `is_sorted` is always true, when the number of input graphs (not the
  //       algorithm's cardinality) is 1.
  template<uint8_t cardinality, bool is_sorted, uint8_t nodes_carried>
  class request
  {
  public:
    static_assert(0 < cardinality,
                  "Request type is not designed for 0-ary algorithms.");

    static_assert(nodes_carried < cardinality,
                  "'nodes_carried' ought not to hold more than the 'cardinality' of the algorithm");

    typedef node::label_t label_t;
    typedef node::ptr_t ptr_t;

    /* ========================== RECURSION TARGET ========================== */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the target tuple.
    ////////////////////////////////////////////////////////////////////////////
    typedef tuple<ptr_t, cardinality, is_sorted> target_t;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Target node(s) of recursion request.
    ////////////////////////////////////////////////////////////////////////////
    target_t target;

    /* ============================= NODE CARRY ============================= */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the set of children carried to the last in `target`.
    ////////////////////////////////////////////////////////////////////////////
    typedef node::children_t children_t;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set of nodes of prior visited nodes in `target.`
    ////////////////////////////////////////////////////////////////////////////
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

  template<class request_t>
  struct request_trd_lt
  {
    inline bool operator()(const request_t &a, const request_t &b)
    {
      return tuple_trd_lt<typename request_t::target_t>()(a.target, b.target);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Extension of the request struct to carry extra data. That data can
  ///        also impact the ordering.
  ///
  /// \sa request
  //////////////////////////////////////////////////////////////////////////////
  template<uint8_t cardinality, bool is_sorted, uint8_t nodes_carried,
           typename data_type>
  class request_data : public request<cardinality, is_sorted, nodes_carried>
  {
  private:
    using request_t = request<cardinality, is_sorted, nodes_carried>;

    /* ================================ DATA ================================ */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the extra data carried with this recursion request.
    ////////////////////////////////////////////////////////////////////////////
    typedef data_type data_t;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Extra data related to this recursion request tuple.
    ////////////////////////////////////////////////////////////////////////////
    data_t data;

    /* ============================ CONSTRUCTORS ============================ */
  public:
    // Provide 'default' constructors to ensure it being a 'POD' inside of TPIE.
    request_data() = default;
    request_data(const request_data &r) = default;
    ~request_data() = default;

  public:
    // Provide 'non-default' constructors to make it easy to use outside of TPIE.
    request_data(const typename request_t::target_t &t,
                 const typename request_t::children_t (& nc) [nodes_carried],
                 const data_t &d)
      : request_t(t, nc), data(d)
    { }
  };

  //////////////////////////////////////////////////////////////////////////////
  // Priority queue functions
  template<class request_t>
  struct request_data_fst_lt
  {
    inline bool operator()(const request_t &a, const request_t &b)
    {
      if (request_t::data_t::sort_on_tiebreak && a.target == b.target) {
        return a.data < b.data;
      }
      return request_fst_lt<request_t>()(a, b);
    }
  };

  template<class request_t>
  struct request_data_snd_lt
  {
    inline bool operator()(const request_t &a, const request_t &b)
    {
      if (request_t::data_t::sort_on_tiebreak && a.target == b.target) {
        return a.data < b.data;
      }
      return request_snd_lt<request_t>()(a, b);
    }
  };

  template<class request_t>
  struct request_data_trd_lt
  {
    inline bool operator()(const request_t &a, const request_t &b)
    {
      if (request_t::data_t::sort_on_tiebreak && a.target == b.target) {
        return a.data < b.data;
      }
      return request_trd_lt<request_t>()(a, b);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Class to carry the parent of a recursion within `request_data`.
  //////////////////////////////////////////////////////////////////////////////
  class with_parent
  {
  public:
#ifdef NDEBUG
    static constexpr bool sort_on_tiebreak = false;
#else
    static constexpr bool sort_on_tiebreak = true;
#endif

    ////////////////////////////////////////////////////////////////////////////
    node::ptr_t source;

    ////////////////////////////////////////////////////////////////////////////
    inline bool operator< (const with_parent &o) const
    { return this->source < o.source; }
  };
}

#endif // ADIAR_INTERNAL_DATA_TYPES_REQUEST_H
