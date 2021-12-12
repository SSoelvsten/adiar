#ifndef ADIAR_INTERNAL_DECISION_DIAGRAM_H
#define ADIAR_INTERNAL_DECISION_DIAGRAM_H

#include <adiar/file.h>

#include <variant>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  // Class declarations to be able to reference it
  class decision_diagram;

  //////////////////////////////////////////////////////////////////////////////
  /// A std::variant is used to distinguish the type of file. This uses
  /// std::monostate to hold a 'nothing' value, i.e. when there is no file.
  //////////////////////////////////////////////////////////////////////////////
  typedef std::monostate no_file;

  //////////////////////////////////////////////////////////////////////////////
  /// An algorithm may return a node-based decision diagram in a node_file or a
  /// yet to-be reduced decision diagram in an arc_file. So, we use a
  /// std::variant to hold the the node_file or arc_file without having to pay
  /// for the expensive constructors and use a lot of space.
  ///
  /// We also include a std::monostate to allow an algorithm to return None,
  /// though that will lead to an exception in most cases.
  //////////////////////////////////////////////////////////////////////////////
  class __decision_diagram
  {
  public:
    ////////////////////////////////////////////////////////////////////////////
    // Friends
    template <typename T, size_t Files, bool REVERSE>
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
  };

  //////////////////////////////////////////////////////////////////////////////
  /// A decision diagram (as used by the user) is the reduced node-based
  /// representation.
  //////////////////////////////////////////////////////////////////////////////
  class decision_diagram
  {
    ////////////////////////////////////////////////////////////////////////////
    // Friends
    // |- classes
    friend class __decision_diagram;

    template <typename T, size_t Files, bool REVERSE>
    friend class level_info_stream;

    template <bool REVERSE>
    friend class node_stream;

    template <typename File_T, size_t Files,
              typename T,
              typename LabelExt,
              typename TComparator, typename LabelComparator,
              size_t MetaStreams, size_t Buckets>
    friend class levelized_priority_queue;

    friend bool is_canonical(const decision_diagram &dd);
    friend bool is_isomorphic(const decision_diagram&, const decision_diagram&);

    template<typename comp_policy>
    friend bool comparison_check(const decision_diagram &in_1, const decision_diagram &in_2);

    template<typename prod_policy>
    friend typename prod_policy::unreduced_t product_construction(const typename prod_policy::reduced_t &in_1,
                                                                  const typename prod_policy::reduced_t &in_2,
                                                                  const bool_op &op);

    friend label_t min_label(const decision_diagram &dd);
    friend label_t max_label(const decision_diagram &dd);

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
  /// If this is true, then equality checking (see 'pred.h') can be done in a
  /// single cheap linear scan rather than with an O(N log N) time-forwarding
  /// algorithm.
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
