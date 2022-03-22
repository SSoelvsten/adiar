#ifndef ADIAR_DEPRECATED_H
#define ADIAR_DEPRECATED_H

#include <adiar/file.h>

namespace adiar
{
  ////////////////////////////////////////////////////////////////////////////
  /// \brief Sorts the content of a <tt>simple_file</tt> given some sorting
  /// predicate.
  ////////////////////////////////////////////////////////////////////////////
  template<typename T, typename pred_t = std::less<>>
  [[deprecated("Use 'simple_file_sorter' in 'adiar/file.h'.")]]
  void sort(simple_file<T> f, pred_t pred = pred_t())
  {
    simple_file_sorter<T, pred_t>::sort(f, pred);
  }

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Return whether <tt>file</tt> represents a sink which satisfies the
  /// given predicate.
  ////////////////////////////////////////////////////////////////////////////
  [[deprecated("Use 'is_sink' and 'value_of' in 'adiar/file.h'.")]]
  inline bool is_sink(const node_file &file, const sink_pred &pred)
  {
    adiar_debug(!file.empty(), "Invalid node_file: empty");

    if (file.size() != 1) {
      return false;
    }
    node_stream<> ns(file);
    node_t n = ns.pull();

    return is_sink(n) && pred(n.uid);
  }

  [[deprecated("Use 'is_sink' and 'value_of' in 'adiar/file.h'.")]]
  inline bool is_sink(const decision_diagram &dd, const sink_pred &pred)
  {
    node_stream<> ns(dd);
    node_t n = ns.pull();

    return is_sink(n) && pred(n.uid);
  }
}

#endif // ADIAR_DEPRECATED_H
