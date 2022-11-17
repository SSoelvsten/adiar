#ifndef ADIAR_ASSIGNMENT_H
#define ADIAR_ASSIGNMENT_H

#include <functional>

#include<adiar/internal/assert.h>
#include<adiar/internal/data_types/ptr.h>

namespace adiar {
  //////////////////////////////////////////////////////////////////////////////
  /// \brief An assignment [label -> value] to a variable with the given label to
  ///        which is assigned the given value.
  //////////////////////////////////////////////////////////////////////////////
  struct assignment {
    typedef ptr_uint64::label_t label_t;

    ptr_uint64::label_t label;
    bool value;
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \copydoc assignment
  //////////////////////////////////////////////////////////////////////////////
  typedef assignment assignment_t;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create an assignment [label -> value].
  //////////////////////////////////////////////////////////////////////////////
  inline assignment_t create_assignment(ptr_uint64::label_t label, bool value)
  {
    adiar_debug(label <= ptr_uint64::MAX_LABEL, "Cannot represent that large a label");

    return { label, value };
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Extract the label from an assignment [label -> value].
  //////////////////////////////////////////////////////////////////////////////
  inline ptr_uint64::label_t label_of(const assignment_t &a)
  {
    return a.label;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Extract the value from an assignment [label -> value].
  //////////////////////////////////////////////////////////////////////////////
  inline bool value_of(const assignment_t &a)
  {
    return a.value;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Negate the value of an assignment [label -> value].
  //////////////////////////////////////////////////////////////////////////////
  inline assignment operator! (const assignment &a)
  {
    return { a.label, !a.value };
  }

  inline bool operator< (const assignment &a, const assignment &b)
  {
    return a.label < b.label;
  }

  inline bool operator> (const assignment &a, const assignment &b)
  {
    return a.label > b.label;
  }

  inline bool operator== (const assignment &a, const assignment &b)
  {
    return a.label == b.label && a.value == b.value;
  }

  inline bool operator!= (const assignment &a, const assignment &b)
  {
    return !(a==b);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Function that computs Boolean assignment to variables with given
  ///          label.
  //////////////////////////////////////////////////////////////////////////////
  typedef std::function<bool(ptr_uint64::label_t)> assignment_func;
}

#endif // ADIAR_ASSIGNMENT_H
