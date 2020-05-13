#ifndef COOM_DATA_CPP
#define COOM_DATA_CPP

#include <assert.h>
#include <stdint.h>

namespace coom {
  /****************************************************************************/
  /*  CONSTANTS                                                               */
  /****************************************************************************/
  const uint8_t  INDEX_BITS = 42;
  const uint8_t  LABEL_BITS = 64 - 1 - INDEX_BITS;

  const uint64_t MAX_INDEX  = (1ull << INDEX_BITS) - 1;
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
  inline uint64_t create_node_ptr(uint64_t label, uint64_t index)
  {
#if COOM_DEBUG
    assert (label < MAX_LABEL);
    assert (index < MAX_INDEX);
#endif

    return (label << INDEX_BITS) + index;
  }

  inline uint64_t label_of(uint64_t n)
  {
    return (n >> INDEX_BITS);
  }

  inline uint64_t index_of(uint64_t n)
  {
    return n & ((1ull << INDEX_BITS) - 1);
  }

  inline bool is_node_ptr(uint64_t n)
  {
    return is_nil(n) || (n < (1ull << 63));
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
  /*  Node                                                                    */
  /****************************************************************************/
  struct node
  {
    uint64_t node_ptr;
    uint64_t low;
    uint64_t high;
  };

  inline node create_node(uint64_t label, uint64_t index, uint64_t low, uint64_t high)
  {
    return { create_node_ptr(label, index) , low, high };
  }

  inline uint64_t index_of(node n)
  {
    return index_of(n.node_ptr);
  }

  inline uint64_t label_of(node n)
  {
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
  struct arc
  {
    uint64_t source;
    bool is_high;
    uint64_t target;
  };

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
}

#endif // COOM_DATA_CPP
