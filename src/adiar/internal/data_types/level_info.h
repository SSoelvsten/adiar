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
    //   template <typename level_t = ptr_uint64::label_t>
  public:
    using level_t = ptr_uint64::label_t;

    /* ================================ VARIABLES =========================== */
  private:
    level_t _label;
    size_t _width;

    /* ============================== CONSTRUCTORS ========================== */
  public:
    // Provide 'default' constructors to ensure it being a 'POD' inside of TPIE.
    level_info() = default;
    level_info(const level_info &) = default;
    ~level_info() = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create a level_info for a variable and a certain width when using
    ///        the identity variable ordering.
    ////////////////////////////////////////////////////////////////////////////
    level_info(level_t label, size_t width)
      : _label(label), _width(width)
    { }

    /* ============================ MEMBER FUNCTIONS ======================== */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the label for a level.
    ////////////////////////////////////////////////////////////////////////////
    level_t label() const
    { return _label; }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the level.
    ////////////////////////////////////////////////////////////////////////////
    level_t level() const
    { return _label; }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the width (i.e. number of nodes) of this level.
    ////////////////////////////////////////////////////////////////////////////
    level_t width() const
    { return _width; }

    /* =============================== OPERATORS ============================ */
  public:
    inline bool operator== (const level_info &o) const
    { return this->_label == o._label && this->_width == o._width; }

    inline bool operator!= (const level_info &o) const
    { return !(*this == o); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the level info for the negated decision diagram.
    ////////////////////////////////////////////////////////////////////////////
    level_info operator~ () const
    { return *this; }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \internal This is only due to \ref file_stream requires the existence of
  ///           the ! operator.
  //////////////////////////////////////////////////////////////////////////////
  inline level_info operator! (const level_info &m)
  { return ~m; }
}

#endif // ADIAR_INTERNAL_DATA_TYPES_LEVEL_INFO_H
