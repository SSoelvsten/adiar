#ifndef ADIAR_INTERNAL_DD_H
#define ADIAR_INTERNAL_DD_H

#include <adiar/internal/cut.h>
#include <adiar/internal/data_types/arc.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/data_types/tuple.h>
#include <adiar/internal/io/arc_file.h>
#include <adiar/internal/io/node_file.h>

#include <variant>

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
  typedef std::monostate no_file;

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
    typedef node node_t;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the file object node-based representation of a diagram.
    ////////////////////////////////////////////////////////////////////////////
    typedef shared_levelized_file<node_t> shared_nodes_t;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of nodes of this diagram.
    ////////////////////////////////////////////////////////////////////////////
    typedef arc arc_t;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the file object arc-based representation of a diagram.
    ////////////////////////////////////////////////////////////////////////////
    typedef shared_levelized_file<arc_t> shared_arcs_t;

    ////////////////////////////////////////////////////////////////////////////
    // Union of levelized node or arc files to reflect the possible return types
    // of a function and a 'no_file' for 'error'.
    const std::variant<no_file, /*const*/ shared_nodes_t, /*const*/ shared_arcs_t>
    _union;

    ////////////////////////////////////////////////////////////////////////////
    // Propagating the negation flag below
    const bool negate = false;

    ////////////////////////////////////////////////////////////////////////////
    // Constructors
    __dd()
    { }

    __dd(const shared_nodes_t &f) : _union(f)
    { }

    __dd(const shared_arcs_t &f) : _union(f)
    { }

    __dd(const dd &dd);

    ////////////////////////////////////////////////////////////////////////////
    // Accessors
    // TODO: change from 'file_t' to 'elem_t'.
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
    typedef node node_t;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of pointers of this diagram.
    ////////////////////////////////////////////////////////////////////////////
    typedef node_t::ptr_t ptr_t;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of this node's variable label.
    ////////////////////////////////////////////////////////////////////////////
    typedef node_t::label_t label_t;

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
    /// \brief File type for the file object representing the diagram.
    ////////////////////////////////////////////////////////////////////////////
    typedef levelized_file<node_t> nodes_t;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief File type for the shared file object representing the diagram.
    ////////////////////////////////////////////////////////////////////////////
    typedef shared_file_ptr<nodes_t> shared_nodes_t;

    ////////////////////////////////////////////////////////////////////////////
    // Internal state
  protected:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief The file describing the actual DAG of the decision diagram.
    ////////////////////////////////////////////////////////////////////////////
    shared_nodes_t file; // TODO: shared_ptr<const levelized_file<node>>?

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
    dd(const shared_nodes_t &f, bool negate = false)
      : file(f), negate(negate)
    { }

    dd(const dd &dd)
      : file(dd.file), negate(dd.negate)
    { }

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
    const shared_nodes_t file_ptr() const
    {
      return file;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Read-only access to the members of the raw files and meta
    ///        information, i.e. this is similar to writing
    ///        `.file_ptr()->`.
    ////////////////////////////////////////////////////////////////////////////
    const nodes_t* operator->() const
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
    friend class __dd;

    template <bool REVERSE>
    friend class level_info_stream;

    template <bool REVERSE>
    friend class node_stream;

    template <typename file_t, typename comp_t, size_t FILES>
    friend class level_merger;

    friend bool is_isomorphic(const dd&, const dd&);

    template<typename comp_policy>
    friend bool comparison_check(const dd &in_1, const dd &in_2);

    friend bool is_terminal(const dd &dd);
    friend bool value_of(const dd &dd);
    friend label_t min_var(const dd &dd);
    friend label_t max_var(const dd &dd);

    template<typename to_policy, typename from_policy>
    friend class convert_dd_policy;
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
    /// \brief Type of pointers of this diagram.
    ////////////////////////////////////////////////////////////////////////////
    typedef typename node_t::children_t children_t;

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
    /// \brief Type of shared nodes for this diagram type.
    ////////////////////////////////////////////////////////////////////////////
    typedef typename __dd_type::shared_nodes_t shared_nodes_t;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of shared arcs for this diagram type.
    ////////////////////////////////////////////////////////////////////////////
    typedef typename __dd_type::shared_arcs_t shared_arcs_t;

    ////////////////////////////////////////////////////////////////////////////
    /// Function declaration
  public:
    static inline ptr_t
    reduction_rule(const node_t &n);

    static inline children_t
    reduction_rule_inv(const ptr_t &child);

    // TODO: stop using these in favour of 'reduction_rule_inv' above
    static inline void
    compute_cofactor(const bool on_curr_level,
                     ptr_t &low,
                     ptr_t &high);

    static inline children_t
    compute_cofactor(const bool on_curr_level,
                     const children_t &children);
  };
  /// \endcond
}

#endif // ADIAR_INTERNAL_DD_H
