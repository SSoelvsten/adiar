#ifndef ADIAR_HOMOMORPHISM_H
#define ADIAR_HOMOMORPHISM_H

#include <adiar/data.h>
#include <adiar/file.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Given two node files, computes whether they are homomorphic; i.e.
  /// whether they are equivalent.
  ///
  /// Checks whether the two files are homomorphic, i.e. whether there is a
  /// structure-preserving mapping between f1 and f2. This assumes, that both
  /// files are of a unique reduced form.
  ///
  /// \param fi      The two files of nodes to compare.
  /// \param negatei Whether the nodes of fi should be read in negated form
  ///
  /// \return    Whether the two node_files represent equivalent graphs.
  //////////////////////////////////////////////////////////////////////////////
  bool is_homomorphic(const node_file &f1, const node_file &f2,
                      bool negate1 = false, bool negate2 = false);
}

#endif // ADIAR_HOMOMORPHISM_H
