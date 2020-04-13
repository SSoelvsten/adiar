#include <assert.h>
#include <stdint.h>

namespace coom {
  /****************************************************************************/
  /*  CONSTANTS                                                               */
  /****************************************************************************/
  const uint8_t  NODE_INDEX_BITS = 42;
  const uint8_t  NODE_LABEL_BITS = 64 - 1 - NODE_INDEX_BITS;

  const uint64_t NODE_MAX_INDEX  = (1ull << NODE_INDEX_BITS) - 1;
  const uint64_t NODE_MAX_LABEL  = (1ull << NODE_LABEL_BITS) - 1;

  /****************************************************************************/
  /*  NODE ARC / DATA                                                         */
  /****************************************************************************/
  inline uint64_t create_node_arc(uint64_t label, uint64_t index)
  {
#if COOM_DEBUG
    assert (label < NODE_MAX_LABEL);
    assert (index < NODE_MAX_INDEX);
#endif

    return (label << NODE_INDEX_BITS) + index;
  }

  inline uint64_t label_of(uint64_t n)
  {
    return (n >> NODE_INDEX_BITS);
  }

  inline uint64_t index_of(uint64_t n)
  {
    return n & ((1ull << NODE_INDEX_BITS) - 1);
  }

  inline bool is_node_arc(uint64_t n)
  {
    return n < (1ull << 63);
  }

  /****************************************************************************/
  /*  SINK ARC                                                                */
  /****************************************************************************/
  inline uint64_t create_sink_arc(bool v)
  {
    return 0x8000000000000000ull + v;
  }

  inline bool value_of(uint64_t n)
  {
    return n & 0xeffffffffffffffull;
  }

  inline bool is_sink_arc(uint64_t n)
  {
    return n >= (1ull << 63);
  }

  /****************************************************************************/
  /*  NODE                                                                    */
  /****************************************************************************/
  struct node
  {
    uint64_t data;
    uint64_t low;
    uint64_t high;
  };

  inline node create_node(uint64_t label,
                          uint64_t index,
                          uint64_t low,
                          uint64_t high)
  {
    return { create_node_arc(label, index) , low, high };
  }

  inline uint64_t index_of(node n)
  {
    return index_of(n.data);
  }

  inline uint64_t label_of(node n)
  {
    return label_of(n.data);
  }

  bool operator< (const node& a, const node& b)
  {
    return a.data < b.data;
  }

  bool operator> (const node& a, const node& b)
  {
    return a.data > b.data;
  }
}
