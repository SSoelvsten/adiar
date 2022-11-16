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
  class arc
  {
  public:
    typedef ptr_uint64 ptr_t;

  private:
    ptr_t _source;

    // TODO (Attributed Edges):
    //   Add logic related to flag on 'target'.
    //   At that point, 'target()' should always return the unflagged value?

    ptr_t _target;

  public:
    // Provide 'default' constructors to ensure it being a 'POD' inside of TPIE.
    arc() = default;
    arc(const arc&) = default;
    ~arc() = default;

  public:
    // Provide 'non-default' constructors to make it easy to use outside of TPIE.
    arc(ptr_t source, ptr_t target) : _source(source), _target(target)
    { }

    // TODO: template with 'is_high'
    /*
      template<>
      arc<false>(uid_t source, uid_t target) : _source(source), _target(target)
      { }

      template<>
      arc<true>(uid_t source, uid_t target) : _source(flag(source)), _target(target)
      { }
     */

  public:
    //////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain 'source' value (including flag).
    ///
    /// \TODO Always returned the unflagged value?
    //////////////////////////////////////////////////////////////////////////////
    ptr_t source() const
    { return _source; }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain 'target' value (including flag).
    //////////////////////////////////////////////////////////////////////////////
    ptr_t target() const
    { return _target; }

  public:
    bool is_high() const
    { return _source.is_flagged(); }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \copydoc arc
  //////////////////////////////////////////////////////////////////////////////
  typedef arc arc_t;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Negates the target value, if it is a terminal.
  //////////////////////////////////////////////////////////////////////////////
  inline arc_t negate(const arc_t &a)
  {
    return { a.source(), a.target().is_terminal() ? negate(a.target()) : a.target() };
  }

  inline arc_t operator! (const arc &a)
  {
    return negate(a);
  }

  inline bool operator== (const arc &a, const arc &b)
  {
    return a.source() == b.source() && a.target() == b.target();
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
      return a.source() < b.source();
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Sorting predicate on the target.
  //////////////////////////////////////////////////////////////////////////////
  struct arc_target_lt
  {
    bool operator ()(const arc_t& a, const arc_t& b) const {
      return a.target() < b.target()
#ifndef NDEBUG
        || (a.target() == b.target() && a.source() < b.source())
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
      return a.target().label();
    }
  };
}

#endif // ADIAR_INTERNAL_DATA_TYPES_ARC_H
