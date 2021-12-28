#ifndef ADIAR_CONVERT_H
#define ADIAR_CONVERT_H

#include <adiar/bdd/bdd.h>
#include <adiar/zdd/zdd.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Obtains the BDD from a ZDD that represents the same function/set
  /// within the given domain.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_from(const zdd &dd, const label_file &dom);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Obtains the ZDD from a BDD that represents the same function/set
  /// within the given domain.
  //////////////////////////////////////////////////////////////////////////////
  __zdd zdd_from(const bdd &dd, const label_file &dom);
}

#endif // ADIAR_CONVERT_H
