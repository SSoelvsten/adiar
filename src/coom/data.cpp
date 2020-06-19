#ifndef COOM_DATA_CPP
#define COOM_DATA_CPP

#include <assert.h>
#include <stdint.h>

#include "data.h"

namespace coom {
  /****************************************************************************/
  /*  CONSTANTS                                                               */
  /****************************************************************************/
  const uint8_t  ID_BITS = 42;
  const uint8_t  LABEL_BITS = 64 - 1 - ID_BITS;

  const uint64_t MAX_ID  = (1ull << ID_BITS) - 1;
  const uint64_t MAX_LABEL  = (1ull << LABEL_BITS) - 1;

  /****************************************************************************/
  /*  NIL PTR                                                                 */
  /****************************************************************************/
  const uint64_t NIL = UINT64_MAX;

  inline bool is_nil(uint64_t n)
  {
    return n == NIL;
  }

  /****************************************************************************/
  /*  NODE PTR                                                                */
  /****************************************************************************/
  inline uint64_t create_node_ptr(uint64_t label, uint64_t id)
  {
#if COOM_DEBUG
    assert (label < MAX_LABEL);
    assert (id < MAX_ID);
#endif

    return (label << ID_BITS) + id;
  }

  inline uint64_t label_of(uint64_t n)
  {
    return (n >> ID_BITS);
  }

  inline uint64_t id_of(uint64_t n)
  {
    return n & ((1ull << ID_BITS) - 1);
  }

  inline bool is_node_ptr(uint64_t n)
  {
    return n < (1ull << 63);
  }

  /****************************************************************************/
  /*  SINK ARC                                                                */
  /****************************************************************************/
  inline uint64_t create_sink(bool v)
  {
    return 0x8000000000000000ull + v;
  }

  inline bool value_of(uint64_t n)
  {
    return n & 0xeffffffffffffffull;
  }

  inline bool is_sink(uint64_t n)
  {
    return !is_nil(n) && n >= (1ull << 63);
  }

  /****************************************************************************/
  /*  NODE                                                                    */
  /****************************************************************************/
  inline node create_node(uint64_t label, uint64_t id, uint64_t low, uint64_t high)
  {
    return { create_node_ptr(label, id) , low, high };
  }

  inline node create_sink_node(bool value)
  {
    return { create_sink(value) , NIL, NIL };
  }

  inline bool is_sink_node(const node& n)
  {
    return is_sink(n.node_ptr);
  }

  inline bool value_of(const node& n)
  {
#if COOM_DEBUG
    assert (is_sink(n));
#endif
    return value_of(n.node_ptr);
  }

  inline uint64_t id_of(const node& n)
  {
#if COOM_DEBUG
    assert (!is_sink(n));
#endif
    return id_of(n.node_ptr);
  }

  inline uint64_t label_of(const node& n)
  {
#if COOM_DEBUG
    assert (!is_sink(n));
#endif
    return label_of(n.node_ptr);
  }

  bool operator< (const node& a, const node& b)
  {
    return a.node_ptr < b.node_ptr;
  }

  bool operator> (const node& a, const node& b)
  {
    return a.node_ptr > b.node_ptr;
  }

  bool operator== (const node& a, const node& b)
  {
    return a.node_ptr == b.node_ptr && a.low == b.low && a.high == b.high;
  }

  bool operator!= (const node& a, const node& b)
  {
    return !(a==b);
  }

  /****************************************************************************/
  /*  ARC                                                                     */
  /****************************************************************************/
  inline arc create_arc(uint64_t source, bool is_high, uint64_t target)
  {
    return { source, is_high, target };
  }

  bool operator< (const arc& a, const arc& b)
  {
    return (a.target < b.target)
      || (a.source == b.source && ((a.is_high < b.is_high)
                                   || (a.target < b.target))
          );
  }

  bool operator> (const arc& a, const arc& b)
  {
    return (a.target > b.target)
      || (a.source == b.source && ((a.is_high > b.is_high)
                                   || (a.target > b.target))
          );
  }

  bool operator== (const arc& a, const arc& b)
  {
    return a.source == b.source && a.is_high == b.is_high && a.target == b.target;
  }

  bool operator!= (const arc& a, const arc& b)
  {
    return !(a==b);
  }

  /****************************************************************************/
  /*  CONVERTERS                                                              */
  /****************************************************************************/
  inline arc low_arc_of_node(const node& n)
  {
    return { n.node_ptr, false, n.low };
  }

  inline arc high_arc_of_node(const node& n)
  {
    return { n.node_ptr, true, n.high };
  }

  inline node node_of_arcs(const arc& low, const arc& high)
  {
#if COOM_DEBUG
    assert (low.source == high.source);
    assert (low.is_high == false);
    assert (high.is_high == true);
#endif
    return { low.source, low.target, high.target };
  }
}

#endif // COOM_DATA_CPP
