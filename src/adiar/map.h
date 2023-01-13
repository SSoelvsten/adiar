#ifndef ADIAR_MAP_H
#define ADIAR_MAP_H

#include <functional>
#include <type_traits>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Minimal enum for `map_pair`.
  ///
  /// \sa map_pair
  //////////////////////////////////////////////////////////////////////////////
  enum boolean : bool
  {
    FALSE = false,
    TRUE  = true
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief A <tt>(x,v)</tt> tuple representing the single association
  ///        \f$ x \mapsto v \f$ where \f$v\f$ is a value of enum type.
  ///
  /// \tparam key_type   The (integral) type for the map's key.
  ///
  /// \tparam value_enum An enum type that has the two values 'FALSE' with the
  ///                    integral value '0' and 'TRUE' with the integral value
  ///                    '1', e.g. `bool_enum`.
  //////////////////////////////////////////////////////////////////////////////
  template<typename key_type, typename value_enum = boolean>
  class map_pair
  {
    /* ================================= TYPES ============================== */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the key.
    ///
    /// \todo Turn into <tt>size_t</tt>?
    ////////////////////////////////////////////////////////////////////////////
    using key_t = key_type;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type (enum) with the values one can assign to a key.
    ////////////////////////////////////////////////////////////////////////////
    using value_t = value_enum;
    static_assert(std::is_enum<value_t>::value);

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The raw integral type used for the value enum.
    ////////////////////////////////////////////////////////////////////////////
    using raw_t = typename std::underlying_type<value_t>::type;

  private:
    /* ================================= CHECKS ============================= */
    static_assert(std::is_integral<key_t>::value);
    static_assert(std::is_integral<raw_t>::value);
    static_assert(static_cast<raw_t>(value_t::FALSE) == static_cast<raw_t>(0u));
    static_assert(static_cast<raw_t>(value_t::TRUE)  == static_cast<raw_t>(1u));

    /* =============================== VARIABLES ============================ */
  private:
    key_t _key;
    value_t _val;

    /* ============================== CONSTRUCTORS ========================== */
  public:
    ////////////////////////////////////////////////////////////////////////////
    // Provide 'default' constructors to ensure it being a 'POD' inside of TPIE.
    map_pair<key_type, value_enum>() = default;
    map_pair<key_type, value_enum>(const map_pair<key_type, value_enum> &a) = default;
    ~map_pair<key_type, value_enum>() = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Constructs map_pair pair (key, value).
    ////////////////////////////////////////////////////////////////////////////
    map_pair<key_type, value_enum>(const key_t key, const value_t val)
      : _key(key), _val(val)
    {
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Constructs map_pair pair (key, value) from a boolean
    ///        value.
    ////////////////////////////////////////////////////////////////////////////
    map_pair<key_type, value_enum>(const key_t key, const bool val)
      : map_pair(key, static_cast<value_t>(val))
    { }

    /* =============================== ACCESSORS ============================ */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the key of this pair.
    ////////////////////////////////////////////////////////////////////////////
    inline key_t key() const
    { return _key; }

    ////////////////////////////////////////////////////////////////////////////
    /// \copydoc key()
    ////////////////////////////////////////////////////////////////////////////
    inline key_t level() const
    { return key(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the value of this pair.
    ////////////////////////////////////////////////////////////////////////////
    inline value_t value() const
    { return _val; }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the *raw* value of this pair.
    ////////////////////////////////////////////////////////////////////////////
    inline raw_t raw_value() const
    { return static_cast<raw_t>(_val); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain whether the value is 'FALSE'.
    ////////////////////////////////////////////////////////////////////////////
    inline bool is_false() const
    { return value() == value_t::FALSE; }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain whether the value is 'TRUE'.
    ////////////////////////////////////////////////////////////////////////////
    inline bool is_true() const
    {
      return value() == value_t::TRUE;
    }

    /* ============================== COMPARATORS =========================== */
  public:
    inline bool operator<  (const map_pair<key_type, value_enum> &o) const
    { return this->_key < o._key; }

    inline bool operator>  (const map_pair<key_type, value_enum> &o) const
    { return o < *this; }

    inline bool operator== (const map_pair<key_type, value_enum> &o) const
    { return this->_key == o._key && this->_val == o._val; }

    inline bool operator!= (const map_pair<key_type, value_enum> &o) const
    { return !(*this == o); }

    /* =============================== OPERATORS ============================ */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the negated value assigned to the same key.
    ////////////////////////////////////////////////////////////////////////////
    map_pair<key_type, value_enum> operator~ () const
    {
      const raw_t value_raw = static_cast<raw_t>(this->_val);

      return {
        this->_key,
        (value_raw & 1) == (value_raw)
          ? static_cast<value_t>(value_raw ^ 1u)
          : this->_val
      };
    }
  };
}

#endif // ADIAR_MAP_H
