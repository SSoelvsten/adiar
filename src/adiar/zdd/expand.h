#ifndef ADIAR_ZDD_EXPAND_H
#define ADIAR_ZDD_EXPAND_H

#include <adiar/file.h>
#include <adiar/zdd/zdd.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Expands the domain of the given ZDD to also include the given set
  /// of labels.
  ///
  /// \param dd     ZDD to expand
  ///
  /// \param ls     Labels of variables to expand with. This set of labels may
  ///               NOT occur in the given ZDD.
  //////////////////////////////////////////////////////////////////////////////
  __zdd zdd_expand(const zdd &dd, const label_file &ls);
}

#endif // ADIAR_ZDD_EXPAND_H
