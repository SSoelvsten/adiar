#ifndef ADIAR_INTERNAL_DATA_TYPES_ARC_H
#define ADIAR_INTERNAL_DATA_TYPES_ARC_H

#include<adiar/internal/data_types/uid.h>

namespace adiar {
  //////////////////////////////////////////////////////////////////////////////
  /// \brief An arc from some source to a target.
  ///
  /// \details An arc contains a value for the source and one for the target.
  ///          Notice, that we don't specifically have a member for is_high. The
  ///          reason for this choice is that the C++ compiler makes everything
  ///          word-aligned. That means with an explicit is_high member it would
  ///          take up 3 x 64 bits rather than only 2 x 64 bits.
  ///
  /// \remark  If <tt>source</tt> is flagged, then this is a high arc rather than
  ///          a low arc.
  ///
  /// \remark  <tt>source</tt> may be NIL
  //////////////////////////////////////////////////////////////////////////////
  struct arc
  {
    ptr_t source;
    ptr_t target;
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \copydoc arc
  //////////////////////////////////////////////////////////////////////////////
  typedef arc arc_t;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether an arc is marked as the high from its source.
  //////////////////////////////////////////////////////////////////////////////
  inline bool is_high(const arc_t &a)
  {
    return is_flagged(a.source);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Negates the target value, if it is a terminal.
  //////////////////////////////////////////////////////////////////////////////
  inline arc_t negate(const arc_t &a)
  {
    uint64_t target = is_terminal(a.target) ? negate(a.target) : a.target;
    return { a.source, target };
  }

  inline arc_t operator! (const arc &a)
  {
    return negate(a);
  }

  inline bool operator== (const arc &a, const arc &b)
  {
    return a.source == b.source && a.target == b.target;
  }

  inline bool operator!= (const arc &a, const arc &b)
  {
    return !(a==b);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Sorting predicate: First on unique identifier of the source, and
  ///        secondly on whether it is the high arc.
  //////////////////////////////////////////////////////////////////////////////
  struct arc_source_lt
  {
    bool operator ()(const arc_t& a, const arc_t& b) const {
      return a.source < b.source;
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Sorting predicate on the target.
  //////////////////////////////////////////////////////////////////////////////
  struct arc_target_lt
  {
    bool operator ()(const arc_t& a, const arc_t& b) const {
      return a.target < b.target
#ifndef NDEBUG
        || (a.target == b.target && a.source < b.source)
#endif
        ;
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Extracts the label from the target of an arc
  //////////////////////////////////////////////////////////////////////////////
  struct arc_target_label
  {
    static inline label_t label_of(const arc_t& a)
    {
      return adiar::label_of(a.target);
    }
  };
}

#endif // ADIAR_INTERNAL_DATA_TYPES_ARC_H
