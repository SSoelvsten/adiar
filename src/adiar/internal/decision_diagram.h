#ifndef ADIAR_INTERNAL_DECISION_DIAGRAM_H
#define ADIAR_INTERNAL_DECISION_DIAGRAM_H

#include <adiar/file.h>

#include <adiar/internal/cut.h>

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
    const std::variant<no_file, node_file, arc_file> _union;

    ////////////////////////////////////////////////////////////////////////////
    // Propagating the negation flag below
    const bool negate = false;

    ////////////////////////////////////////////////////////////////////////////
    // Constructors
    __decision_diagram() { }

    __decision_diagram(const node_file &f) : _union(f) { }

    __decision_diagram(const arc_file &f) : _union(f) { }

    __decision_diagram(const decision_diagram &dd);

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

    friend bool is_isomorphic(const decision_diagram&, const decision_diagram&);

    template<typename comp_policy>
    friend bool comparison_check(const decision_diagram &in_1, const decision_diagram &in_2);

    friend bool is_sink(const decision_diagram &dd);
    friend bool value_of(const decision_diagram &dd);
    friend label_t min_label(const decision_diagram &dd);
    friend label_t max_label(const decision_diagram &dd);

    template<typename to_policy, typename from_policy>
    friend class convert_decision_diagram_policy;

    ////////////////////////////////////////////////////////////////////////////
    // Internal state
  protected:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief The file describing the actual DAG of the decision diagram.
    ////////////////////////////////////////////////////////////////////////////
    node_file file;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether to negate the leaves when reading <tt>file</tt>.
    ////////////////////////////////////////////////////////////////////////////
    bool negate = false;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Release claim on the underlying file and (possibly) garbage
    ///        collect it.
    ////////////////////////////////////////////////////////////////////////////
    void free()
    {
      file._file_ptr.reset();
    }

    ////////////////////////////////////////////////////////////////////////////
    // Constructors
  public:
    decision_diagram(const node_file &f, bool negate = false)
      : file(f), negate(negate) { }

    decision_diagram(const decision_diagram &dd)
      : file(dd.file), negate(dd.negate) { }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Read-only access to the negation flag.
    ////////////////////////////////////////////////////////////////////////////
    bool is_negated() const
    {
      return negate;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Read-only access to the raw files and meta information.
    ////////////////////////////////////////////////////////////////////////////
    const std::shared_ptr<const __meta_file<node_t>> file_ptr() const
    {
      return file._file_ptr;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Read-only access to the members of the raw files and meta
    ///        information, i.e. this is similar to writing
    ///        <tt>.file_ptr()-></tt>.
    ////////////////////////////////////////////////////////////////////////////
    const __meta_file<node_t>* operator->() const
    {
      return file_ptr().get();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the 1-level cut of the desired type, i.e. of the sub-graph
    ///        including the desired type of arcs.
    ///
    /// \param ct The type of the cut to obtain
    ////////////////////////////////////////////////////////////////////////////
    cut_size_t max_1level_cut(const cut_type ct) const
    {
      return file._file_ptr->max_1level_cut[negate_cut_type(ct)];
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the 2-level cut of the desired type, i.e. of the sub-graph
    ///        including the desired type of arcs.
    ///
    /// \param ct The type of the cut to obtain
    ////////////////////////////////////////////////////////////////////////////
    cut_size_t max_2level_cut(const cut_type ct) const
    {
      return file._file_ptr->max_2level_cut[negate_cut_type(ct)];
    }

  private:
    cut_type negate_cut_type(const cut_type ct) const
    {
      if (!negate) { return ct; }

      switch (ct) {
      case cut_type::INTERNAL_FALSE:
        return cut_type::INTERNAL_TRUE;
      case cut_type::INTERNAL_TRUE:
        return cut_type::INTERNAL_FALSE;
      default:
        return ct;
      }
    }
  };

  inline __decision_diagram::__decision_diagram(const decision_diagram &dd)
    : _union(dd.file), negate(dd.negate) { };

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
  inline bool is_canonical(const decision_diagram &dd)
  {
    return dd->canonical;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a given decision diagram represents a sink.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_sink(const decision_diagram &dd)
  {
    return is_sink(dd.file);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain the sink's value (if 'is_sink' is true).
  //////////////////////////////////////////////////////////////////////////////
  inline bool value_of(const decision_diagram &dd)
  {
    return dd.negate ^ value_of(dd.file);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a given decision diagram represents the false sink.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_false(const decision_diagram &dd)
  {
    return is_sink(dd) && !value_of(dd);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a given decision diagram represents the true sink.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_true(const decision_diagram &dd)
  {
    return is_sink(dd) && value_of(dd);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get the minimal occurring label in the decision diagram
  //////////////////////////////////////////////////////////////////////////////
  inline label_t min_label(const decision_diagram &dd)
  {
    return min_label(dd.file);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get the maximal occurring label in the decision diagram
  //////////////////////////////////////////////////////////////////////////////
  inline label_t max_label(const decision_diagram &dd)
  {
    return max_label(dd.file);
  }
}

#endif // ADIAR_INTERNAL_DECISION_DIAGRAM_H
