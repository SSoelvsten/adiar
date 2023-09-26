#ifndef ADIAR_INTERNAL_CUT_H
#define ADIAR_INTERNAL_CUT_H

#include <array>
#include <stddef.h>
#include <limits>

#include <adiar/internal/assert.h>

namespace adiar::internal
{
  ////////////////////////////////////////////////////////////////////////////
  /// \brief Type for a cut size
  ////////////////////////////////////////////////////////////////////////////
  using cut_size_t = size_t;

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Maximum value for a cut.
  ////////////////////////////////////////////////////////////////////////////
  constexpr cut_size_t max_cut = std::numeric_limits<cut_size_t>::max();

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Available types of cuts. This should be used with the
  ///        `max_ilevel_cut` variables in \ref shared_levelized_file<node>
  ///        in and \ref shared_levelized_file<arc>.
  ///
  /// \remark This is not an `enum class` to ensure it has implicit conversion
  ///         to integers (and hence can be used for indexation).
  ////////////////////////////////////////////////////////////////////////////
  enum cut_type {
    /** Internal arcs only */
    Internal       = 0, // 0x00
    /** Internal arcs and false arcs only */
    Internal_False = 1, // 0x01
    /** Internal arcs and true arcs only */
    Internal_True  = 2, // 0x10
    /** All types of arcs: internal, false, and true */
    All            = 3  // 0x11
  };

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Number of different types of cuts.
  ////////////////////////////////////////////////////////////////////////////
  constexpr size_t cut_types = 4u;

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Type for list of (all) possible cuts.
  ////////////////////////////////////////////////////////////////////////////
  using cuts_t = std::array<cut_size_t, cut_types>;

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Get the desired \ref cut_type based on whether to respectively
  ///        include the false and true terminals.
  ///
  /// \param incl_false Whether to include arcs to the false terminal.
  /// \param incl_true  Whether to include arcs to the true terminal.
  ////////////////////////////////////////////////////////////////////////////
  inline cut_type cut_type_with(const bool incl_false, const bool incl_true)
  {
    return static_cast<cut_type>((incl_true << 1) + incl_false);
  }

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a type of cut includes arcs to the desired terminal.
  ////////////////////////////////////////////////////////////////////////////
  inline bool includes_terminal(const cut_type ct, const bool terminal_val)
  {
    return terminal_val
      ? ct >= cut_type::Internal_True
      : ct == cut_type::Internal_False || ct == cut_type::All;
  }

  inline bool includes_terminal(const size_t ct, const bool terminal_val)
  {
    adiar_assert(ct < cut_types);
    return includes_terminal(static_cast<cut_type>(ct), terminal_val);
  }

  ////////////////////////////////////////////////////////////////////////////
  /// \brief The number of terminals included in a cut type.
  ////////////////////////////////////////////////////////////////////////////
  inline size_t number_of_terminals(const cut_type ct)
  {
    return includes_terminal(ct, false) + includes_terminal(ct, true);
  }

  inline size_t number_of_terminals(const size_t ct)
  {
    adiar_assert(ct < cut_types);
    return number_of_terminals(static_cast<cut_type>(ct));
  }

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Interface hiding how the 1-level cut is obtained
  ////////////////////////////////////////////////////////////////////////////
  struct get_1level_cut
  {
    template<typename dd_t>
    static inline cut_size_t get(const dd_t &dd, const cut_type ct)
    {
      return dd.max_1level_cut(ct);
    }
  };

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Interface hiding how the 2-level cut is obtained
  ////////////////////////////////////////////////////////////////////////////
  struct get_2level_cut
  {
    template<typename dd_t>
    static inline cut_size_t get(const dd_t &dd, const cut_type ct)
    {
      return dd.max_2level_cut(ct);
    }
  };
}

#endif // ADIAR_INTERNAL_CUT_H
