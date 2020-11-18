#ifndef COOM_QUANTIFY_H
#define COOM_QUANTIFY_H

#include <coom/data.h>
#include <coom/file.h>

#include <coom/bdd/bdd.h>

namespace coom
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Existentially quantify an OBDD for a variable with a given label.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_exists(const bdd &bdd, const label_t &label);

  bdd bdd_exists(const bdd &bdd, const label_file &labels);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Forall quantify an OBDD for a variable with a given label.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_forall(const bdd &bdd, const label_t &label);

  bdd bdd_forall(const bdd &bdd, const label_file &labels);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Uniquely quantify an OBDD for a variable with a given label.
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_unique(const bdd &bdd, const label_t &label);

  bdd bdd_unique(const bdd &bdd, const label_file &labels);
}

#endif // COOM_QUANTIFY_H
