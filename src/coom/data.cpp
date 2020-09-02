#ifndef COOM_DATA_CPP
#define COOM_DATA_CPP

#include <assert.h>
#include <stdint.h>

#include "data.h"

namespace coom {
  /****************************************************************************/
  /*  NIL PTR                                                                 */
  /****************************************************************************/
  const ptr_t NIL = UINT64_MAX - 1;

  inline bool is_nil(ptr_t p)
  {
    // Check for flagged and unflagged NIL
    return p >= NIL;
  }

  /****************************************************************************/
  /*  COMMON VARIABLES AND GENERAL PTR                                        */
  /****************************************************************************/
  const uint8_t  ID_BITS = 42;
  const uint8_t  LABEL_BITS = 64 - 2 - ID_BITS;

  const uint64_t MAX_ID  = (1ull << ID_BITS) - 1;
  const uint64_t MAX_LABEL  = (1ull << LABEL_BITS) - 1;

  const uint64_t SINK_BIT = 0x8000000000000000ull;
  const uint64_t FLAG_BIT = 0x0000000000000001ull;

  inline bool is_sink_ptr(ptr_t p)
  {
    return !is_nil(p) && p >= SINK_BIT;
  }

  inline bool is_node_ptr(ptr_t p)
  {
    return p <= ~SINK_BIT;
  }

  inline bool is_flagged(ptr_t p)
  {
    return p & FLAG_BIT;
  }

  inline ptr_t flag(ptr_t p)
  {
    return p | FLAG_BIT;
  }

  inline ptr_t unflag(ptr_t p)
  {
    return p & (~FLAG_BIT);
  }

  /****************************************************************************/
  /*  NODE PTR                                                                */
  /****************************************************************************/
  inline uid_t create_node_uid(label_t label, id_t id)
  {
#if COOM_ASSERT
    assert (label <= MAX_LABEL);
    assert (id <= MAX_ID);
#endif

    return (label << (ID_BITS + 1)) + (id << 1);
  }

  inline ptr_t create_node_ptr(label_t label, id_t id)
  {
    return create_node_uid(label, id);
  }

  inline label_t label_of(uint64_t n)
  {
    return n >> (ID_BITS + 1);
  }

  inline id_t id_of(uint64_t n)
  {
    return (n >> 1) & MAX_ID;
  }

  /****************************************************************************/
  /*  SINK ARC                                                                */
  /****************************************************************************/
  inline ptr_t create_sink_ptr(bool v)
  {
    return SINK_BIT + (v << 1);
  }

  inline bool value_of(uint64_t n)
  {
    return (n & ~SINK_BIT) >> 1;
  }


  /****************************************************************************/
  /*  NODE                                                                    */
  /****************************************************************************/
  inline node create_node(uid_t uid, ptr_t low, ptr_t high)
  {
    return { uid, low, high };
  }

  inline node create_node(label_t label, id_t id, ptr_t low, ptr_t high)
  {
    return create_node(create_node_uid(label, id), low, high);
  }

  inline node create_sink(bool value)
  {
    return { create_sink_ptr(value) , NIL, NIL };
  }

  inline bool is_sink(const node& n)
  {
    return n.uid >= SINK_BIT;
  }

  inline bool value_of(const node& n)
  {
#if COOM_ASSERT
    assert (is_sink(n));
#endif
    return value_of(n.uid);
  }

  inline id_t id_of(const node& n)
  {
#if COOM_ASSERT
    assert (!is_sink(n));
#endif
    return id_of(n.uid);
  }

  inline label_t label_of(const node& n)
  {
#if COOM_ASSERT
    assert (!is_sink(n));
#endif
    return label_of(n.uid);
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

  /****************************************************************************/
  /*  ARC                                                                     */
  /****************************************************************************/
  inline bool is_high(arc& a)
  {
    return is_flagged(a.source);
  }

  bool operator== (const arc& a, const arc& b)
  {
    return a.source == b.source && a.target == b.target;
  }

  bool operator!= (const arc& a, const arc& b)
  {
    return !(a==b);
  }

  /****************************************************************************/
  /*  CONVERTERS                                                              */
  /****************************************************************************/
  inline arc low_arc_of(const node& n)
  {
    return { n.uid, n.low };
  }

  inline arc high_arc_of(const node& n)
  {
    return { flag(n.uid), n.high };
  }

  inline node node_of(const arc& low, const arc& high)
  {
#if COOM_ASSERT
    assert (unflag(low.source) == unflag(high.source));
    assert (!is_flagged(low.source));
    assert (is_flagged(high.source));
#endif
    return { low.source, low.target, high.target };
  }

  /****************************************************************************/
  /*  META                                                                    */
  /****************************************************************************/
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
