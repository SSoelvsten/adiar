#ifndef COOM_DATA_CPP
#define COOM_DATA_CPP

#include "data.h"

#include <assert.h>
#include <coom/assert.h>

namespace coom {
  //////////////////////////////////////////////////////////////////////////////
  ///  NIL PTR
  //////////////////////////////////////////////////////////////////////////////
  const ptr_t NIL = UINT64_MAX - 1;

  bool is_nil(ptr_t p)
  {
    // Check for flagged and unflagged NIL
    return p >= NIL;
  }

  //////////////////////////////////////////////////////////////////////////////
  ///  COMMON VARIABLES AND GENERAL PTR
  //////////////////////////////////////////////////////////////////////////////
  const uint8_t  LABEL_BITS = 16;
  const uint64_t MAX_LABEL  = (1ull << LABEL_BITS) - 1;

  const uint8_t  ID_BITS = 64 - 2 - LABEL_BITS;
  const uint64_t MAX_ID  = (1ull << ID_BITS) - 1;

  const uint64_t SINK_BIT = 0x8000000000000000ull;
  const uint64_t FLAG_BIT = 0x0000000000000001ull;

  bool is_sink_ptr(ptr_t p)
  {
    return !is_nil(p) && p >= SINK_BIT;
  }

  bool is_node_ptr(ptr_t p)
  {
    return p <= ~SINK_BIT;
  }

  bool is_flagged(ptr_t p)
  {
    return p & FLAG_BIT;
  }

  ptr_t flag(ptr_t p)
  {
    return p | FLAG_BIT;
  }

  ptr_t unflag(ptr_t p)
  {
    return p & (~FLAG_BIT);
  }

  //////////////////////////////////////////////////////////////////////////////
  //// NODE PTR
  //////////////////////////////////////////////////////////////////////////////
  uid_t create_node_uid(label_t label, id_t id)
  {
#if COOM_ASSERT
    assert (label <= MAX_LABEL);
    assert (id <= MAX_ID);
#endif

    return (label << (ID_BITS + 1)) + (id << 1);
  }

  ptr_t create_node_ptr(label_t label, id_t id)
  {
    return create_node_uid(label, id);
  }

  label_t label_of(uint64_t n)
  {
    return n >> (ID_BITS + 1);
  }

  id_t id_of(uint64_t n)
  {
    return (n >> 1) & MAX_ID;
  }

  //////////////////////////////////////////////////////////////////////////////
  ///  SINK PTR
  //////////////////////////////////////////////////////////////////////////////
  ptr_t create_sink_ptr(bool v)
  {
    return SINK_BIT + (v << 1);
  }

  bool value_of(uint64_t n)
  {
#if COOM_ASSERT
    assert(is_sink_ptr(n));
#endif

    return (n & ~SINK_BIT) >> 1;
  }

  ptr_t negate(ptr_t n)
  {
#if COOM_ASSERT
    assert(is_sink_ptr(n));
#endif

    return 2u ^ n;
  }

  const sink_pred is_any = [] ([[maybe_unused]]ptr_t sink) -> bool
  {
#if COOM_ASSERT
    assert(is_sink_ptr(sink));
#endif
    return true;
  };

  const sink_pred is_true = [] (ptr_t sink) -> bool
  {
#if COOM_ASSERT
    assert(is_sink_ptr(sink));
#endif
    return value_of(sink);
  };

  const sink_pred is_false = [] (ptr_t sink) -> bool
  {
#if COOM_ASSERT
    assert(is_sink_ptr(sink));
#endif
    return !value_of(sink);
  };

  const bool_op and_op = [](ptr_t sink1, ptr_t sink2) -> ptr_t
  {
    return unflag(sink1 & sink2);
  };

  const bool_op nand_op = [](ptr_t sink1, ptr_t sink2) -> ptr_t
  {
    return negate(unflag(sink1 & sink2));
  };

  const bool_op or_op = [](ptr_t sink1, ptr_t sink2) -> ptr_t
  {
    return unflag(sink1 | sink2);
  };

  const bool_op nor_op = [](ptr_t sink1, ptr_t sink2) -> ptr_t
  {
    return negate(unflag(sink1 | sink2));
  };

  const bool_op xor_op = [](ptr_t sink1, ptr_t sink2) -> ptr_t
  {
    return SINK_BIT | unflag(sink1 ^ sink2);
  };

  const bool_op implies_op = [](ptr_t sink1, ptr_t sink2) -> ptr_t
  {
    return create_sink_ptr(!value_of(sink1) || value_of(sink2));
  };

  const bool_op impliedby_op = [](ptr_t sink1, ptr_t sink2) -> ptr_t
  {
    return create_sink_ptr(!value_of(sink2) || value_of(sink1));
  };

  const bool_op equiv_op = [](ptr_t sink1, ptr_t sink2) -> ptr_t
  {
    return create_sink_ptr(sink1 == sink2);
  };

  const bool_op diff_op = [](ptr_t sink1, ptr_t sink2) -> ptr_t
  {
    return create_sink_ptr(value_of(sink1) && !value_of(sink2));
  };

  const bool_op less_op = [](ptr_t sink1, ptr_t sink2) -> ptr_t
  {
    return create_sink_ptr(!value_of(sink1) && value_of(sink2));
  };

  bool can_right_shortcut(const bool_op &op, const ptr_t sink)
  {
    return op(create_sink_ptr(false), sink) == op(create_sink_ptr(true), sink);
  }

  bool can_left_shortcut(const bool_op &op, const ptr_t sink)
  {
    return op(sink, create_sink_ptr(false)) == op(sink, create_sink_ptr(true));
  }

  bool is_right_irrelevant(const bool_op &op, const ptr_t sink)
  {
    return op(create_sink_ptr(false), sink) == create_sink_ptr(false)
      && op(create_sink_ptr(true), sink) == create_sink_ptr(true);
  }

  bool is_left_irrelevant(const bool_op &op, const ptr_t sink)
  {
    return op(sink, create_sink_ptr(false)) == create_sink_ptr(false)
      && op(sink, create_sink_ptr(true)) == create_sink_ptr(true);
  }

  bool is_right_negating(const bool_op &op, const ptr_t sink)
  {
    return op(sink, create_sink_ptr(false)) == create_sink_ptr(true)
      && op(sink, create_sink_ptr(true)) == create_sink_ptr(false);
  }

  bool is_left_negating(const bool_op &op, const ptr_t sink)
  {
    return op(sink, create_sink_ptr(false)) == create_sink_ptr(true)
      && op(sink, create_sink_ptr(true)) == create_sink_ptr(false);
  }

  bool is_commutative(const bool_op &op)
  {
    ptr_t sink_T = create_sink_ptr(true);
    ptr_t sink_F = create_sink_ptr(false);

    return op(sink_T, sink_F) == op(sink_F, sink_T);
  }

  //////////////////////////////////////////////////////////////////////////////
  ///  NODE
  //////////////////////////////////////////////////////////////////////////////
  node create_node(uid_t uid, ptr_t low, ptr_t high)
  {
    return { uid, low, high };
  }

  node_t create_node(label_t label, id_t id, ptr_t low, ptr_t high)
  {
#if COOM_ASSERT
    assert(!is_nil(low));
    assert(is_sink_ptr(low) || label < label_of(low));

    assert(!is_nil(high));
    assert(is_sink_ptr(high) || label < label_of(high));
#endif

    return create_node(create_node_uid(label, id), low, high);
  }

  node create_node(label_t label, id_t id, ptr_t low, node_t high)
  {
    return create_node(label, id, low, high.uid);
  }

  node create_node(label_t label, id_t id, node_t low, ptr_t high)
  {
    return create_node(label, id, low.uid, high);
  }

  node create_node(label_t label, id_t id, node_t low, node_t high)
  {
    return create_node(label, id, low.uid, high.uid);
  }

  node_t create_sink(bool value)
  {
    return { create_sink_ptr(value) , NIL, NIL };
  }

  bool is_sink(const node_t& n)
  {
    return n.uid >= SINK_BIT;
  }

  bool value_of(const node_t& n)
  {
#if COOM_ASSERT
    assert (is_sink(n));
#endif
    return value_of(n.uid);
  }

  id_t id_of(const node& n)
  {
#if COOM_ASSERT
    assert (!is_sink(n));
#endif
    return id_of(n.uid);
  }

  label_t label_of(const node& n)
  {
#if COOM_ASSERT
    assert (!is_sink(n));
#endif
    return label_of(n.uid);
  }

  node_t negate(const node_t &n)
  {
    if (is_sink(n)) {
      return { negate(n.uid), NIL, NIL };
    }

    uint64_t low =  is_sink_ptr(n.low)  ? negate(n.low)  : n.low;
    uint64_t high = is_sink_ptr(n.high) ? negate(n.high) : n.high;
    return { n.uid, low, high };
  }

  node operator! (const node& n)
  {
    return negate(n);
  }

  bool operator< (const node& a, const node& b)
  {
    return a.uid < b.uid;
  }

  bool operator> (const node& a, const node& b)
  {
    return a.uid > b.uid;
  }

  bool operator== (const node& a, const node& b)
  {
    return a.uid == b.uid && a.low == b.low && a.high == b.high;
  }

  bool operator!= (const node& a, const node& b)
  {
    return !(a==b);
  }

  //////////////////////////////////////////////////////////////////////////////
  ///  ARC
  //////////////////////////////////////////////////////////////////////////////
  bool is_high(arc& a)
  {
    return is_flagged(a.source);
  }

  arc_t negate(const arc_t &a)
  {
    uint64_t target = is_sink_ptr(a.target) ? negate(a.target) : a.target;
    return { a.source, target };
  }

  arc_t operator! (const arc& a)
  {
    return negate(a);
  }

  bool operator== (const arc& a, const arc& b)
  {
    return a.source == b.source && a.target == b.target;
  }

  bool operator!= (const arc& a, const arc& b)
  {
    return !(a==b);
  }

  //////////////////////////////////////////////////////////////////////////////
  ///  CONVERTERS
  //////////////////////////////////////////////////////////////////////////////
  arc low_arc_of(const node& n)
  {
    return { n.uid, n.low };
  }

  arc high_arc_of(const node& n)
  {
    return { flag(n.uid), n.high };
  }

  node node_of(const arc& low, const arc& high)
  {
#if COOM_ASSERT
    assert (unflag(low.source) == unflag(high.source));
    assert (!is_flagged(low.source));
    assert (is_flagged(high.source));
#endif
    return { low.source, low.target, high.target };
  }

  //////////////////////////////////////////////////////////////////////////////
  ///  ASSIGNMENT
  //////////////////////////////////////////////////////////////////////////////
  assignment_t create_assignment(label_t label, bool value)
  {
#if COOM_ASSERT
    assert (label <= MAX_LABEL);
#endif
    return { label, value };
  }

  assignment operator! (const assignment& a)
  {
    return { a.label, !a.value };
  }

  bool operator< (const assignment& a, const assignment& b)
  {
    return a.label < b.label || (a.label == b.label && a.value < b.value);
  }

  bool operator> (const assignment& a, const assignment& b)
  {
    return a.label > b.label || (a.label == b.label && a.value > b.value);
  }

  bool operator== (const assignment& a, const assignment& b)
  {
    return a.label == b.label && a.value == b.value;
  }

  bool operator!= (const assignment& a, const assignment& b)
  {
    return !(a==b);
  }

  //////////////////////////////////////////////////////////////////////////////
  ///  META
  //////////////////////////////////////////////////////////////////////////////
  meta operator! (const meta& m)
  {
    return m;
  }

  bool operator== (const meta& a, const meta& b)
  {
    return a.label == b.label;
  }

  bool operator!= (const meta& a, const meta& b)
  {
    return !(a==b);
  }
}

#endif // COOM_DATA_CPP
