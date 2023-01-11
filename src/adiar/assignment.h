#ifndef ADIAR_ASSIGNMENT_H
#define ADIAR_ASSIGNMENT_H

#include <functional>

#include <adiar/internal/assert.h>
#include <adiar/internal/data_types/ptr.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief A <tt>(x,v)</tt> tuple representing the single assignment
  ///        \f$ x \mapsto v \f$.
  //////////////////////////////////////////////////////////////////////////////
  class assignment
  {
    /* ================================= TYPES ============================== */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the variable label.
    ///
    /// \todo Turn into <tt>size_t</tt>?
    ////////////////////////////////////////////////////////////////////////////
    using label_t = internal::ptr_uint64::label_t;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Enum with the values one can assign to a variable.
    ///
    /// \details A variable can be assigned `FALSE` and `TRUE`. Furthermore,
    ///          with `DONT_CARE` it can also be assigned some value. Finally,
    ///          with `NONE` you can .
    ///
    ////////////////////////////////////////////////////////////////////////////
    // TODO(MTBDD): Replace 'FALSE' and 'TRUE' with a value of type T. In this
    //              case, the `value_t` would turn into a std::variant of 'T',
    //              'DONT_CARE', and 'NONE'.
    enum value_t {
      FALSE     = 0, // false
      TRUE      = 1, // true
      NONE      = -1 // <-- TODO: test
    };

    /* =============================== VARIABLES ============================ */
  private:
    label_t _var;
    value_t _val;

    /* ============================== CONSTRUCTORS ========================== */
  public:
    ////////////////////////////////////////////////////////////////////////////
    // Provide 'default' constructors to ensure it being a 'POD' inside of TPIE.
    assignment() = default;
    assignment(const assignment &a) = default;
    ~assignment() = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Constructs assignment pair (variable, value).
    ////////////////////////////////////////////////////////////////////////////
    assignment(const label_t &var, const value_t &val)
      : _var(var), _val(val)
    {
      adiar_debug(var <= internal::ptr_uint64::MAX_LABEL,
                  "Cannot represent that large a variable label");
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Constructs assignment pair (variable, value) from a boolean
    ///        value.
    ////////////////////////////////////////////////////////////////////////////
    assignment(const label_t &var, const bool &val)
      : assignment(var, (value_t) val)
    { }

    /* =============================== ACCESSORS ============================ */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the variable of this assignment.
    ////////////////////////////////////////////////////////////////////////////
    inline label_t var() const
    { return _var; }

    ////////////////////////////////////////////////////////////////////////////
    inline label_t level() const
    { return var(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the value of this assignment pair.
    ////////////////////////////////////////////////////////////////////////////
    inline value_t value() const
    { return _val; }

    /* ============================== COMPARATORS =========================== */
  public:
    inline bool operator<  (const assignment &o) const
    {
      adiar_debug(this->_var != o._var,
                  "Currently, '<' is only designed for distinct '_var' values");
      return this->_var < o._var;
    }

    inline bool operator>  (const assignment &o) const
    { return o < *this; }

    inline bool operator== (const assignment &o) const
    { return this->_var == o._var && this->_val == o._val; }

    inline bool operator!= (const assignment &o) const
    { return !(*this == o); }

    /* =============================== OPERATORS ============================ */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the negated value assigned to the same variable.
    ////////////////////////////////////////////////////////////////////////////
    assignment operator~ () const
    {
      return {
        this->_var,
        (this->_val & 1) == (this->_val)
          ? static_cast<value_t>(this->_val ^ 1)
          : this->_val
      };
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Assignment function which provides for each variable label its
  ///        assigned value.
  //////////////////////////////////////////////////////////////////////////////
  using assignment_func = std::function<assignment::value_t(assignment::label_t)>;
}

#endif // ADIAR_ASSIGNMENT_H
