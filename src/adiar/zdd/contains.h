#ifndef ADIAR_ZDD_CONTAINS_H
#define ADIAR_ZDD_CONTAINS_H

#include <adiar/data.h>
#include <adiar/file.h>

#include <adiar/zdd/zdd.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Decide whether the ZDD contains the given set of labels
  //////////////////////////////////////////////////////////////////////////////
  bool zdd_contains(const zdd &dd, const label_file &ls);
}

#endif // ADIAR_ZDD_CONTAINS_H
