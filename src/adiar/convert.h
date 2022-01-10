#ifndef ADIAR_CONVERT_H
#define ADIAR_CONVERT_H

#include <adiar/bdd/bdd.h>
#include <adiar/zdd/zdd.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief     Obtains the BDD that represents the same function/set as the
  ///            given ZDD within the given domain.
  ///
  /// \param A   Family of a set (within the given domain)
  ///
  /// \param dom Domain of all variables (in ascending order)
  ///
  /// \returns   BDD that is true for the exact same assignments to variables in
  ///            the given domain.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_from(const zdd &A, const label_file &dom);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief     Obtains the ZDD that represents the same function/set as the
  ///            given BDD within the given domain.
  ///
  /// \param f   Boolean function with the given domain
  ///
  /// \param dom Domain of all variables (in ascending order)
  ///
  /// \returns   ZDD that is true for the exact same assignments to variables in
  ///            the given domain.
  //////////////////////////////////////////////////////////////////////////////
  __zdd zdd_from(const bdd &f, const label_file &dom);
}

#endif // ADIAR_CONVERT_H
