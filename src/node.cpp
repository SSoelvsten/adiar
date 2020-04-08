#ifndef NODE_CPP
#define NODE_CPP
/******************************************************************************/
#include <assert.h>
#include <stdint.h>

/******************************************************************************/
/*  CONSTANTS                                                                 */
/******************************************************************************/
const unsigned char NODE_INDEX_BITS = 42;
const unsigned char NODE_LABEL_BITS = 64 - 1 - NODE_INDEX_BITS;

const uint64_t NODE_MAX_INDEX = (1ull << NODE_INDEX_BITS) - 1;
const uint64_t NODE_MAX_LABEL = (1ull << NODE_LABEL_BITS) - 1;

/******************************************************************************/
/*  NODE ARC / DATA                                                           */
/******************************************************************************/
uint64_t create_node_arc(uint64_t label, uint64_t index)
{
  #if DEBUG
  assert (label < NODE_MAX_LABEL);
  assert (index < NODE_MAX_INDEX);
  #endif

  return (label << NODE_INDEX_BITS) + index;
}

uint64_t label_of(uint64_t n)
{
  return (n >> NODE_INDEX_BITS);
}

uint64_t index_of(uint64_t n)
{
  return n & ((1ull << NODE_INDEX_BITS) - 1);
}

bool is_node_arc(uint64_t n)
{
  return n < (1ull << 63);
}

/******************************************************************************/
/*  SINK ARC                                                                  */
/******************************************************************************/
uint64_t create_sink_arc(bool v)
{
  return 0x8000000000000000ull + v;
}

bool value_of(uint64_t n)
{
  return n & 0xeffffffffffffffull;
}

bool is_sink_arc(uint64_t n)
{
  return n >= (1ull << 63);
}

/******************************************************************************/
/*  NODE                                                                      */
/******************************************************************************/
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

/******************************************************************************/
#endif // NODE_CPP
