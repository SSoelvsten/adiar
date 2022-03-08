#ifndef ADIAR_INTERNAL_DECISION_DIAGRAM_H
#define ADIAR_INTERNAL_DECISION_DIAGRAM_H

#include <adiar/file.h>

#include <variant>

namespace adiar
{
  class decision_diagram;

  //////////////////////////////////////////////////////////////////////////////
  /// A std::variant is used to distinguish the type of file. This uses
  /// std::monostate to hold a 'nothing' value, i.e. when there is no file.
  //////////////////////////////////////////////////////////////////////////////
  typedef std::monostate no_file;

  //////////////////////////////////////////////////////////////////////////////
  /// \warning You should never explicitly be dealing with this class or have it
  /// be an l-value. Implicit conversion from an unreduced to the reduced type
  /// will call the correct reduce algorithm.
  ///
  /// An algorithm may return a node-based decision diagram in a
  /// <tt>node_file</tt> or a yet to-be reduced decision diagram in an
  /// <tt>arc_file</tt>. So, we use a <tt>std::variant</tt> to hold the the
  /// <tt>node_file</tt> or <tt>arc_file</tt> without having to pay for the
  /// expensive constructors and use a lot of space. The implicit
  ///
  /// We also include a <tt>std::monostate</tt> to allow an algorithm to return
  /// None, though that will lead to an exception in most cases.
  //////////////////////////////////////////////////////////////////////////////
  class __decision_diagram
  {
  public:
    ////////////////////////////////////////////////////////////////////////////
    // Friends
    template <typename T, bool REVERSE>
    friend class level_info_stream;

    ////////////////////////////////////////////////////////////////////////////
    // Union of node_file and arc_file (with std::monostate for 'error')
    const std::variant<std::monostate, node_file, arc_file> _union;

    ////////////////////////////////////////////////////////////////////////////
    // Propagating the negation flag below
    const bool negate = false;

    ////////////////////////////////////////////////////////////////////////////
    // Constructors
    __decision_diagram() { }

    __decision_diagram(const node_file &f) : _union(f) { }

    __decision_diagram(const arc_file &f) : _union(f) { }

    __decision_diagram(const decision_diagram &dd); // decision_diagram.cpp

    ////////////////////////////////////////////////////////////////////////////
    // Accessors
    template<typename T>
    bool has() const
    {
      return std::holds_alternative<T>(_union);
    }

    template<typename T>
    const T& get() const
    {
      return std::get<T>(_union);
    }

    bool empty() const
    {
      return has<no_file>();
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// Container for the files that represent a Decision Diagram. To ensure the
  /// most disk-space is available, try to garbage collect objects of this type
  /// as quickly as possible and/or minimise the number of lvalues of this type.
  //////////////////////////////////////////////////////////////////////////////
  class decision_diagram
  {
    ////////////////////////////////////////////////////////////////////////////
    // Friends
    // |- classes
    friend class __decision_diagram;

    template <typename T, bool REVERSE>
    friend class level_info_stream;

    template <bool REVERSE>
    friend class node_stream;

    template <typename file_t, typename comp_t, size_t FILES>
    friend class label_merger;

    friend bool is_canonical(const decision_diagram &dd);
    friend bool is_isomorphic(const decision_diagram&, const decision_diagram&);

    template<typename comp_policy>
    friend bool comparison_check(const decision_diagram &in_1, const decision_diagram &in_2);

    friend size_t __count_size_based_upper_bound(const decision_diagram &dd);

    template<typename prod_policy>
    friend typename prod_policy::unreduced_t product_construction(const typename prod_policy::reduced_t &in_1,
                                                                  const typename prod_policy::reduced_t &in_2,
                                                                  const bool_op &op);

    template<typename prod_policy>
    friend size_t __prod_size_based_upper_bound(const typename prod_policy::reduced_t &in_1,
                                                const typename prod_policy::reduced_t &in_2);

    friend size_t __ite_size_based_upper_bound(const decision_diagram &in_if,
                                               const decision_diagram &in_then,
                                               const decision_diagram &in_else);

    template<typename quantify_policy>
    friend size_t __quantify_size_based_upper_bound(const typename quantify_policy::reduced_t &in);

    friend label_t min_label(const decision_diagram &dd);
    friend label_t max_label(const decision_diagram &dd);

    template<typename to_policy, typename from_policy>
    friend class convert_decision_diagram_policy;

    ////////////////////////////////////////////////////////////////////////////
    // Internal state
  protected:
    bool negate = false;
    node_file file;

    void free()
    {
      file._file_ptr.reset();
    }

    ////////////////////////////////////////////////////////////////////////////
    // Constructors
  public:
    decision_diagram(const node_file &f, bool negate = false)
      : negate(negate), file(f) { }

    decision_diagram(const decision_diagram &dd)
      : negate(dd.negate), file(dd.file) { }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Check whether a given decision diagram is canonical, i.e. has the
  /// following stronger guarantees than the total ordering of nodes.
  ///
  /// - Nodes within a level are effectively sorted based on their children:
  ///   high first, then low.
  ///
  /// - Identifiers are from MAX_ID and down (when read bottom-up)
  ///
  /// If this is true, then equality checking (see 'internal/pred.h') can be
  /// done in a single cheap linear scan rather than with an O(N log N)
  /// time-forwarding algorithm.
  //////////////////////////////////////////////////////////////////////////////
  bool is_canonical(const decision_diagram &dd);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Check whether a given decision diagram is sink-only and satisfies
  /// the given sink_pred.
  ///
  /// \param file   The node_file to check its content
  /// \param pred   If the given decision diagram is sink-only, then secondly
  ///               the sink is checked with the given sink predicate. Default
  ///               is any type sink.
  //////////////////////////////////////////////////////////////////////////////
  bool is_sink(const decision_diagram &dd, const sink_pred &pred = is_any);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get the minimal occurring label in the decision diagram
  //////////////////////////////////////////////////////////////////////////////
  label_t min_label(const decision_diagram &dd);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get the maximal occurring label in the decision diagram
  //////////////////////////////////////////////////////////////////////////////
  label_t max_label(const decision_diagram &dd);
}

#endif // ADIAR_INTERNAL_DECISION_DIAGRAM_H
