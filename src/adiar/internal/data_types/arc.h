#ifndef ADIAR_INTERNAL_DATA_TYPES_ARC_H
#define ADIAR_INTERNAL_DATA_TYPES_ARC_H

#include <adiar/internal/data_types/ptr.h>
#include <adiar/internal/data_types/uid.h>

namespace adiar::internal
{
  // TODO (LDD):
  //   Make the source a weighted pointer.

  // TODO (QMDD):
  //   Same as for LDD but with the weight specifically being complex values.

  //////////////////////////////////////////////////////////////////////////////
  /// \brief An arc from some source to a target.
  ///
  /// \details An arc contains a value for the source and one for the target.
  ///          Notice, that we don't specifically have a member for is_high. The
  ///          reason for this choice is that the C++ compiler makes everything
  ///          word-aligned. That means with an explicit is_high member it would
  ///          take up 3 x 64 bits rather than only 2 x 64 bits.
  ///
  /// \remark If `source` is flagged, then this is a high arc rather than a low
  ///         arc.
  ///
  /// \remark  `source` may be nil
  //////////////////////////////////////////////////////////////////////////////
  class arc
  {
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the pointer of this arc's source and target.
    ////////////////////////////////////////////////////////////////////////////
    using ptr_t = ptr_uint64;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the a node's unique identifier.
    ////////////////////////////////////////////////////////////////////////////
    using uid_t = __uid<ptr_t>;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of variable label.
    ////////////////////////////////////////////////////////////////////////////
    using label_t = ptr_t::label_t;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of terminal values.
    ////////////////////////////////////////////////////////////////////////////
    using value_t = ptr_t::value_t;

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

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct an arc with the given source and target.
    ///
    /// \pre The flags on both `source` and `target` are already set correctly
    ///      and the out-index on `source` is too.
    ////////////////////////////////////////////////////////////////////////////
    arc(const ptr_t &source, const ptr_t &target)
      : _source(source), _target(target)
    {
      adiar_assert(!target.is_node() || target.out_idx() == 0u);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct an arc with the given source and target.
    ///
    /// \pre The flags on `target` is already set correctly.
    ////////////////////////////////////////////////////////////////////////////
    arc(const uid_t &source,
        const ptr_t::out_idx_t &out_idx,
        const ptr_t &target)
      : _source(source.with(out_idx))
      , _target(target)
    {
      adiar_assert(!target.is_node() || target.out_idx() == 0u);
    }

    /* ================================ NODES =============================== */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain 'source' value (including flag and out-index).
    ////////////////////////////////////////////////////////////////////////////
    // TODO Always return the essential pointer?
    ptr_t source() const
    { return _source; }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain 'target' value (including flag).
    ////////////////////////////////////////////////////////////////////////////
    // TODO Always return the essential pointer?
    ptr_t target() const
    { return _target; }

    /* ================================= FLAGS ============================== */
  public:
    ptr_t::out_idx_t out_idx() const
    { return _source.out_idx(); }

    /* ============================== COMPARATORS =========================== */

  public:
    inline bool operator== (const arc &o) const
    { return this->_source == o._source && this->_target == o._target; }

    inline bool operator!= (const arc &o) const
    { return !(*this == o); }

    /* =============================== OPERATORS ============================ */
  public:

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain an arc where the target's value (if any) is negated.
    ////////////////////////////////////////////////////////////////////////////
    arc operator~ () const
    {
      return arc(this->_source,
                 this->_target.is_terminal() ? ~this->_target : this->_target);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain an arc where the target's value (if any) is negated.
  //////////////////////////////////////////////////////////////////////////////
  inline arc negate(const arc &a)
  { return ~a; }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Sorting predicate: First on unique identifier of the source, and
  ///        secondly on whether it is the high arc.
  //////////////////////////////////////////////////////////////////////////////
  struct arc_source_lt
  {
    bool operator ()(const arc& a, const arc& b) const {
      return a.source() < b.source();
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Sorting predicate on the target.
  //////////////////////////////////////////////////////////////////////////////
  struct arc_target_lt
  {
    bool operator ()(const arc& a, const arc& b) const {
      return a.target() < b.target()
#ifndef NDEBUG
        || (a.target() == b.target() && a.source() < b.source())
#endif
        ;
    }
  };
}

#endif // ADIAR_INTERNAL_DATA_TYPES_ARC_H
