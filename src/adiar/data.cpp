#include "data.h"

#include <assert.h>
#include <adiar/assert.h>

namespace adiar {
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
  const uint8_t  LABEL_BITS = 24;
  const uint64_t MAX_LABEL  = (1ull << LABEL_BITS) - 1;

  const uint8_t  ID_BITS = 64 - 2 - LABEL_BITS;
  const uint64_t MAX_ID  = (1ull << ID_BITS) - 1;

  const uint64_t SINK_BIT = 0x8000000000000000ull;
  const uint64_t FLAG_BIT = 0x0000000000000001ull;

  bool is_sink(ptr_t p)
  {
    return !is_nil(p) && p >= SINK_BIT;
  }

  bool is_node(ptr_t p)
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
    adiar_debug(label <= MAX_LABEL, "Cannot represent given label");
    adiar_debug(id <= MAX_ID, "Cannot represent given id");

    return ((uint64_t) label << (ID_BITS + 1)) + (id << 1);
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
  uid_t create_sink_uid(bool v)
  {
    return SINK_BIT + (v << 1);
  }

  ptr_t create_sink_ptr(bool v)
  {
    return create_sink_uid(v);
  }

  bool value_of(uint64_t n)
  {
    adiar_debug(is_sink(n), "Cannot extract value of non-sink");

    return (n & ~SINK_BIT) >> 1;
  }

  ptr_t negate(ptr_t n)
  {
    adiar_debug(is_sink(n), "Cannot negate non-sink");

    return 2u ^ n;
  }

  const sink_pred is_any = [] ([[maybe_unused]]ptr_t sink) -> bool
  {
    adiar_debug(is_sink(sink), "Cannot examine non-sink");

    return true;
  };

  const sink_pred is_true = [] (ptr_t sink) -> bool
  {
    adiar_debug(is_sink(sink), "Cannot examine non-sink");

    return value_of(sink);
  };

  const sink_pred is_false = [] (ptr_t sink) -> bool
  {
    adiar_debug(is_sink(sink), "Cannot examine non-sink");

    return !value_of(sink);
  };

  const bool_op and_op = [](ptr_t sink1, ptr_t sink2) -> ptr_t
  {
    return unflag(sink1 & sink2);
  };

  const bool_op nand_op = [](ptr_t sink1, ptr_t sink2) -> ptr_t
  {
    return negate(and_op(sink1, sink2));
  };

  const bool_op or_op = [](ptr_t sink1, ptr_t sink2) -> ptr_t
  {
    return unflag(sink1 | sink2);
  };

  const bool_op nor_op = [](ptr_t sink1, ptr_t sink2) -> ptr_t
  {
    return negate(or_op(sink1, sink2));
  };

  const bool_op xor_op = [](ptr_t sink1, ptr_t sink2) -> ptr_t
  {
    return SINK_BIT | unflag(sink1 ^ sink2);
  };

  const bool_op xnor_op  = [](ptr_t sink1, ptr_t sink2) -> ptr_t
  {
    return negate(xor_op(sink1, sink2));
  };

  const bool_op imp_op = [](ptr_t sink1, ptr_t sink2) -> ptr_t
  {
    return create_sink_ptr(unflag(sink1) <= unflag(sink2));
  };

  const bool_op invimp_op = [](ptr_t sink1, ptr_t sink2) -> ptr_t
  {
    return create_sink_ptr(unflag(sink2) <= unflag(sink1));
  };

  const bool_op equiv_op = xnor_op;

  const bool_op diff_op = [](ptr_t sink1, ptr_t sink2) -> ptr_t
  {
    return and_op(sink1, negate(sink2));
  };

  const bool_op less_op = [](ptr_t sink1, ptr_t sink2) -> ptr_t
  {
    return and_op(negate(sink1), sink2);
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
  bool on_level(ptr_t p, label_t level)
  {
    return is_sink(p) ? false : label_of(p) == level;
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
    // TODO: Should these be adiar_assert instead to check validity of user input?
    adiar_debug(!is_nil(low), "Cannot create a node with NIL child");
    adiar_debug(is_sink(low) || label < label_of(low), "Node is not prior to given low child");

    adiar_debug(!is_nil(high), "Cannot create a node with NIL child");
    adiar_debug(is_sink(high) || label < label_of(high), "Node is not prior to given high child");

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
    adiar_debug(is_sink(n), "Cannot extract value from non-sink");

    return value_of(n.uid);
  }

  id_t id_of(const node& n)
  {
    adiar_debug(!is_sink(n), "Cannot extract id of a sink");

    return id_of(n.uid);
  }

  label_t label_of(const node& n)
  {
    adiar_debug(!is_sink(n), "Cannot extract label of a sink");

    return label_of(n.uid);
  }

  bool on_level(const node&n, label_t level)
  {
    return on_level(n.uid, level);
  }

  node_t negate(const node_t &n)
  {
    if (is_sink(n)) {
      return { negate(n.uid), NIL, NIL };
    }

    uint64_t low =  is_sink(n.low)  ? negate(n.low)  : n.low;
    uint64_t high = is_sink(n.high) ? negate(n.high) : n.high;
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
  bool is_high(const arc& a)
  {
    return is_flagged(a.source);
  }

  arc_t negate(const arc_t &a)
  {
    uint64_t target = is_sink(a.target) ? negate(a.target) : a.target;
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
    adiar_debug(unflag(low.source) == unflag(high.source), "Arcs are not of the same node");
    adiar_debug(!is_high(low), "High flag set on low child");
    adiar_debug(is_high(high), "High flag not set on high child");

    return { low.source, low.target, high.target };
  }

  //////////////////////////////////////////////////////////////////////////////
  ///  ASSIGNMENT
  //////////////////////////////////////////////////////////////////////////////
  assignment_t create_assignment(label_t label, bool value)
  {
    adiar_debug(label <= MAX_LABEL, "Cannot represent that large a label");

    return { label, value };
  }

  label_t label_of(const assignment_t& a)
  {
    return a.label;
  }

  bool value_of(const assignment_t& a)
  {
    return a.value;
  }

  assignment operator! (const assignment& a)
  {
    return { a.label, !a.value };
  }

  bool operator< (const assignment& a, const assignment& b)
  {
    return a.label < b.label;
  }

  bool operator> (const assignment& a, const assignment& b)
  {
    return a.label > b.label;
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
  level_info operator! (const level_info& m)
  {
    return m;
  }

  level_info_t create_meta(label_t label, size_t level_size)
  {
    return { label, level_size };
  }

  label_t label_of(const level_info_t& m)
  {
    return m.label;
  }

  size_t size_of(const level_info_t& m)
  {
    return m.size;
  }

  bool operator== (const level_info& a, const level_info& b)
  {
    return a.label == b.label && a.size == b.size;
  }

  bool operator!= (const level_info& a, const level_info& b)
  {
    return !(a==b);
  }
}
