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
  [[deprecated("Replaced by is_terminal in 'adiar/data.h'")]]
  inline bool is_sink(ptr_t p)
  { return is_terminal(p); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create the unique identifier for a sink with the given value.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by create_terminal_uid in 'adiar/data.h'")]]
  inline uid_t create_sink_uid(bool v)
  { return create_terminal_uid(v); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create a pointer to a sink with the given value.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by create_terminal_ptr in 'adiar/data.h'")]]
  inline ptr_t create_sink_ptr(bool v)
  { return create_terminal_ptr(v); }

  /* ================================ NODES ================================= */

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create a sink node representing the given boolean value.
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by create_terminal in 'adiar/data.h'")]]
  inline node_t create_sink(bool value)
  { return create_terminal(value); }

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
  /// \brief       The ZDD of only a single sink.
  ///
  /// \param value The constant sink value.
  ///
  /// \sa          zdd_empty zdd_null
  //////////////////////////////////////////////////////////////////////////////
  [[deprecated("Replaced by zdd_terminal in 'adiar/zdd.h'")]]
  inline zdd zdd_sink(bool value)
  { return zdd_terminal(value); }

  // LCOV_EXCL_STOP
}

#endif // ADIAR_DEPRECATED_H
