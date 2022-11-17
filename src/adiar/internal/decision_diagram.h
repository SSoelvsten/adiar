#ifndef ADIAR_INTERNAL_DECISION_DIAGRAM_H
#define ADIAR_INTERNAL_DECISION_DIAGRAM_H

#include <adiar/file.h>

#include <adiar/internal/cut.h>
#include <adiar/internal/data_types/arc.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/data_types/tuple.h>

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
  /// \ref node_file or a yet to-be reduced decision diagram in an
  /// \ref arc_file. So, we use a `std::variant` to hold the the
  /// \ref node_file or \ref arc_file without having to pay for the
  /// expensive constructors and use a lot of space. The implicit
  ///
  /// A third possiblity is for it to contain a `std::monostate`, i.e. \ref
  /// no_file, such that an algorithm can return 'null' in some specific places.
  /// In most cases, this should be ignored and will otherwise lead to
  /// exceptions.
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
    // Constants
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of nodes of this diagram.
    ////////////////////////////////////////////////////////////////////////////
    typedef node node_t;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of pointers of this diagram.
    ////////////////////////////////////////////////////////////////////////////
    typedef node_t::ptr_t ptr_t;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of this node's variable label.
    ////////////////////////////////////////////////////////////////////////////
    typedef typename node_t::label_t label_t;

    //////////////////////////////////////////////////////////////////////////////
    /// \brief The maximal possible value for a unique identifier's label.
    //////////////////////////////////////////////////////////////////////////////
    static constexpr label_t MAX_LABEL = node_t::MAX_LABEL;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of this node's level identifier.
    ////////////////////////////////////////////////////////////////////////////
    typedef node_t::id_t id_t;

    //////////////////////////////////////////////////////////////////////////////
    /// \brief The maximal possible value for this nodes level identifier.
    //////////////////////////////////////////////////////////////////////////////
    static constexpr id_t MAX_ID = node_t::MAX_ID;

    ////////////////////////////////////////////////////////////////////////////
    // Internal state
  protected:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief The file describing the actual DAG of the decision diagram.
    ////////////////////////////////////////////////////////////////////////////
    node_file file;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether to negate the leaves when reading nodes from the file.
    ////////////////////////////////////////////////////////////////////////////
    bool negate = false;

    ////////////////////////////////////////////////////////////////////////////
    /// \internal
    ///
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
    const std::shared_ptr<const __meta_file<node>> file_ptr() const
    {
      return file._file_ptr;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Read-only access to the members of the raw files and meta
    ///        information, i.e. this is similar to writing
    ///        `.file_ptr()->`.
    ////////////////////////////////////////////////////////////////////////////
    const __meta_file<node>* operator->() const
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

    friend bool is_terminal(const decision_diagram &dd);
    friend bool value_of(const decision_diagram &dd);
    friend label_t min_label(const decision_diagram &dd);
    friend label_t max_label(const decision_diagram &dd);

    template<typename to_policy, typename from_policy>
    friend class convert_decision_diagram_policy;
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
  /// - Identifiers are from \ref MAX_ID and down (when read bottom-up)
  ///
  /// If this is true, then equality checking can be done in a single cheap
  /// linear scan rather than with an *O(N log N)* time-forwarding algorithm.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_canonical(const decision_diagram &dd)
  {
    return dd->canonical;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a given decision diagram represents a terminal.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_terminal(const decision_diagram &dd)
  {
    return is_terminal(dd.file);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain the terminal's value (if 'is_terminal' is true).
  //////////////////////////////////////////////////////////////////////////////
  inline bool value_of(const decision_diagram &dd)
  {
    return dd.negate ^ value_of(dd.file);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a given decision diagram represents the false terminal.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_false(const decision_diagram &dd)
  {
    return is_terminal(dd) && !value_of(dd);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a given decision diagram represents the true terminal.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_true(const decision_diagram &dd)
  {
    return is_terminal(dd) && value_of(dd);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get the minimal occurring label in the decision diagram
  //////////////////////////////////////////////////////////////////////////////
  inline decision_diagram::label_t min_label(const decision_diagram &dd)
  {
    return min_label(dd.file);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get the maximal occurring label in the decision diagram
  //////////////////////////////////////////////////////////////////////////////
  inline decision_diagram::label_t max_label(const decision_diagram &dd)
  {
    return max_label(dd.file);
  }


  template<typename dd_type, typename __dd_type>
  class decision_diagram_policy
  {
    ////////////////////////////////////////////////////////////////////////////
    /// Constants
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the class representing the \em reduced decision diagram.
    ////////////////////////////////////////////////////////////////////////////
    // TODO: rename to 'dd_t'?
    typedef dd_type reduced_t;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the class representing the \em possibly \em unreduced
    ///        decision diagram.
    ////////////////////////////////////////////////////////////////////////////
    // TODO: rename to '__dd_t'?
    typedef __dd_type unreduced_t;

    ////////////////////////////////////////////////////////////////////////////
    // Constants
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of nodes of this diagram.
    ////////////////////////////////////////////////////////////////////////////
    typedef typename dd_type::node_t node_t;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of pointers of this diagram.
    ////////////////////////////////////////////////////////////////////////////
    typedef typename dd_type::ptr_t ptr_t;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of this node's variable label.
    ////////////////////////////////////////////////////////////////////////////
    typedef typename dd_type::label_t label_t;

    //////////////////////////////////////////////////////////////////////////////
    /// \brief The maximal possible value for a unique identifier's label.
    //////////////////////////////////////////////////////////////////////////////
    static constexpr label_t MAX_LABEL = dd_type::MAX_LABEL;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of this node's level identifier.
    ////////////////////////////////////////////////////////////////////////////
    typedef typename dd_type::id_t id_t;

    //////////////////////////////////////////////////////////////////////////////
    /// \brief The maximal possible value for this nodes level identifier.
    //////////////////////////////////////////////////////////////////////////////
    static constexpr id_t MAX_ID = dd_type::MAX_ID;

    ////////////////////////////////////////////////////////////////////////////
    /// Function declaration
  public:
    static inline ptr_uint64
    reduction_rule(const typename dd_type::node_t &n);

    // TODO: replace 'tuple' with 2-ary array.
    static inline tuple<ptr_uint64>
    reduction_rule_inv(const typename dd_type::ptr_t &child);

    static inline void
    compute_cofactor(bool on_curr_level,
                     typename dd_type::ptr_t &low,
                     typename dd_type::ptr_t &high);
  };
}

#endif // ADIAR_INTERNAL_DECISION_DIAGRAM_H
