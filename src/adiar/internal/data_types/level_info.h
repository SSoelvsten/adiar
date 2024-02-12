#ifndef ADIAR_INTERNAL_DATA_TYPES_LEVEL_INFO_H
#define ADIAR_INTERNAL_DATA_TYPES_LEVEL_INFO_H

#include <adiar/internal/data_types/ptr.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Meta information on a single level in a decision diagram.
  //////////////////////////////////////////////////////////////////////////////
  class level_info
  {
    // TODO (Larger variable identifiers):
    //   template <typename level_type = ptr_uint64::label_type>
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of a variable level.
    ////////////////////////////////////////////////////////////////////////////
    using level_type = ptr_uint64::label_type;

    /* ================================ VARIABLES =========================== */
  private:
    level_type _label;

    size_t _width;

    /* ============================== CONSTRUCTORS ========================== */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Default construction (trivial).
    ///
    /// \details The default, copy, and move construction has to be `default` to
    ///          ensure it is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    level_info() = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Copy construction (trivial).
    ///
    /// \details The default, copy, and move construction has to be `default` to
    ///          ensure it is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    level_info(const level_info& li) = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Move construction (trivial).
    ///
    /// \details The default, copy, and move construction has to be `default` to
    ///          ensure it is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    level_info(level_info&& li) = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Destruction (trivial).
    ///
    /// \details The destructor has to be `default` to ensure it is a *POD* and
    ///          hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    ~level_info() = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create a level_info for a variable and a certain width when using
    ///        the identity variable ordering.
    ////////////////////////////////////////////////////////////////////////////
    level_info(level_type label, size_t width)
      : _label(label)
      , _width(width)
    {}

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Copy assignment (trivial).
    ///
    /// \details The copy and move assignment has to be `default` to ensure it
    ///          is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    level_info&
    operator=(const level_info& li) = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Move assignment (trivial).
    ///
    /// \details The copy and move assignment has to be `default` to ensure it
    ///          is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    level_info&
    operator=(level_info&& li) = default;

    /* ============================ MEMBER FUNCTIONS ======================== */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the label for a level.
    ////////////////////////////////////////////////////////////////////////////
    level_type
    label() const
    {
      return _label;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the level.
    ////////////////////////////////////////////////////////////////////////////
    level_type
    level() const
    {
      return _label;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the width (i.e. number of nodes) of this level.
    ////////////////////////////////////////////////////////////////////////////
    level_type
    width() const
    {
      return _width;
    }

    /* =============================== OPERATORS ============================ */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether all entries on this level match.
    ////////////////////////////////////////////////////////////////////////////
    inline bool
    operator==(const level_info& o) const
    {
      return this->_label == o._label && this->_width == o._width;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether any entry on this level mismatches.
    ////////////////////////////////////////////////////////////////////////////
    inline bool
    operator!=(const level_info& o) const
    {
      return !(*this == o);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the level info for the negated decision diagram.
    ////////////////////////////////////////////////////////////////////////////
    level_info
    operator!() const
    {
      return *this;
    }
  };
}

#endif // ADIAR_INTERNAL_DATA_TYPES_LEVEL_INFO_H
