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
  /// 2. Identifiers are from \ref max_id and down (when read bottom-up)
  ///
  /// If this is true, then equality checking can be done in a single cheap
  /// linear scan rather than with an *O(N log N)* time-forwarding algorithm.
  //////////////////////////////////////////////////////////////////////////////
  template <typename DD>
  bool dd_iscanonical(const DD &dd)
  {
    // TODO: Move into 'dd' class...
    return dd->canonical;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a given decision diagram represents a terminal.
  //////////////////////////////////////////////////////////////////////////////
  template <typename DD>
  bool dd_isterminal(const DD &dd)
  {
    // TODO: Move into 'dd' class...
    return dd->is_terminal();
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain the terminal's value (if 'is_terminal' is true).
  //////////////////////////////////////////////////////////////////////////////
  template <typename DD>
  bool dd_valueof(const DD &dd)
  {
    // TODO: Move into 'dd' class...
    return dd.negate ^ dd->value();
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a given decision diagram represents the false terminal.
  //////////////////////////////////////////////////////////////////////////////
  template <typename DD>
  bool dd_isfalse(const DD &dd)
  {
    // TODO: Move into 'dd' class...
    return dd_isterminal(dd) && !dd_valueof(dd);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a given decision diagram represents the true terminal.
  //////////////////////////////////////////////////////////////////////////////
  template <typename DD>
  bool dd_istrue(const DD &dd)
  {
    // TODO: Move into 'dd' class...
    return dd_isterminal(dd) && dd_valueof(dd);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get the minimal occurring label in the decision diagram
  //////////////////////////////////////////////////////////////////////////////
  template <typename DD>
  dd::label_type dd_minvar(const DD &dd)
  {
    // TODO: Exception if terminal.

    // TODO: Move into 'dd' class...
    return dd->first_level();
  }

  // TODO: dd_topvar(dd&) := dd_minvar(dd&)

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get the maximal occurring label in the decision diagram
  //////////////////////////////////////////////////////////////////////////////
  template <typename DD>
  dd::label_type dd_maxvar(const DD &dd)
  {
    // TODO: Exception if terminal.

    // TODO: Move into 'dd' class...
    return dd->last_level();
  }

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Number of nodes in a decision diagram.
  ////////////////////////////////////////////////////////////////////////////
  template <typename DD>
  size_t dd_nodecount(const DD &dd)
  {
    return dd_isterminal(dd) ? 0u : dd->size();
  }

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Number of variables, i.e. levels, present in a decision diagram.
  ////////////////////////////////////////////////////////////////////////////
  template <typename DD>
  typename DD::label_type dd_varcount(const DD &dd)
  {
    return dd->levels();
  }

  ////////////////////////////////////////////////////////////////////////////
  /// \brief The variable labels (in order of their level) that are present in a
  ///        decision diagram.
  ////////////////////////////////////////////////////////////////////////////
  template <typename DD>
  void dd_support(const DD &dd, const consumer<typename DD::label_type> &cb)
  {
    level_info_stream<> info_stream(dd);
    while(info_stream.can_pull()) { cb(info_stream.pull().label()); }
  }
}

#endif // ADIAR_INTERNAL_DD_FUNC_H
