#ifndef ADIAR_ZDD_COMPLEMENT_H
#define ADIAR_ZDD_COMPLEMENT_H

#include <adiar/data.h>
#include <adiar/zdd/zdd.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Computes the family of sets U \ a for all a in A.
  //////////////////////////////////////////////////////////////////////////////
  __zdd zdd_complement(const zdd &A, const label_file &U);
}

#endif // ADIAR_ZDD_COMPLEMENT_H
