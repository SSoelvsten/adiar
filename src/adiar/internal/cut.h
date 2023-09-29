#ifndef ADIAR_INTERNAL_CUT_H
#define ADIAR_INTERNAL_CUT_H

#include <array>
#include <stddef.h>
#include <limits>

#include <adiar/internal/assert.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Available types of cuts. Each instance of a class holds onto a
  ///        certain type of cut which in turn can be used to index into the
  ///        `max_ilevel_cut` variables in \ref shared_levelized_file<node> and
  ///        in \ref shared_levelized_file<arc>.
  ///
  /// \details The implementation of this 'enum with logic' is based on jtlim's
  ///          comment on Stackoverflow: https://stackoverflow.com/a/53284026
  //////////////////////////////////////////////////////////////////////////////
  class cut
  {
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type for a cut size.
    ////////////////////////////////////////////////////////////////////////////
    using size_type = size_t;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type for a cut size.
    ////////////////////////////////////////////////////////////////////////////
    using terminal_type = bool;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Maximum value for a cut.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr size_type max = std::numeric_limits<size_type>::max();

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Enum values for `cut`.
    ////////////////////////////////////////////////////////////////////////////
    enum type
    {
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
    static constexpr size_t size = 4u;

  private:
    type _ct;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of different types of cuts.
    ////////////////////////////////////////////////////////////////////////////
    cut() = delete;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Conversion from the `t` enum value (possibly at compile-time).
    ////////////////////////////////////////////////////////////////////////////
    constexpr cut(type ct)
      : _ct(ct)
    { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Conversion from a `size_t` enum value (possibly at compile-time).
    ////////////////////////////////////////////////////////////////////////////
    constexpr cut(size_t ct)
      : _ct(static_cast<type>(ct))
    { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Convert the boolean values of whether to include the `false` and
    ///        the `true` terminal into a `cut_type`.
    ////////////////////////////////////////////////////////////////////////////
    constexpr cut(bool incl_false, bool incl_true)
      : _ct(static_cast<type>((incl_true << 1) + incl_false))
    { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Implicit conversion to enum to allow for switch and comparisons.
    ////////////////////////////////////////////////////////////////////////////
    constexpr operator type() const
    { return _ct; }

    // Prevent usage: if(cut)
    explicit operator bool() const = delete;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether this cut includes the given terminal.
    ////////////////////////////////////////////////////////////////////////////
    bool includes(const terminal_type &t) const
    { return t ? _ct >= Internal_True : (_ct == Internal_False || _ct == All); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The number of terminals included in this cut.
    ////////////////////////////////////////////////////////////////////////////
    size_t number_of_terminals() const
    { return this->includes(false) + this->includes(true); }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Type for list of (all) possible cuts.
  //////////////////////////////////////////////////////////////////////////////
  using cuts_t = std::array<cut::size_type, cut::size>;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Interface hiding how the 1-level cut is obtained
  //////////////////////////////////////////////////////////////////////////////
  struct get_1level_cut
  {
    using type = cut;

    template<typename dd_t>
    static inline cut::size_type get(const dd_t &dd, const type ct)
    {
      return dd.max_1level_cut(ct);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Interface hiding how the 2-level cut is obtained
  //////////////////////////////////////////////////////////////////////////////
  struct get_2level_cut
  {
    using type = cut;

    template<typename dd_t>
    static inline cut::size_type get(const dd_t &dd, const type ct)
    {
      return dd.max_2level_cut(ct);
    }
  };
}

#endif // ADIAR_INTERNAL_CUT_H
