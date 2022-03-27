#ifndef ADIAR_DEPRECATED_H
#define ADIAR_DEPRECATED_H

#include <adiar/file.h>

namespace adiar
{
  // LCOV_EXCL_START

  /* ======================= SORTING SIMPLE FILES ========================= */

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

  // LCOV_EXCL_STOP
}

#endif // ADIAR_DEPRECATED_H
