#ifndef ADIAR_INTERNAL_DATA_TYPES_REQUEST_H
#define ADIAR_INTERNAL_DATA_TYPES_REQUEST_H

#include <adiar/internal/data_types/node.h>
#include <adiar/internal/data_types/tuple.h>

namespace adiar::internal
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
  template<uint8_t CARDINALITY,
           uint8_t NODE_CARRY_SIZE = 0u,
           uint8_t INPUTS = CARDINALITY>
  class request
  {
  public:
    ////////////////////////////////////////////////////////////////////////////
    static_assert(0 < CARDINALITY,
                  "Request type is not designed for 0-ary targets, i.e. without at least one node in either diagram.");

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Cardinality of the request target.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr uint8_t cardinality = CARDINALITY;

    ////////////////////////////////////////////////////////////////////////////
    static_assert(NODE_CARRY_SIZE < cardinality,
                  "'node_carry_size' ought not to hold more than the 'cardinality' of the algorithm");

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Size of the node forwarding array.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr uint8_t node_carry_size = NODE_CARRY_SIZE;

    ////////////////////////////////////////////////////////////////////////////
    static_assert(INPUTS > 0,
                  "Request type is not designed for a 0-ary operation, i.e. an algorithm taking no diagrams as input.");

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of input files.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr uint8_t inputs = INPUTS;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the target tuple ought to be sorted.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr uint8_t sorted_target = cardinality == 1u || inputs == 1u;

    typedef node::label_t label_t;
    typedef node::ptr_t ptr_t;

    /* ========================== RECURSION TARGET ========================== */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the target tuple.
    ////////////////////////////////////////////////////////////////////////////
    typedef tuple<ptr_t, cardinality, sorted_target> target_t;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Target node(s) of recursion request.
    ////////////////////////////////////////////////////////////////////////////
    target_t target;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The level at which this request should be resolved.
    ////////////////////////////////////////////////////////////////////////////
    ptr_t::label_t level() const
    { return target.fst().label(); }

    /* ============================= NODE CARRY ============================= */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the set of children carried to the last in `target`.
    ////////////////////////////////////////////////////////////////////////////
    typedef node::children_t children_t;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set of nodes of prior visited nodes in `target`.
    ///
    /// \sa NO_CHILDREN
    ////////////////////////////////////////////////////////////////////////////
    children_t node_carry[node_carry_size];

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The value to be inserted in empy slots in the `node_carry`.
    ////////////////////////////////////////////////////////////////////////////
    static inline constexpr children_t NO_CHILDREN()
    { return node::children_t(node::ptr_t::NIL()); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The number of nodes actually carried within `node_carry`.
    ///
    /// \sa NO_CHILDREN
    ////////////////////////////////////////////////////////////////////////////
    uint8_t nodes_carried() const
    {
      uint8_t sum = 0u;
      for (uint8_t n_idx = 0u; n_idx < node_carry_size; n_idx++) {
        if (node_carry[n_idx][0] == node::ptr_t::NIL()) {
          adiar_debug(node_carry[n_idx] == NO_CHILDREN(),
                      "Either no entry is NIL or all of them are");
          break;
        }
        sum++;
      }
      return sum;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the node carry is empty. That is, this is a faster way to
    ///        compute whether `nodes_carried() == 0`.
    ///
    /// \sa nodes_carried, NO_CHILDREN
    ////////////////////////////////////////////////////////////////////////////
    bool empty_carry() const
    {
      if constexpr (node_carry_size == 0u) return true;
      return node_carry[0][0] == node::ptr_t::NIL();
    }

    /* ============================ CONSTRUCTORS ============================ */
  public:
    // Provide 'default' constructors to ensure it being a 'POD' inside of TPIE.
    request() = default;
    request(const request &r) = default;
    ~request() = default;

  public:
    // Provide 'non-default' constructors to make it easy to use outside of TPIE.
    request(const target_t &t,
            const children_t (& nc) [node_carry_size])
      : target(t), node_carry(nc)
    { }
  };

  //////////////////////////////////////////////////////////////////////////////
  // Priority queue functions

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
  template<uint8_t cardinality,
           typename data_type,
           uint8_t node_carry_size = 0u,
           uint8_t inputs = cardinality>
  class request_data : public request<cardinality, node_carry_size, inputs>
  {
  private:
    using request_t = request<cardinality, node_carry_size, inputs>;

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
                 const typename request_t::children_t (& nc) [node_carry_size],
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
