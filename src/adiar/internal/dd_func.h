#ifndef ADIAR_INTERNAL_DD_FUNC_H
#define ADIAR_INTERNAL_DD_FUNC_H

#include <adiar/functional.h>

#include <adiar/internal/io/levelized_file_stream.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  // Collection of simple functions common to all types of decision diagrams.

  //////////////////////////////////////////////////////////////////////////////
  /// \brief  Check whether a given decision diagram is canonical.
  ///
  /// \details In Adiar, the word *canonical* refers to the following two
  /// stronger guarantees on the total ordering of nodes.
  ///
  /// 1. Nodes within a level are effectively sorted based on their children:
  ///    high first, then low.
  ///
  /// 2. Identifiers are from \ref MAX_ID and down (when read bottom-up)
  ///
  /// If this is true, then equality checking can be done in a single cheap
  /// linear scan rather than with an *O(N log N)* time-forwarding algorithm.
  //////////////////////////////////////////////////////////////////////////////
  template <typename dd_t>
  bool dd_iscanonical(const dd_t &dd)
  {
    // TODO: Move into 'dd' class...
    return dd->canonical;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a given decision diagram represents a terminal.
  //////////////////////////////////////////////////////////////////////////////
  template <typename dd_t>
  bool dd_isterminal(const dd_t &dd)
  {
    // TODO: Move into 'dd' class...
    return dd->is_terminal();
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain the terminal's value (if 'is_terminal' is true).
  //////////////////////////////////////////////////////////////////////////////
  template <typename dd_t>
  bool dd_valueof(const dd_t &dd)
  {
    // TODO: Move into 'dd' class...
    return dd.negate ^ dd->value();
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a given decision diagram represents the false terminal.
  //////////////////////////////////////////////////////////////////////////////
  template <typename dd_t>
  bool dd_isfalse(const dd_t &dd)
  {
    // TODO: Move into 'dd' class...
    return dd_isterminal(dd) && !dd_valueof(dd);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a given decision diagram represents the true terminal.
  //////////////////////////////////////////////////////////////////////////////
  template <typename dd_t>
  bool dd_istrue(const dd_t &dd)
  {
    // TODO: Move into 'dd' class...
    return dd_isterminal(dd) && dd_valueof(dd);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get the minimal occurring label in the decision diagram
  //////////////////////////////////////////////////////////////////////////////
  template <typename dd_t>
  dd::label_t dd_minvar(const dd_t &dd)
  {
    // TODO: Exception if terminal.

    // TODO: Move into 'dd' class...
    return dd->first_level();
  }

  // TODO: dd_topvar(dd&) := dd_minvar(dd&)

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get the maximal occurring label in the decision diagram
  //////////////////////////////////////////////////////////////////////////////
  template <typename dd_t>
  dd::label_t dd_maxvar(const dd_t &dd)
  {
    // TODO: Exception if terminal.

    // TODO: Move into 'dd' class...
    return dd->last_level();
  }

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Number of nodes in a decision diagram.
  ////////////////////////////////////////////////////////////////////////////
  template <typename dd_t>
  size_t dd_nodecount(const dd_t &dd)
  {
    return dd_isterminal(dd) ? 0u : dd->size();
  }

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Number of variables, i.e. levels, present in a decision diagram.
  ////////////////////////////////////////////////////////////////////////////
  template <typename dd_t>
  typename dd_t::label_t dd_varcount(const dd_t &dd)
  {
    return dd->levels();
  }

  ////////////////////////////////////////////////////////////////////////////
  /// \brief The variable labels (in order of their level) that are present in a
  ///        decision diagram.
  ////////////////////////////////////////////////////////////////////////////
  template <typename dd_t>
  void dd_varprofile(const dd_t &dd, const consumer<typename dd_t::label_t> &cb)
  {
    level_info_stream<> info_stream(dd);
    while(info_stream.can_pull()) { cb(info_stream.pull().label()); }
  }
}

#endif // ADIAR_INTERNAL_DD_FUNC_H
