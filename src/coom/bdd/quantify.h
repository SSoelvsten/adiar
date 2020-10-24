#ifndef COOM_QUANTIFY_H
#define COOM_QUANTIFY_H

#include <tpie/tpie.h>
#include <tpie/file_stream.h>

#include <stdint.h>

#include <coom/data.h>
#include <coom/file.h>

namespace coom
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Existentially quantify an OBDD for a variable with a given label.
  //////////////////////////////////////////////////////////////////////////////
  node_or_arc_file bdd_exists(const node_file &bdd, const label_t &label);

  node_file bdd_exists(const node_file &bdd, const label_file &labels);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Forall quantify an OBDD for a variable with a given label.
  //////////////////////////////////////////////////////////////////////////////
  node_or_arc_file bdd_forall(const node_file &bdd, const label_t &label);

  node_file bdd_forall(const node_file &bdd, const label_file &labels);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Uniquely quantify an OBDD for a variable with a given label.
  //////////////////////////////////////////////////////////////////////////////
  node_or_arc_file bdd_unique(const node_file &bdd, const label_t &label);

  node_file bdd_unique(const node_file &bdd, const label_file &labels);
}

#endif // COOM_QUANTIFY_H
