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

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the values.
    ///
    /// \todo Replace with enum for { FALSE (0), TRUE (1), DONT_CARE (2) }.
    //////////////////////////////////////////////////////////////////////////////
    typedef bool value_t;

    /* ================================ VARIABLESS ============================= */
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

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Constructs assignment pair (variable, value).
    ////////////////////////////////////////////////////////////////////////////
    assignment(const label_t &var, const value_t &val)
      : _var(var), _val(val)
    {
      adiar_debug(var <= internal::ptr_uint64::MAX_LABEL,
                  "Cannot represent that large a variable label");
    }

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
    { return this->_var < o._var; }

    inline bool operator>  (const assignment &o) const
    { return this->_var > o._var; }

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
      return { this->_var, !this->_val };
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Assignment function which provides for each variable label its
  ///        assigned value.
  //////////////////////////////////////////////////////////////////////////////
  using assignment_func = std::function<assignment::value_t(assignment::label_t)>;
}

#endif // ADIAR_ASSIGNMENT_H
