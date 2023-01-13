#ifndef ADIAR_INTERNAL_DATA_TYPES_VAR_MAP_H
#define ADIAR_INTERNAL_DATA_TYPES_VAR_MAP_H

#include <functional>
#include <type_traits>

#include <adiar/internal/assert.h>
#include <adiar/internal/data_types/ptr.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief A <tt>(x,v)</tt> tuple representing the single association
  ///        \f$ x \mapsto v \f$.
  ///
  /// \details The given enum type should have two values: 'FALSE' with the
  ///          integral value '0' and 'TRUE' with the integral value '1'.
  //////////////////////////////////////////////////////////////////////////////
  template<typename value_enum>
  class var_mapping
  {
    /* ================================= TYPES ============================== */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the variable label.
    ///
    /// \todo Turn into <tt>size_t</tt>?
    ////////////////////////////////////////////////////////////////////////////
    using label_t = ptr_uint64::label_t;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type (enum) with the values one can assign to a variable.
    ////////////////////////////////////////////////////////////////////////////
    using value_t = value_enum;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The raw integral type used for the value enum.
    ////////////////////////////////////////////////////////////////////////////
    using raw_t = typename std::underlying_type<value_t>::type;

  private:
    /* ================================= CHECKS ============================= */
    static_assert(std::is_integral<raw_t>::value);
    static_assert(static_cast<raw_t>(value_t::FALSE) == static_cast<raw_t>(0u));
    static_assert(static_cast<raw_t>(value_t::TRUE)  == static_cast<raw_t>(1u));

    /* =============================== VARIABLES ============================ */
  private:
    label_t _var;
    value_t _val;

    /* ============================== CONSTRUCTORS ========================== */
  public:
    ////////////////////////////////////////////////////////////////////////////
    // Provide 'default' constructors to ensure it being a 'POD' inside of TPIE.
    var_mapping<value_enum>() = default;
    var_mapping<value_enum>(const var_mapping<value_enum> &a) = default;
    ~var_mapping<value_enum>() = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Constructs var_mapping pair (variable, value).
    ////////////////////////////////////////////////////////////////////////////
    var_mapping<value_enum>(const label_t &var, const value_t &val)
      : _var(var), _val(val)
    {
      adiar_debug(var <= ptr_uint64::MAX_LABEL,
                  "Cannot represent that large a variable label");
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Constructs var_mapping pair (variable, value) from a boolean
    ///        value.
    ////////////////////////////////////////////////////////////////////////////
    var_mapping<value_enum>(const label_t &var, const bool &val)
      : var_mapping(var, static_cast<value_t>(val))
    { }

    /* =============================== ACCESSORS ============================ */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the variable of this pair.
    ////////////////////////////////////////////////////////////////////////////
    inline label_t var() const
    { return _var; }

    ////////////////////////////////////////////////////////////////////////////
    /// \copydoc var()
    ////////////////////////////////////////////////////////////////////////////
    inline label_t level() const
    { return var(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the value of this pair.
    ////////////////////////////////////////////////////////////////////////////
    inline value_t value() const
    { return _val; }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the *raw* value of this pair.
    ////////////////////////////////////////////////////////////////////////////
    inline raw_t raw() const
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
    { return value() == value_t::TRUE; }

    /* ============================== COMPARATORS =========================== */
  public:
    inline bool operator<  (const var_mapping<value_enum> &o) const
    {
      adiar_debug(this->_var != o._var,
                  "Currently, '<' is only designed for distinct '_var' values");
      return this->_var < o._var;
    }

    inline bool operator>  (const var_mapping<value_enum> &o) const
    { return o < *this; }

    inline bool operator== (const var_mapping<value_enum> &o) const
    { return this->_var == o._var && this->_val == o._val; }

    inline bool operator!= (const var_mapping<value_enum> &o) const
    { return !(*this == o); }

    /* =============================== OPERATORS ============================ */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the negated value assigned to the same variable.
    ////////////////////////////////////////////////////////////////////////////
    var_mapping<value_enum> operator~ () const
    {
      const raw_t value_raw = static_cast<raw_t>(this->_val);

      return {
        this->_var,
        (value_raw & 1) == (value_raw)
          ? static_cast<value_t>(value_raw ^ 1u)
          : this->_val
      };
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief A function that provides a mapping \f$ x \mapsto v \f$.
  //////////////////////////////////////////////////////////////////////////////
  template<typename value_enum>
  using var_func_map = std::function<value_enum(ptr_uint64::label_t)>;
}

#endif // ADIAR_INTERNAL_DATA_TYPES_VAR_MAP_H
