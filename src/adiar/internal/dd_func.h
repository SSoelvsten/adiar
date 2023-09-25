#ifndef ADIAR_INTERNAL_DD_FUNC_H
#define ADIAR_INTERNAL_DD_FUNC_H

#include <adiar/internal/dd.h>
#include <adiar/internal/io/levelized_file_stream.h>
#include <adiar/internal/io/file.h>
#include <adiar/internal/io/file_writer.h>

namespace adiar::internal
{
  ////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain the level_info stream projected onto the labels.
  ////////////////////////////////////////////////////////////////////////////
  template <typename dd_t>
  void dd_varprofile(const dd_t &dd, const consumer<typename dd_t::label_t> &cb)
  {
    level_info_stream<> info_stream(dd);
    while(info_stream.can_pull()) { cb(info_stream.pull().label()); }
  }

  // TODO: move into 'dd' class?

  //////////////////////////////////////////////////////////////////////////////
  /// Check whether a given decision diagram is canonical, i.e. has the
  /// following stronger guarantees than the total ordering of nodes.
  ///
  /// 1. Nodes within a level are effectively sorted based on their children:
  ///    high first, then low.
  ///
  /// 2. Identifiers are from \ref MAX_ID and down (when read bottom-up)
  ///
  /// If this is true, then equality checking can be done in a single cheap
  /// linear scan rather than with an *O(N log N)* time-forwarding algorithm.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_canonical(const dd &dd)
  {
    return dd->canonical;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a given decision diagram represents a terminal.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_terminal(const dd &dd)
  {
    return dd->is_terminal();
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain the terminal's value (if 'is_terminal' is true).
  //////////////////////////////////////////////////////////////////////////////
  inline bool value_of(const dd &dd)
  {
    return dd.negate ^ dd->value();
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a given decision diagram represents the false terminal.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_false(const dd &dd)
  {
    return is_terminal(dd) && !value_of(dd);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a given decision diagram represents the true terminal.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_true(const dd &dd)
  {
    return is_terminal(dd) && value_of(dd);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get the minimal occurring label in the decision diagram
  //////////////////////////////////////////////////////////////////////////////
  inline dd::label_t min_var(const dd &dd)
  {
    return dd->first_level();
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get the maximal occurring label in the decision diagram
  //////////////////////////////////////////////////////////////////////////////
  inline dd::label_t max_var(const dd &dd)
  {
    return dd->last_level();
  }
}

#endif // ADIAR_INTERNAL_DD_FUNC_H
