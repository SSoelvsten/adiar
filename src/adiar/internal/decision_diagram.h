#ifndef ADIAR_DECISION_DIAGRAM_H
#define ADIAR_DECISION_DIAGRAM_H

#include <adiar/union.h>
#include <adiar/file.h>

namespace adiar {
  // Class declarations to be able to reference it
  class decision_diagram;

  //////////////////////////////////////////////////////////////////////////////
  /// An algorithm may return a node-based decision diagram in a node_file or a
  /// yet to-be reduced decision diagram in an arc_file. So, the union_t will be
  /// a wrapper for the combined type.
  ///
  /// The union_t class ensures we don't call any expensive yet unnecessary
  /// constructors and ensures only one of the two types are instantiated at a
  /// time.
  //////////////////////////////////////////////////////////////////////////////
  class __decision_diagram : public union_t<node_file, arc_file>
  {
    ////////////////////////////////////////////////////////////////////////////
    // 'privatize' mutating functions from union.h
  protected:
    using union_t<node_file, arc_file>::set;

    ////////////////////////////////////////////////////////////////////////////
    // Propagating the negation flag below
  public:
    const bool negate = false;

  public:
    ////////////////////////////////////////////////////////////////////////////
    // Constructors
    __decision_diagram(const node_file &f) : union_t(f) { }

    __decision_diagram(const arc_file &f) : union_t(f) { }

    __decision_diagram(const decision_diagram &dd); // decision_diagram.cppx
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
    friend class meta_stream;

    template <bool REVERSE>
    friend class node_stream;

    template <typename File_T, size_t Files,
              typename T,
              typename LabelExt,
              typename TComparator, typename LabelComparator,
              size_t MetaStreams, size_t Buckets>
    friend class levelized_priority_queue;

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

#endif // ADIAR_DECISION_DIAGRAM_H
