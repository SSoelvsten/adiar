#ifndef ADIAR_INTERNAL_DD_H
#define ADIAR_INTERNAL_DD_H

#include <variant>

#include <adiar/internal/cut.h>
#include <adiar/internal/data_types/arc.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/arc_file.h>
#include <adiar/internal/io/node_file.h>

namespace adiar::internal
{
  // TODO (MDD):
  // TODO (QMDD):
  // TODO (ADD (64-bit)):
  //   template both 'dd' and '__dd' with the node type (and derive the
  //   corresponding arc type).

  class dd;

  //////////////////////////////////////////////////////////////////////////////
  /// A std::variant is used to distinguish the type of file. This uses
  /// std::monostate to hold a 'nothing' value, i.e. when there is no file.
  //////////////////////////////////////////////////////////////////////////////
  using no_file = std::monostate;

  //////////////////////////////////////////////////////////////////////////////
  /// \warning You should never explicitly be dealing with this class or have it
  /// be an l-value. Implicit conversion from an unreduced to the reduced type
  /// will call the correct reduce algorithm.
  ///
  /// An algorithm may return a node-based decision diagram in a
  /// \ref shared_levelized_file<node> or a yet to-be reduced decision diagram in
  /// in an \ref shared_levelized_file<arc>. So, we use a `std::variant` to hold
  /// the \ref shared_levelized_file<node> or \ref shared_levelized_file<arc>
  /// without having to pay for the expensive constructors and use a lot of space.
  ///
  /// A third possiblity is for it to contain a `std::monostate`, i.e. \ref
  /// no_file, such that an algorithm can return 'null' in some specific places.
  /// In most cases, this should be ignored and will otherwise lead to
  /// exceptions.
  //////////////////////////////////////////////////////////////////////////////
  class __dd
  {
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of nodes of this diagram.
    ////////////////////////////////////////////////////////////////////////////
    using node_type = node;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the file object node-based representation of a diagram.
    ////////////////////////////////////////////////////////////////////////////
    using shared_node_file_type = shared_levelized_file<node_type>;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of nodes of this diagram.
    ////////////////////////////////////////////////////////////////////////////
    using arc_type = arc;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the file object arc-based representation of a diagram.
    ////////////////////////////////////////////////////////////////////////////
    using shared_arc_file_type = shared_levelized_file<arc_type>;

    ////////////////////////////////////////////////////////////////////////////
    // Union of levelized node or arc files to reflect the possible return types
    // of a function and a 'no_file' for 'error'.
    std::variant<no_file, shared_node_file_type, shared_arc_file_type>
    _union;

    ////////////////////////////////////////////////////////////////////////////
    // Propagating the negation flag below
    bool negate = false;

    ////////////////////////////////////////////////////////////////////////////
    // Constructors
    __dd()
    { }

    __dd(const shared_node_file_type &f) 
      : _union(f)
    { }

    __dd(const shared_arc_file_type &f) 
      : _union(f)
    { }

    __dd(const dd &dd);

    ////////////////////////////////////////////////////////////////////////////
    // Accessors
    // TODO: change from 'file_t' to 'file::value_type'.
    template<typename file_t>
    bool has() const
    {
      return std::holds_alternative<file_t>(_union);
    }

    template<typename file_t>
    const file_t& get() const
    {
      return std::get<file_t>(_union);
    }

    bool empty() const
    {
      return has<no_file>();
    }

    // TODO (optimisation):
    //   Add precondition to be with 'arcs' only?

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of nodes.
    ////////////////////////////////////////////////////////////////////////////
    size_t size() const
    {
      if (has<shared_arc_file_type>()) {
        // TODO (QMDD): Divide by node::outdegree instead of 2u
        return get<shared_arc_file_type>()->size() / 2u;
      }
      if (has<shared_node_file_type>()) {
        return get<shared_node_file_type>()->size();
      }
      return 0u;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the 1-level cut of the desired type, i.e. of the sub-graph
    ///        including the desired type of arcs.
    ///
    /// \param ct The type of the cut to obtain
    ////////////////////////////////////////////////////////////////////////////
    cut_size_t max_1level_cut(const cut_type ct) const
    {
      if (has<shared_arc_file_type>()) {
        const shared_arc_file_type &af = get<shared_arc_file_type>();
        return af->max_1level_cut
          + (includes_terminal(ct, false) ? af->number_of_terminals[false] : 0u)
          + (includes_terminal(ct, true)  ? af->number_of_terminals[true]  : 0u);
      }
      if (has<shared_node_file_type>()) {
        return get<shared_node_file_type>()->max_1level_cut[ct];
      }
      return 0u;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the 2-level cut of the desired type, i.e. of the sub-graph
    ///        including the desired type of arcs.
    ///
    /// \param ct The type of the cut to obtain
    ////////////////////////////////////////////////////////////////////////////
    cut_size_t max_2level_cut(const cut_type ct) const
    {
      if (has<shared_arc_file_type>()) {
        const shared_arc_file_type &af = get<shared_arc_file_type>();
        return std::min(// 3/2 times the 1-level cut
                        (3 * af->max_1level_cut) / 2
                        + (includes_terminal(ct, false) ? af->number_of_terminals[false] : 0u)
                        + (includes_terminal(ct, true)  ? af->number_of_terminals[true]  : 0u),
                        // At most the number of nodes + 1
                        (af->size() / 2u) + 1);
      }
      if (has<shared_node_file_type>()) {
        return get<shared_node_file_type>()->max_2level_cut[ct];
      }
      return 0u;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of terminals of a certain value.
    ////////////////////////////////////////////////////////////////////////////
    size_t number_of_terminals(const bool value) const
    {
      if (has<shared_arc_file_type>()) {
        return get<shared_arc_file_type>()->number_of_terminals[negate ^ value];
      }
      if (has<shared_node_file_type>()) {
        return get<shared_node_file_type>()->number_of_terminals[negate ^ value];
      }
      return 0u;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of terminals.
    ////////////////////////////////////////////////////////////////////////////
    size_t number_of_terminals() const
    {
      if (has<shared_arc_file_type>()) {
        const shared_arc_file_type &af = get<shared_arc_file_type>();
        return af->number_of_terminals[false] + af->number_of_terminals[true];
      }
      if (has<shared_node_file_type>()) {
        const shared_node_file_type &nf = get<shared_node_file_type>();
        return nf->number_of_terminals[false] + nf->number_of_terminals[true];
      }
      return 0u;
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Container for the files that represent a Decision Diagram.
  ///
  /// \remark To ensure the most disk-space is available, try to garbage collect
  ///         objects of this type as quickly as possible and/or minimise the
  ///         number of lvalues of this type.
  //////////////////////////////////////////////////////////////////////////////
  class dd
  {
    ////////////////////////////////////////////////////////////////////////////
    // Constants
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of nodes of this diagram.
    ////////////////////////////////////////////////////////////////////////////
    using node_type = node;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of pointers of this diagram.
    ////////////////////////////////////////////////////////////////////////////
    using pointer_type = node_type::pointer_type;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of this node's variable label.
    ////////////////////////////////////////////////////////////////////////////
    using label_type = node_type::label_type;

    //////////////////////////////////////////////////////////////////////////////
    /// \brief The maximal possible value for a unique identifier's label.
    //////////////////////////////////////////////////////////////////////////////
    static constexpr label_type max_label = node_type::max_label;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of this node's level identifier.
    ////////////////////////////////////////////////////////////////////////////
    using id_type = node_type::id_type;

    //////////////////////////////////////////////////////////////////////////////
    /// \brief The maximal possible value for this nodes level identifier.
    //////////////////////////////////////////////////////////////////////////////
    static constexpr id_type max_id = node_type::max_id;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of a terminal value.
    ////////////////////////////////////////////////////////////////////////////
    using terminal_type = typename node_type::terminal_type;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief File type for the file object representing the diagram.
    ////////////////////////////////////////////////////////////////////////////
    using node_file_type = levelized_file<node_type>;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief File type for the shared file object representing the diagram.
    ////////////////////////////////////////////////////////////////////////////
    using shared_node_file_type = shared_file_ptr<node_file_type>;

    ////////////////////////////////////////////////////////////////////////////
    // Internal state
  protected:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief The file describing the actual DAG of the decision diagram.
    ////////////////////////////////////////////////////////////////////////////
    shared_node_file_type file;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Release the claim on the underlying file, thereby decreasing its
    ///        reference counter. If this is the sole owner of that file object,
    ///        then that object is destructed together with the physical files
    ///        on disk (if temporary).
    ////////////////////////////////////////////////////////////////////////////
    void deref()
    {
      file.reset();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether to negate the leaves when reading nodes from the file.
    ////////////////////////////////////////////////////////////////////////////
    bool negate = false; // TODO: move to 'bdd' or generalize to 'attribute'?

    ////////////////////////////////////////////////////////////////////////////
    // Constructors
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor to wrap the node-based result of an algorithm.
    ////////////////////////////////////////////////////////////////////////////
    dd(const shared_node_file_type &f, bool negate = false)
      : file(f), negate(negate)
    { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Copy construction, incrementing the reference count on the file
    ///        underneath.
    ////////////////////////////////////////////////////////////////////////////
    dd(const dd &dd)
      : file(dd.file), negate(dd.negate)
    { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Move construction, taking over ownership of the files underneath.
    ////////////////////////////////////////////////////////////////////////////
    dd(dd &&dd)
      : file(std::move(dd.file)), negate(std::move(dd.negate))
    { }

    ////////////////////////////////////////////////////////////////////////////
    // NOTE:
    //
    //   To implement the specific DD, add the following move-conversion that
    //   runs the Reduce algorithm.
    //
    // dd(__dd &&dd)
    ////////////////////////////////////////////////////////////////////////////

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
    const shared_node_file_type file_ptr() const
    {
      return file;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Read-only access to the members of the raw files and meta
    ///        information, i.e. this is similar to writing
    ///        `.file_ptr()->`.
    ////////////////////////////////////////////////////////////////////////////
    const node_file_type* operator->() const
    {
      return file_ptr().get();
    }

    /// \cond
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the 1-level cut of the desired type, i.e. of the sub-graph
    ///        including the desired type of arcs.
    ///
    /// \param ct The type of the cut to obtain
    ////////////////////////////////////////////////////////////////////////////
    cut_size_t max_1level_cut(const cut_type ct) const
    {
      return file->max_1level_cut[negate_cut_type(ct)];
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the 2-level cut of the desired type, i.e. of the sub-graph
    ///        including the desired type of arcs.
    ///
    /// \param ct The type of the cut to obtain
    ////////////////////////////////////////////////////////////////////////////
    cut_size_t max_2level_cut(const cut_type ct) const
    {
      return file->max_2level_cut[negate_cut_type(ct)];
    }
    /// \endcond

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The number of elements in the node file.
    ////////////////////////////////////////////////////////////////////////////
    size_t size() const
    {
      return file->size();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of terminals of a certain value.
    ////////////////////////////////////////////////////////////////////////////
    size_t number_of_terminals(const bool value) const
    {
      return file->number_of_terminals[negate ^ value];
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of terminals.
    ////////////////////////////////////////////////////////////////////////////
    size_t number_of_terminals() const
    {
      return number_of_terminals(false) + number_of_terminals(true);
    }

  private:
    cut_type negate_cut_type(const cut_type ct) const
    {
      if (!negate) { return ct; }

      switch (ct) {
      case cut_type::Internal_False:
        return cut_type::Internal_True;
      case cut_type::Internal_True:
        return cut_type::Internal_False;
      default:
        return ct;
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    // Friends
    // |- streaming classes
    friend class __dd;

    template <bool REVERSE>
    friend class level_info_stream;

    template <bool REVERSE>
    friend class node_stream;

    template <typename file_t, typename comp_t, size_t FILES, bool reverse>
    friend class level_merger;

    // |- algorithm functions and classes
    friend bool is_isomorphic(const dd&, const dd&);

    template<typename comp_policy>
    friend bool comparison_check(const dd &in_1, const dd &in_2);

    template<typename to_policy, typename from_policy>
    friend class convert_dd_policy;

    // |- public API
    template <typename dd_t>
    friend bool dd_isterminal(const dd_t &dd);

    template <typename dd_t>
    friend bool dd_valueof(const dd_t &dd);

    template <typename dd_t>
    friend label_type dd_minvar(const dd_t &dd);

    template <typename dd_t>
    friend label_type dd_maxvar(const dd_t &dd);
  };

  inline __dd::__dd(const dd &dd)
    : _union(dd.file), negate(dd.negate)
  { }

  /// \cond
  template<typename dd_type, typename __dd_type>
  class dd_policy
  {
    ////////////////////////////////////////////////////////////////////////////
    /// Constants
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the class representing the \em reduced decision diagram.
    ////////////////////////////////////////////////////////////////////////////
    // TODO: rename to 'dd_t'?
    using reduced_t = dd_type;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the class representing the \em possibly \em unreduced
    ///        decision diagram.
    ////////////////////////////////////////////////////////////////////////////
    // TODO: rename to '__dd_t'?
    using unreduced_t = __dd_type;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of nodes of this diagram.
    ////////////////////////////////////////////////////////////////////////////
    using node_type = typename dd_type::node_type;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of pointers of this diagram.
    ////////////////////////////////////////////////////////////////////////////
    using pointer_type = typename dd_type::pointer_type;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of pointers of this diagram.
    ////////////////////////////////////////////////////////////////////////////
    using children_type = typename node_type::children_type;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of this node's variable label.
    ////////////////////////////////////////////////////////////////////////////
    using label_type = typename dd_type::label_type;

    //////////////////////////////////////////////////////////////////////////////
    /// \brief The maximal possible value for a unique identifier's label.
    //////////////////////////////////////////////////////////////////////////////
    static constexpr label_type max_label = dd_type::max_label;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of this node's level identifier.
    ////////////////////////////////////////////////////////////////////////////
    using id_type = typename dd_type::id_type;

    //////////////////////////////////////////////////////////////////////////////
    /// \brief The maximal possible value for this nodes level identifier.
    //////////////////////////////////////////////////////////////////////////////
    static constexpr id_type max_id = dd_type::max_id;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of a terminal value.
    ////////////////////////////////////////////////////////////////////////////
    using terminal_type = typename dd_type::terminal_type;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of shared nodes for this diagram type.
    ////////////////////////////////////////////////////////////////////////////
    using shared_node_file_type = typename __dd_type::shared_node_file_type;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of shared arcs for this diagram type.
    ////////////////////////////////////////////////////////////////////////////
    using shared_arc_file_type = typename __dd_type::shared_arc_file_type;

    ////////////////////////////////////////////////////////////////////////////
    /// Function declaration
  public:
    static inline pointer_type
    reduction_rule(const node_type &n);

    static inline children_type
    reduction_rule_inv(const pointer_type &child);

    // TODO: stop using these in favour of 'reduction_rule_inv' above
    static inline void
    compute_cofactor(const bool on_curr_level,
                     pointer_type &low,
                     pointer_type &high);

    static inline children_type
    compute_cofactor(const bool on_curr_level,
                     const children_type &children);
  };
  /// \endcond
}

#endif // ADIAR_INTERNAL_DD_H
