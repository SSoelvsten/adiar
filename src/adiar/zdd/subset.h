#ifndef ADIAR_ZDD_SUBSET_H
#define ADIAR_ZDD_SUBSET_H

#include <adiar/data.h>
#include <adiar/zdd/zdd.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain the family A' of all a in A (dd) where a does not contain
  /// any of the mentioned labels (l)
  //////////////////////////////////////////////////////////////////////////////
  __zdd zdd_offset(const zdd &dd, const label_file &l);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain the family A' of all a in A (dd) where a does contain all of
  /// the mentioned labels (l)
  //////////////////////////////////////////////////////////////////////////////
  __zdd zdd_onset(const zdd &dd, const label_file &l);
}

#endif // ADIAR_ZDD_SUBSET_H
