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

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of a variable label.
    ////////////////////////////////////////////////////////////////////////////
    using label_type = node::label_type;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of a pointer.
    ////////////////////////////////////////////////////////////////////////////
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
    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Default construction (trivial).
    ///
    /// \details The default, copy, and move construction has to be `default` to
    ///          ensure it is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    request() = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Copy construction (trivial).
    ///
    /// \details The default, copy, and move construction has to be `default` to
    ///          ensure it is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    request(const request &r) = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Move construction (trivial).
    ///
    /// \details The default, copy, and move construction has to be `default` to
    ///          ensure it is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    request(request &&r) = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Destruction (trivial).
    ///
    /// \details The destructor has to be `default` to ensure it is a *POD* and
    ///          hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    ~request() = default;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Copy assignment (trivial).
    ///
    /// \details The copy and move assignment has to be `default` to ensure it
    ///          is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    request& operator =(const request &r) = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Move assignment (trivial).
    ///
    /// \details The copy and move assignment has to be `default` to ensure it
    ///          is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    request& operator =(request &&r) = default;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for a request to the given `target`.
    ///
    /// \param t  Target of the request
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
    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Default construction (trivial).
    ///
    /// \details The default, copy, and move construction has to be `default` to
    ///          ensure it is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    request() = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Copy construction (trivial).
    ///
    /// \details The default, copy, and move construction has to be `default` to
    ///          ensure it is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    request(const request &r) = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Move construction (trivial).
    ///
    /// \details The default, copy, and move construction has to be `default` to
    ///          ensure it is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    request(request &&r) = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Destruction (trivial).
    ///
    /// \details The destructor has to be `default` to ensure it is a *POD* and
    ///          hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    ~request() = default;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Copy assignment (trivial).
    ///
    /// \details The copy and move assignment has to be `default` to ensure it
    ///          is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    request& operator =(const request &r) = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Move assignment (trivial).
    ///
    /// \details The copy and move assignment has to be `default` to ensure it
    ///          is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    request& operator =(request &&r) = default;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construction of a request.
    ///
    /// \param t  Target of the request
    ///
    /// \param nc List of the children that already have been visited but needs
    ///           to be forwarded to fully resolve the request to `t`.
    ////////////////////////////////////////////////////////////////////////////
    request(const typename base::target_t &t,
            const std::array<typename base::children_type, node_carry_size> &nc)
      : base(t), node_carry(nc)
    { }
  };

  //////////////////////////////////////////////////////////////////////////////
  // Priority queue functions

  // TODO: turn it into only having the cardinality

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Level/Lexicographical ordering on a request' target(s).
  //////////////////////////////////////////////////////////////////////////////
  // TODO: request_lt without `idx` for a pure lexicographical ordering...
  //  - Flip parameters, make `idx = 0` default.
  //  - Optimise for default case.
  template<typename Request, size_t idx = 0>
  struct request_lt
  {
    static_assert(idx == 0, "Non-lexicographical ordering not (yet) supported.");

    /// \copydoc request_lt
    inline bool operator()(const Request &a, const Request &b)
    {
      const typename Request::label_type label_a = a.target.first().label();
      const typename Request::label_type label_b = b.target.first().label();

      return label_a < label_b || (label_a == label_b && a.target < b.target);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Ordering based on the first target in sorted order.
  //////////////////////////////////////////////////////////////////////////////
  template<typename Request>
  struct request_first_lt
  {
    /// \copydoc request_first_lt
    inline bool operator()(const Request &a, const Request &b)
    {
      return tuple_first_lt<typename Request::target_t>()(a.target, b.target);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Ordering based on the second target in sorted order.
  //////////////////////////////////////////////////////////////////////////////
  template<typename Request>
  struct request_second_lt
  {
    /// \copydoc request_second_lt
    inline bool operator()(const Request &a, const Request &b)
    {
      return tuple_second_lt<typename Request::target_t>()(a.target, b.target);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Ordering based on the third target in sorted order.
  //////////////////////////////////////////////////////////////////////////////
  template<typename Request>
  struct request_third_lt
  {
    /// \copydoc request_third_lt
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
    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Default construction (trivial).
    ///
    /// \details The default, copy, and move construction has to be `default` to
    ///          ensure it is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    request_data() = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Copy construction (trivial).
    ///
    /// \details The default, copy, and move construction has to be `default` to
    ///          ensure it is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    request_data(const request_data &r) = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Move construction (trivial).
    ///
    /// \details The default, copy, and move construction has to be `default` to
    ///          ensure it is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    request_data(request_data &&r) = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Destruction (trivial).
    ///
    /// \details The destructor has to be `default` to ensure it is a *POD* and
    ///          hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    ~request_data() = default;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Copy assignment (trivial).
    ///
    /// \details The copy and move assignment has to be `default` to ensure it
    ///          is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    request_data& operator =(const request_data &r) = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Move assignment (trivial).
    ///
    /// \details The copy and move assignment has to be `default` to ensure it
    ///          is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    request_data& operator =(request_data &&r) = default;


  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construction of a request.
    ///
    /// \param t  Target of the request
    ///
    /// \param nc List of the children that already have been visited but needs
    ///           to be forwarded to fully resolve the request to `t`.
    ///
    /// \param d  Additional data to be forwarded.
    ////////////////////////////////////////////////////////////////////////////
    request_data(const typename Request::target_t &t,
                 const std::array<typename Request::children_type, NodeCarrySize> &nc,
                 const data_type &d)
      : Request(t, nc), data(d)
    { }
  };

  //////////////////////////////////////////////////////////////////////////////
  // Priority queue functions

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Level/Lexicographical ordering on a request's target(s). Ties are
  ///        (potentially) broken on the data.
  //////////////////////////////////////////////////////////////////////////////
  template<typename Request, size_t idx = 0>
  struct request_data_lt
  {
    /// \copydoc request_data_lt
    inline bool operator()(const Request &a, const Request &b)
    {
      if constexpr (Request::data_type::sort_on_tiebreak) {
        if (a.target == b.target) return a.data < b.data;
      }
      return request_lt<Request, idx>()(a, b);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Ordering based on the first target in sorted order. Ties are
  ///        (potentially) broken on the data.
  //////////////////////////////////////////////////////////////////////////////
  template<typename Request>
  struct request_data_first_lt
  {
    /// \copydoc request_first_lt
    inline bool operator()(const Request &a, const Request &b)
    {
      if constexpr (Request::data_type::sort_on_tiebreak) {
        if (a.target == b.target) return a.data < b.data;
      }
      return request_first_lt<Request>()(a, b);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Ordering based on the second target in sorted order. Ties are
  ///        (potentially) broken on the data.
  //////////////////////////////////////////////////////////////////////////////
  template<typename Request>
  struct request_data_second_lt
  {
    /// \copydoc request_second_lt
    inline bool operator()(const Request &a, const Request &b)
    {
      if constexpr (Request::data_type::sort_on_tiebreak) {
        if (a.target == b.target) return a.data < b.data;
      }
      return request_second_lt<Request>()(a, b);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Ordering based on the third target in sorted order. Ties are
  ///        (potentially) broken on the data.
  //////////////////////////////////////////////////////////////////////////////
  template<typename Request>
  struct request_data_third_lt
  {
    /// \copydoc request_third_lt
    inline bool operator()(const Request &a, const Request &b)
    {
      if constexpr (Request::data_type::sort_on_tiebreak) {
        if (a.target == b.target) return a.data < b.data;
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
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the parent pointer should be part of resolving sorting
    /// two requests.
    ///
    /// \details Currently, this is only true in debug mode, since we do not
    /// need it for correctness but only for predictability of the order of the
    /// output.
    ////////////////////////////////////////////////////////////////////////////
#ifdef NDEBUG
    static constexpr bool sort_on_tiebreak = false;
#else
    static constexpr bool sort_on_tiebreak = true;
#endif

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Pointer to the source of the request.
    ////////////////////////////////////////////////////////////////////////////
    node::pointer_type source;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Comparator to break ties based on the .
    ////////////////////////////////////////////////////////////////////////////
    inline bool operator< (const with_parent &o) const
    { return this->source < o.source; }
  };
}

#endif // ADIAR_INTERNAL_DATA_TYPES_REQUEST_H
