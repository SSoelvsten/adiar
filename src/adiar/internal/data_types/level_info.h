#ifndef ADIAR_INTERNAL_DATA_TYPES_LEVEL_INFO_H
#define ADIAR_INTERNAL_DATA_TYPES_LEVEL_INFO_H

#include <adiar/internal/assert.h>
#include <adiar/internal/data_types/ptr.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Meta information on a single level in a decision diagram.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  class level_info
  {
    // TODO (Larger variable identifiers):
    //   template <typename level_type = ptr_uint64::label_type>
  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Type of a variable level.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    using level_type = ptr_uint64::level_type;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Type of a difference between variable levels.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    using signed_level_type = ptr_uint64::signed_level_type;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Type of a variable label.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    using label_type = ptr_uint64::level_type;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Type of a difference between variable labels.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    using signed_label_type = ptr_uint64::signed_level_type;

    /* ========================================== VARIABLES ===================================== */
  private:
    level_type _level;

    size_t _width;

    /* ======================================== CONSTRUCTORS ==================================== */
  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief   Default construction (trivial).
    ///
    /// \details The default, copy, and move construction has to be `default` to ensure it is a
    ///          *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    level_info() = default;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief   Copy construction (trivial).
    ///
    /// \details The default, copy, and move construction has to be `default` to ensure it is a
    ///          *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    level_info(const level_info& li) = default;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief   Move construction (trivial).
    ///
    /// \details The default, copy, and move construction has to be `default` to ensure it is a
    ///          *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    level_info(level_info&& li) = default;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief   Destruction (trivial).
    ///
    /// \details The destructor has to be `default` to ensure it is a *POD* and hence can be used by
    ///          TPIE's files.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ~level_info() = default;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Create a level_info for a variable and a certain width when using the identity
    ///        variable ordering.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    level_info(level_type level, size_t width)
      : _level(level)
      , _width(width)
    {
      adiar_assert(level <= ptr_uint64::max_label, "Level should be valid in 'ptr_uint64'");
      adiar_assert(width != 0, "Empty levels are never to-be recorded");
      adiar_assert(width <= ptr_uint64::max_id + 1, "A level should be valid in 'ptr_uint64'");
    }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief   Copy assignment (trivial).
    ///
    /// \details The copy and move assignment has to be `default` to ensure it is a *POD* and hence
    ///          can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    level_info&
    operator=(const level_info& li) = default;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief   Move assignment (trivial).
    ///
    /// \details The copy and move assignment has to be `default` to ensure it is a *POD* and hence
    ///          can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    level_info&
    operator=(level_info&& li) = default;

    /* ====================================== MEMBER FUNCTIONS ================================== */
  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the level.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    level_type
    level() const
    {
      return this->_level;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the label for a level.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    level_type
    label() const
    {
      return this->level();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the width (i.e. number of nodes) of this level.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    size_t
    width() const
    {
      return this->_width;
    }

    /* ========================================= OPERATORS ====================================== */
  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether all entries on this level match.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    inline bool
    operator==(const level_info& o) const
    {
      return this->_level == o._level && this->_width == o._width;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether any entry on this level mismatches.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    inline bool
    operator!=(const level_info& o) const
    {
      return !(*this == o);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the level info for the negated decision diagram.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    level_info
    operator!() const
    {
      return *this;
    }
  };

  /* ============================================ LEVELS ======================================== */
  inline level_info
  shift_replace(const level_info& li, const level_info::signed_level_type levels)
  {
    using level_type        = level_info::level_type;
    using signed_level_type = level_info::signed_level_type;

    adiar_assert(levels < 0 ? (static_cast<level_type>(std::abs(levels)) <= li.level())
                            : (li.level() + levels <= ptr_uint64::max_label));

    // TODO: Use dynamic casts instead?
    const level_type new_level =
      static_cast<level_type>(static_cast<signed_level_type>(li.level()) + levels);

    return { new_level, li.width() };
  }
}

#endif // ADIAR_INTERNAL_DATA_TYPES_LEVEL_INFO_H
