#ifndef ADIAR_ZDD_CHANGE_H
#define ADIAR_ZDD_CHANGE_H

#include <adiar/file.h>
#include <adiar/zdd/zdd.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Computes for a ZDD one where the values of the given labels are
  /// flipped.
  ///
  /// \param zdd     ZDD to apply with the other.
  ///
  /// \param ls      Labels that should be flipped
  //////////////////////////////////////////////////////////////////////////////
  __zdd zdd_change(const zdd &dd, const label_file &ls);
}

#endif // ADIAR_ZDD_CHANGE_H
