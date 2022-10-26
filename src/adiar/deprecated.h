#ifndef ADIAR_DEPRECATED_H
#define ADIAR_DEPRECATED_H

#include <adiar/file.h>

namespace adiar
{
  // LCOV_EXCL_START

  /* ======================== SORTING SIMPLE FILES ========================== */

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Sorts the content of a <tt>simple_file</tt> given some sorting
  /// predicate.
  //////////////////////////////////////////////////////////////////////////////
  template<typename T, typename pred_t = std::less<>>
  [[deprecated("Use 'simple_file_sorter' in 'adiar/file.h'.")]]
  void sort(simple_file<T> f, pred_t pred = pred_t())
  {
    simple_file_sorter<T, pred_t>::sort(f, pred);
  }

  /* ================== UNIQUE IDENTIFIERS : SINK NODES====================== */

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a pointer is for a sink.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by is_terminal in 'adiar/internal/data_types/uid.h'")]]
  inline bool is_sink(ptr_t p)
  { return is_terminal(p); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create the unique identifier for a sink with the given value.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by create_terminal_uid in 'adiar/internal/data_types/uid.h'")]]
  inline uid_t create_sink_uid(bool v)
  { return create_terminal_uid(v); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create a pointer to a sink with the given value.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by create_terminal_ptr in 'adiar/internal/data_types/uid.h'")]]
  inline ptr_t create_sink_ptr(bool v)
  { return create_terminal_ptr(v); }

  /* ================================ NODES ================================= */

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create a sink node representing the given boolean value.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by node constructor in 'adiar/internal/data_types/node.h'")]]
  inline node_t create_sink(bool value)
  { return node(value); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create a sink node representing the given boolean value.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by node constructor in 'adiar/internal/data_types/node.h'")]]
  inline node_t create_terminal(bool value)
  { return node(value); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create a node with the given unique identifier (label, id) and a
  ///        pointer to its two children.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by node constructor in 'adiar/internal/data_types/node.h'")]]
  inline node_t create_node(uid_t uid, ptr_t low, ptr_t high)
  { return node(uid, low, high); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a node is a terminal.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by member function with same name in 'adiar/internal/data_types/node.h'")]]
  inline bool is_terminal(const node &n)
  { return n.is_terminal(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain the value of a terminal node
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by member function 'value' in 'adiar/internal/data_types/node.h'")]]
  inline bool value_of(const node &n)
  { return n.value(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a terminal node is 'false'
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by member function with same name in 'adiar/internal/data_types/node.h'")]]
  inline bool is_false(const node &n)
  { return n.is_false(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a terminal node is 'true'
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by member function with same name in 'adiar/internal/data_types/node.h'")]]
  inline bool is_true(const node &n)
  { return n.is_true(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain the label of a non-terminal node.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by member function 'label' in 'adiar/internal/data_types/node.h'")]]
  inline bool label_of(const node &n)
  { return n.label(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain the id of a non-terminal node.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by member function 'label' in 'adiar/internal/data_types/node.h'")]]
  inline bool id_of(const node &n)
  { return n.id(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain the id of a non-terminal node.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by member function with same name in 'adiar/internal/data_types/node.h'")]]
  inline bool on_level(const node &n, label_t lvl)
  { return n.on_level(lvl); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create a node with the given its label, its level identifier, and a
  ///        pointer to its two children.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by node constructor in 'adiar/internal/data_types/node.h'")]]
  inline node_t create_node(label_t label, id_t id, ptr_t low, ptr_t high)
  { return node(create_node_uid(label, id), low, high); }

  [[deprecated("Replaced by node constructor in 'adiar/internal/data_types/node.h'")]]
  inline node_t create_node(label_t label, id_t id, ptr_t low, node_t high)
  { return node(label, id, low, high.uid()); }

  [[deprecated("Replaced by node constructor in 'adiar/internal/data_types/node.h'")]]
  inline node_t create_node(label_t label, id_t id, node_t low, ptr_t high)
  { return node(label, id, low.uid(), high); }

  [[deprecated("Replaced by node constructor in 'adiar/internal/data_types/node.h'")]]
  inline node_t create_node(label_t label, id_t id, node_t low, node_t high)
  { return node(label, id, low.uid(), high.uid()); }

  /* =========================== DECISION DIAGRAM =========================== */

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a given decision diagram represents a constant value.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by is_terminal in 'adiar/internal/decision_diagram.h'")]]
  inline bdd is_sink(const decision_diagram& dd)
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

  // LCOV_EXCL_STOP
}

#endif // ADIAR_DEPRECATED_H
