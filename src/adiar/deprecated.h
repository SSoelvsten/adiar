#ifndef ADIAR_DEPRECATED_H
#define ADIAR_DEPRECATED_H

namespace adiar
{
  // LCOV_EXCL_START

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Variable label.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Use 'bdd::label_t' or 'zdd::label_t' instead")]]
  typedef internal::ptr_uint64::label_t label_t;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The maximal possible value for a unique identifier's label.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Use 'bdd::MAX_LABEL' or 'zdd::MAX_LABEL' instead")]]
  constexpr internal::ptr_uint64::label_t MAX_LABEL = internal::ptr_uint64::MAX_LABEL;

  /* ============================= ASSIGNMENT =============================== */

  //////////////////////////////////////////////////////////////////////////////
  /// \brief A file of labels.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Use 'shared_file<label_t>' in 'adiar/file.h' instead")]]
  typedef shared_file<internal::ptr_uint64::label_t> label_file;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Level identifier.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Use 'bdd::id_t' or 'zdd::id_t' instead")]]
  typedef internal::ptr_uint64::id_t id_t;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The maximal possible value for a unique identifier's id.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Use 'bdd::MAX_ID' or 'zdd::MAX_ID' instead")]]
  constexpr internal::ptr_uint64::id_t MAX_ID = internal::ptr_uint64::MAX_ID;

  /* ======================== SORTING SIMPLE FILES ========================== */

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Sorts the content of a <tt>shared_file_ptr</tt> given some sorting
  ///        predicate.
  //////////////////////////////////////////////////////////////////////////////
  template<typename elem_t, typename pred_t = std::less<>>
  [[deprecated("Use 'f->sort()' in 'adiar/file.h'.")]]
  void sort(internal::shared_file_ptr<internal::file<elem_t>> f, pred_t pred = pred_t())
  { f->sort(pred); }

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Provides sorting for <tt>shared_file_ptr</tt>.
  ////////////////////////////////////////////////////////////////////////////
  template <typename elem_t, typename pred_t = std::less<elem_t>>
  class [[deprecated("Use 'f->sort()' in 'adiar/file.h'.")]] simple_file_sorter
  {
  public:
    static void sort(internal::shared_file_ptr<internal::file<elem_t>> f, pred_t pred = pred_t())
    { f->sort(pred); }
  };

  /* =============== UNIQUE IDENTIFIERS : INTERNAL NODES ==================== */

  [[deprecated("Use the 'ptr' class type directly 'adiar/internal/data_types/ptr.h'")]]
  typedef internal::ptr_uint64 ptr_t;

  [[deprecated("Use the '__uid<ptr_uint64>' class directly 'adiar/internal/data_types/ptr.h'")]]
  typedef internal::uid_uint64 uid_t;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create a pointer to the uid = (label, id).
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by constructor in 'adiar/internal/data_types/ptr.h'")]]
  inline internal::ptr_uint64 create_node_ptr(internal::ptr_uint64::label_t label, internal::ptr_uint64::id_t id)
  { return internal::ptr_uint64(label, id); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create a pointer to the given uid.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by constructor in 'adiar/internal/data_types/ptr.h'")]]
  inline internal::ptr_uint64 create_node_ptr(internal::node::uid_t uid)
  { return uid; }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Extract the label from a uid = (label, id).
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by 'label()' member function in 'adiar/internal/data_types/ptr.h'")]]
  inline internal::ptr_uint64::label_t label_of(internal::ptr_uint64 p)
  { return p.label(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Extract the level identifier from a uid = (label, id).
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by 'label()' member function in 'adiar/internal/data_types/ptr.h'")]]
  inline internal::ptr_uint64::id_t id_of(internal::ptr_uint64 p)
  { return p.id(); }

  /* =============== UNIQUE IDENTIFIERS : TERMINAL NODES ==================== */

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a pointer is for a sink.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by 'is_terminal()' member function in 'adiar/internal/data_types/ptr.h'")]]
  inline bool is_sink(internal::ptr_uint64 p)
  { return p.is_terminal(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a pointer is for a terminal.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by 'is_terminal()' member function in 'adiar/internal/data_types/ptr.h'")]]
  inline bool is_terminal(internal::ptr_uint64 p)
  { return p.is_terminal(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create the unique identifier for a sink with the given value.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by constructor in 'adiar/internal/data_types/uid.h'")]]
  inline internal::__uid<internal::ptr_uint64> create_sink_uid(bool v)
  { return internal::__uid<internal::ptr_uint64>(v); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create the uid = (label, id).
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by constructor in 'adiar/internal/data_types/uid.h'")]]
  inline internal::__uid<internal::ptr_uint64> create_node_uid(internal::ptr_uint64::label_t label,
                                                               internal::ptr_uint64::id_t id)
  { return internal::__uid<internal::ptr_uint64>(label, id); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create the unique identifier for a terminal with the given value.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by constructor in 'adiar/internal/data_types/uid.h'")]]
  inline internal::__uid<internal::ptr_uint64> create_terminal_uid(bool v)
  { return internal::__uid<internal::ptr_uint64>(v); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create a pointer to a sink with the given value.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by 'ptr(v)' constructor in 'adiar/internal/data_types/ptr.h'")]]
  inline internal::ptr_uint64 create_sink_ptr(bool v)
  { return internal::ptr_uint64(v); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create a pointer to a terminal with the given value.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by 'ptr(v)' constructor in 'adiar/internal/data_types/ptr.h'")]]
  inline internal::ptr_uint64 create_terminal_ptr(bool v)
  { return internal::ptr_uint64(v); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Extract the value from a terminal.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by 'value()' member function in 'adiar/internal/data_types/ptr.h'")]]
  inline bool value_of(internal::ptr_uint64 p)
  { return p.value(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the pointer points to the `false` terminal.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by 'is_false()' member function in 'adiar/internal/data_types/ptr.h'")]]
  inline bool is_false(internal::ptr_uint64 p)
  { return p.is_false(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the pointer points to the `true` terminal.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by 'is_true()' member function in 'adiar/internal/data_types/ptr.h'")]]
  inline bool is_true(internal::ptr_uint64 p)
  { return p.is_true(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a pointer is for a node on a given level.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by 'on_level(level)' member function in 'adiar/internal/data_types/ptr.h'")]]
  inline bool on_level(internal::ptr_uint64 p, internal::ptr_uint64::label_t level)
  { return p.on_level(level); }

  /* ================================ NODES ================================= */

  [[deprecated("Use the 'node' class type in 'adiar/internal/data_types/node.h' directly")]]
  //////////////////////////////////////////////////////////////////////////////
  /// \copydoc node
  //////////////////////////////////////////////////////////////////////////////
  typedef internal::node node_t;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create a sink node representing the given boolean value.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by node constructor in 'adiar/internal/data_types/node.h'")]]
  inline internal::node create_sink(bool value)
  { return internal::node(value); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create a sink node representing the given boolean value.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by node constructor in 'adiar/internal/data_types/node.h'")]]
  inline internal::node create_terminal(bool value)
  { return internal::node(value); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create a node with the given unique identifier (label, id) and a
  ///        pointer to its two children.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by node constructor in 'adiar/internal/data_types/node.h'")]]
  inline internal::node create_node(internal::node::uid_t uid,
                                    internal::ptr_uint64 low,
                                    internal::ptr_uint64 high)
  { return internal::node(uid, low, high); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a node is a terminal.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by member function with same name in 'adiar/internal/data_types/node.h'")]]
  inline bool is_terminal(const internal::node &n)
  { return n.is_terminal(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain the value of a terminal node
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by member function 'value' in 'adiar/internal/data_types/node.h'")]]
  inline bool value_of(const internal::node &n)
  { return n.value(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a terminal node is 'false'
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by member function with same name in 'adiar/internal/data_types/node.h'")]]
  inline bool is_false(const internal::node &n)
  { return n.is_false(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a terminal node is 'true'
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by member function with same name in 'adiar/internal/data_types/node.h'")]]
  inline bool is_true(const internal::node &n)
  { return n.is_true(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain the label of a non-terminal node.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by member function 'label' in 'adiar/internal/data_types/node.h'")]]
  inline internal::node::label_t label_of(const internal::node &n)
  { return n.label(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain the id of a non-terminal node.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by member function 'label' in 'adiar/internal/data_types/node.h'")]]
  inline internal::node::id_t id_of(const internal::node &n)
  { return n.id(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain the id of a non-terminal node.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by member function with same name in 'adiar/internal/data_types/node.h'")]]
  inline bool on_level(const internal::node &n, internal::node::label_t lvl)
  { return n.on_level(lvl); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create a node with the given its label, its level identifier, and a
  ///        pointer to its two children.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by node constructor in 'adiar/internal/data_types/node.h'")]]
  inline internal::node create_node(internal::node::label_t label, internal::node::id_t id, internal::ptr_uint64 low, internal::ptr_uint64 high)
  { return internal::node(label, id, low, high); }

  [[deprecated("Replaced by node constructor in 'adiar/internal/data_types/node.h'")]]
  inline internal::node create_node(internal::node::label_t label, internal::node::id_t id, internal::ptr_uint64 low, internal::node high)
  { return internal::node(label, id, low, high.uid()); }

  [[deprecated("Replaced by node constructor in 'adiar/internal/data_types/node.h'")]]
  inline internal::node create_node(internal::node::label_t label, internal::node::id_t id, internal::node low, internal::ptr_uint64 high)
  { return internal::node(label, id, low.uid(), high); }

  [[deprecated("Replaced by node constructor in 'adiar/internal/data_types/node.h'")]]
  inline internal::node create_node(internal::node::label_t label, internal::node::id_t id, internal::node low, internal::node high)
  { return internal::node(label, id, low.uid(), high.uid()); }

  //////////////////////////////////////////////////////////////////////////////
  /// \copydoc adiar::internal::node::operator~
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by '~' operator in 'adiar/internal/data_types/node.h'")]]
  inline internal::node operator! (const internal::node &n)
  { return ~n; }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Files of arcs to represent an unreduced decision diagram.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Use the 'shared_nodes_t' carried by the 'dd' class in 'adiar/internal/dd.h'")]]
  typedef internal::shared_levelized_file<internal::node> node_file;

  /* ================================= ARC ================================== */

  [[deprecated("Use the 'arc' class type in 'adiar/internal/data_types/arc.h' directly")]]
  //////////////////////////////////////////////////////////////////////////////
  /// \copydoc arc
  //////////////////////////////////////////////////////////////////////////////
  typedef internal::arc arc_t;

  [[deprecated("Replaced by member function in 'adiar/internal/data_types/arc.h'")]]
  inline internal::node is_high(const internal::arc &a)
  { return a.source().out_idx(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \copydoc adiar::internal::arc::operator~
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced with '~' operator in 'adiar/internal/data_types/arc.h'")]]
  inline internal::arc operator! (const internal::arc &a)
  { return negate(a); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Files of arcs to represent an unreduced decision diagram.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Use the 'shared_arcs_t' carried by the '__dd' class in 'adiar/internal/dd.h'")]]
  typedef internal::shared_levelized_file<internal::arc> arc_file;

  /* ============================= ASSIGNMENT =============================== */

  //////////////////////////////////////////////////////////////////////////////
  /// \brief A file of assignments (label, value)
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Use 'shared_file<assignment>' in 'adiar/file.h' instead")]]
  typedef shared_file<map_pair<bdd::label_t, boolean>> assignment_file;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Negate the value of an assignment [label -> value].
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced with '~' operator in 'adiar/assignment.h'")]]
  inline map_pair<bdd::label_t, boolean>
  operator! (const map_pair<bdd::label_t, boolean> &a)
  { return ~a; }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Extract the label from an assignment [label -> value].
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by member function in 'adiar/assignment.h'")]]
  inline bdd::label_t label_of(const map_pair<bdd::label_t, boolean> &a)
  { return a.key(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Extract the value from an assignment [label -> value].
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by member function in 'adiar/assignment.h'")]]
  inline bool value_of(const map_pair<bdd::label_t, boolean> &a)
  { return a.is_true(); }

  /* =========================== DECISION DIAGRAM =========================== */

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a given decision diagram represents a constant value.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by is_terminal in 'adiar/internal/dd.h'")]]
  inline bdd is_sink(const internal::dd& dd)
  { return is_terminal(dd); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief       The BDD representing a constant value.
  ///
  /// \param value The constant sink value
  ///
  /// \sa          bdd_false bdd_true
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by bdd_terminal in 'adiar/bdd.h'")]]
  inline bdd bdd_sink(bool value)
  { return bdd_terminal(value); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Output a DOT drawing a decision diagram to an output stream or a
  ///        file with the given file name.
  ///
  /// \sa    bdd_printdot
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by bdd_printdot in 'adiar/bdd.h'")]]
  inline void output_dot(const bdd &f, const std::string &file_name)
  { return bdd_printdot(f, file_name); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief       The ZDD of only a single sink.
  ///
  /// \param value The constant sink value.
  ///
  /// \sa          zdd_empty zdd_null
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by zdd_terminal in 'adiar/zdd.h'")]]
  inline zdd zdd_sink(bool value)
  { return zdd_terminal(value); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Output a DOT drawing of a ZDD to the file with the given name.
  ///
  /// \sa zdd_printdot
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by zdd_printdot in 'adiar/zdd.h'")]]
  inline void output_dot(const zdd &A, const std::string &file_name)
  { return zdd_printdot(A, file_name); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get the minimal occurring variable in this BDD.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by min_var in 'adiar/bdd.h'")]]
  inline bdd::label_t min_label(const bdd &f)
  { return min_var(f); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get the maximal occurring variable in this BDD.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by max_var in 'adiar/bdd.h'")]]
  inline bdd::label_t max_label(const bdd &f)
  { return max_var(f); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get the minimal occurring variable in this BDD.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by min_var in 'adiar/zdd.h'")]]
  inline zdd::label_t min_label(const zdd &A)
  { return min_var(A); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get the maximal occurring variable in this BDD.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by max_var in 'adiar/zdd.h'")]]
  inline zdd::label_t max_label(const zdd &A)
  { return max_var(A); }

  // LCOV_EXCL_STOP
}

#endif // ADIAR_DEPRECATED_H
