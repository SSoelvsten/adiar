#ifndef ADIAR_INTERNAL_DATA_TYPES_REQUEST_H
#define ADIAR_INTERNAL_DATA_TYPES_REQUEST_H

#include <algorithm>
#include <array>

#include <adiar/internal/assert.h>
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
  /// \tparam Cardinality   The cardinality of the algorithm, e.g. it is 1 for
  ///                       `bdd_restrict`, 2 for `bdd_apply` and 3 for
  ///                       `bdd_ite`.
  ///
  /// \tparam NodeCarrySize Number of children being forwarded with the request.
  ///                       This is used when `cardinality` is greater than 1
  ///                       and a per-level priority queue forwards the children
  ///                       of `target.first()` to `target.second()` and so on.
  ///
  /// \tparam Inputs        The number of inputs to the algorithm. This value
  ///                       differs from `Cardinality` (default) in algorithms
  ///                       such as `bdd_exists` and `bdd_compose` where the
  ///                       request includes pairs of nodes from the same input.
  //////////////////////////////////////////////////////////////////////////////
  template<uint8_t Cardinality,
           uint8_t NodeCarrySize = 0u,
           uint8_t Inputs = Cardinality>
  class request;

  //////////////////////////////////////////////////////////////////////////////
  /// \details Common details for requests with and without a node carry.
  //////////////////////////////////////////////////////////////////////////////
  template<uint8_t Cardinality, uint8_t Inputs>
  class request<Cardinality, 0, Inputs>
  {
  public:
    ////////////////////////////////////////////////////////////////////////////
    static_assert(0 < Cardinality,
                  "Request type is not designed for 0-ary targets, i.e. without at least one node in either diagram.");

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Cardinality of the request target.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr uint8_t cardinality = Cardinality;

    ////////////////////////////////////////////////////////////////////////////
    static_assert(Inputs > 0,
                  "Request type is not designed for a 0-ary operation, i.e. an algorithm taking no diagrams as input.");

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of input files.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr uint8_t inputs = Inputs;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the target tuple ought to be sorted.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr bool sorted_target = cardinality == 1u || inputs == 1u;

    using label_type = node::label_type;
    using pointer_type = node::pointer_type;

    /* ========================== RECURSION TARGET ========================== */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the target tuple.
    ////////////////////////////////////////////////////////////////////////////
    using target_t = tuple<pointer_type, cardinality, sorted_target>;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Target node(s) of recursion request.
    ////////////////////////////////////////////////////////////////////////////
    target_t target;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The level at which this request should be resolved.
    ////////////////////////////////////////////////////////////////////////////
    pointer_type::label_type level() const
    { return target.first().label(); }

    /* ============================= NODE CARRY ============================= */

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Size of the node forwarding array.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr uint8_t node_carry_size = 0u;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the set of children carried to the last in `target`.
    ////////////////////////////////////////////////////////////////////////////
    using children_type = node::children_type;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The value to be inserted in empy slots in the `node_carry`.
    ////////////////////////////////////////////////////////////////////////////
    static inline constexpr children_type NO_CHILDREN()
    { return children_type(pointer_type::nil()); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The number of nodes actually carried by this request.
    ////////////////////////////////////////////////////////////////////////////
    uint8_t nodes_carried() const
    { return 0u; }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether no nodes are carried by this request.
    ////////////////////////////////////////////////////////////////////////////
    bool empty_carry() const
    { return true; }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The number of non-nil target values.
    ////////////////////////////////////////////////////////////////////////////
    uint8_t targets() const
    {
      if constexpr (sorted_target) {
        // Since nil is the greatest value, we can look for the first nil entry
        // (if any).
        for (uint8_t i = 0u; i < cardinality; i++) {
          if (target[i] == pointer_type::nil()) { return i; }
        }
        return cardinality;
      } else { // !sorted_target
        uint8_t sum = 0u;
        for (uint8_t i = 0u; i < cardinality; i++) {
          if (target[i] != pointer_type::nil()) { sum++; }
        }
        return sum;
      }
    }

    /* ============================ CONSTRUCTORS ============================ */
  public:
    // Provide 'default' constructors to ensure it being a 'POD' inside of TPIE.
    request() = default;
    request(const request &r) = default;
    ~request() = default;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for a request to the given `target`.
    ////////////////////////////////////////////////////////////////////////////
    request(const target_t &t) : target(t)
    { }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for a request to the given `target` with an empty
    ///        node carry.
    ////////////////////////////////////////////////////////////////////////////
    request(const target_t &t,
            const std::array<children_type, node_carry_size>&/*nc*/)
      : request(t)
    { }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \details Implementation of `request` template with a non-empty node carry.
  //////////////////////////////////////////////////////////////////////////////
  template<uint8_t Cardinality, uint8_t NodeCarrySize, uint8_t Inputs>
  class request : public request<Cardinality, 0, Inputs>
  {
    using base = request<Cardinality, 0, Inputs>;

    /* ============================= NODE CARRY ============================= */
  public:
    ////////////////////////////////////////////////////////////////////////////
    static_assert(NodeCarrySize < base::cardinality,
                  "'NodeCarrySize' ought not to hold more than the 'cardinality' of the algorithm");

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Size of the node forwarding array.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr uint8_t node_carry_size = NodeCarrySize;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set of nodes of prior visited nodes in `target`.
    ///
    /// \details This is not implemented as a `std::array<children_type,
    /// node_carry_size>` because for `node_carry_size` being 0 we still spend 8
    /// bytes on empty space.
    ///
    /// \see NO_CHILDREN
    ////////////////////////////////////////////////////////////////////////////
    std::array<typename base::children_type, node_carry_size> node_carry;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The number of nodes actually carried within `node_carry`.
    ///
    /// \see NO_CHILDREN
    ////////////////////////////////////////////////////////////////////////////
    uint8_t nodes_carried() const
    {
      uint8_t sum = 0u;
      for (uint8_t n_idx = 0u; n_idx < node_carry_size; n_idx++) {
        if (node_carry[n_idx][0] == base::pointer_type::nil()) {
          adiar_assert(node_carry[n_idx] == base::NO_CHILDREN(),
                       "Either no entry is nil or all of them are");
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
    /// \see nodes_carried, NO_CHILDREN
    ////////////////////////////////////////////////////////////////////////////
    bool empty_carry() const
    {
      if constexpr (node_carry_size == 0u) return true;
      return node_carry[0][0] == base::pointer_type::nil();
    }

    /* ============================ CONSTRUCTORS ============================ */
  public:
    // Provide 'default' constructors to ensure it being a 'POD' inside of TPIE.
    request() = default;
    request(const request &r) = default;
    ~request() = default;

  public:
    // Provide 'non-default' constructors to make it easy to use outside of TPIE.
    request(const typename base::target_t &t,
            const std::array<typename base::children_type, node_carry_size> &nc)
      : base(t), node_carry(nc)
    { }
  };

  //////////////////////////////////////////////////////////////////////////////
  // Priority queue functions

  // TODO: turn it into only having the cardinality
  template<size_t idx, typename Request>
  struct request_lt
  {
    inline bool operator()(const Request &a, const Request &b)
    {
      const typename Request::label_type label_a = a.target.first().label();
      const typename Request::label_type label_b = b.target.first().label();
      
      if constexpr (Request::cardinality == 2) {
        constexpr size_t o_idx = 1u - idx;

        return label_a < label_b
          || (label_a == label_b && a.target[idx] < b.target[idx])
          || (label_a == label_b && a.target[idx] == b.target[idx] && a.target[o_idx] < b.target[o_idx]);
      }

      return label_a < label_b
        || (label_a == label_b && a.target[idx] < b.target[idx])
        || (label_a == label_b && a.target[idx] == b.target[idx] && a.target < b.target);
    }
  };

  template<typename Request>
  struct request_first_lt
  {
    inline bool operator()(const Request &a, const Request &b)
    {
      return tuple_first_lt<typename Request::target_t>()(a.target, b.target);
    }
  };

  template<typename Request>
  struct request_second_lt
  {
    inline bool operator()(const Request &a, const Request &b)
    {
      return tuple_second_lt<typename Request::target_t>()(a.target, b.target);
    }
  };

  template<typename Request>
  struct request_third_lt
  {
    inline bool operator()(const Request &a, const Request &b)
    {
      return tuple_third_lt<typename Request::target_t>()(a.target, b.target);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Extension of the request struct to carry extra data. That data can
  ///        also impact the ordering.
  ///
  /// \see request
  //////////////////////////////////////////////////////////////////////////////
  template<uint8_t Cardinality,
           typename Data,
           uint8_t NodeCarrySize = 0u,
           uint8_t Inputs = Cardinality>
  class request_data 
    : public request<Cardinality, NodeCarrySize, Inputs>
  {
  private:
    using Request = request<Cardinality, NodeCarrySize, Inputs>;

    /* ================================ DATA ================================ */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the extra data carried with this recursion request.
    ////////////////////////////////////////////////////////////////////////////
    using data_type = Data;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Extra data related to this recursion request tuple.
    ////////////////////////////////////////////////////////////////////////////
    data_type data;

    /* ============================ CONSTRUCTORS ============================ */
  public:
    // Provide 'default' constructors to ensure it being a 'POD' inside of TPIE.
    request_data() = default;
    request_data(const request_data &r) = default;
    ~request_data() = default;

  public:
    // Provide 'non-default' constructors to make it easy to use outside of TPIE.
    request_data(const typename Request::target_t &t,
                 const std::array<typename Request::children_type, NodeCarrySize> &nc,
                 const data_type &d)
      : Request(t, nc), data(d)
    { }
  };

  //////////////////////////////////////////////////////////////////////////////
  // Priority queue functions
  template<size_t idx, typename Request>
  struct request_data_lt
  {
    inline bool operator()(const Request &a, const Request &b)
    {
      if (Request::data_type::sort_on_tiebreak && a.target == b.target) {
        return a.data < b.data;
      }
      return request_lt<idx, Request>()(a, b);
    }
  };

  template<typename Request>
  struct request_data_first_lt
  {
    inline bool operator()(const Request &a, const Request &b)
    {
      if (Request::data_type::sort_on_tiebreak && a.target == b.target) {
        return a.data < b.data;
      }
      return request_first_lt<Request>()(a, b);
    }
  };

  template<typename Request>
  struct request_data_second_lt
  {
    inline bool operator()(const Request &a, const Request &b)
    {
      if (Request::data_type::sort_on_tiebreak && a.target == b.target) {
        return a.data < b.data;
      }
      return request_second_lt<Request>()(a, b);
    }
  };

  template<typename Request>
  struct request_data_third_lt
  {
    inline bool operator()(const Request &a, const Request &b)
    {
      if (Request::data_type::sort_on_tiebreak && a.target == b.target) {
        return a.data < b.data;
      }
      return request_third_lt<Request>()(a, b);
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
    node::pointer_type source;

    ////////////////////////////////////////////////////////////////////////////
    inline bool operator< (const with_parent &o) const
    { return this->source < o.source; }
  };
}

#endif // ADIAR_INTERNAL_DATA_TYPES_REQUEST_H
