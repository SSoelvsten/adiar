#ifndef ADIAR_NEGATE_H
#define ADIAR_NEGATE_H

#include <adiar/data.h>

#include <adiar/bdd/bdd.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Negate a given node-based OBDD.
  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_not(const bdd &bdd);
  bdd bdd_not(bdd &&bdd);
}

#endif // ADIAR_NEGATE_H
