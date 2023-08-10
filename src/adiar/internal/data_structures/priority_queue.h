#ifndef ADIAR_INTERNAL_DATA_STRUCTURES_PRIORITY_QUEUE_H
#define ADIAR_INTERNAL_DATA_STRUCTURES_PRIORITY_QUEUE_H

#include <functional>

#include <tpie/tpie.h>
#include <tpie/priority_queue.h>

#include <adiar/memory_mode.h>
#include <adiar/internal/assert.h>
#include <adiar/internal/memory.h>

namespace adiar::internal
{
  template <memory_mode_t mem_mode, typename elem_t, typename comp_t = std::less<elem_t>>
  class priority_queue;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Wrapper for TPIE's internal binary heap.
  //////////////////////////////////////////////////////////////////////////////
  template<typename elem_t, typename comp_t>
  class priority_queue<memory_mode_t::INTERNAL, elem_t, comp_t>
  {
  public:
    static tpie::memory_size_type unsafe_memory_usage(tpie::memory_size_type no_elements)
    {
      return tpie::internal_priority_queue<elem_t, comp_t>::memory_usage(no_elements);
    }

    static tpie::memory_size_type memory_usage(tpie::memory_size_type no_elements)
    {
      const tpie::memory_size_type max_value = std::numeric_limits<tpie::memory_size_type>::max();
      const tpie::memory_size_type max_elem = memory_fits(max_value);
      if (no_elements > max_elem) {
        return max_value;
      }
      return unsafe_memory_usage(no_elements);
    }

    static tpie::memory_size_type memory_fits(tpie::memory_size_type memory_bytes)
    {
      const tpie::memory_size_type ret = tpie::internal_priority_queue<elem_t, comp_t>::memory_fits(memory_bytes);
      adiar_debug(unsafe_memory_usage(ret) <= memory_bytes,
                   "memory_fits and memory_usage should agree.");
      return ret;
    }

    static constexpr size_t DATA_STRUCTURES = 1u;

  private:
    tpie::internal_priority_queue<elem_t, comp_t> pq;

  public:
    priority_queue([[maybe_unused]] size_t memory_bytes, size_t max_size)
      : pq(max_size)
    {
      adiar_debug(max_size <= memory_fits(memory_bytes),
                  "Must be instantiated with enough memory.");
    }

    elem_t top() const
    { return pq.top(); }

    void pop()
    { pq.pop(); }

    void push(const elem_t &e)
    { pq.push(e); }

    size_t size() const
    { return pq.size(); }

    bool empty() const
    { return pq.empty(); }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Type alias for sorter for partial type application of the
  ///        'internal' memory type.
  //////////////////////////////////////////////////////////////////////////////
  template <typename elem_t, typename comp_t = std::less<elem_t>>
  using internal_priority_queue = priority_queue<memory_mode_t::INTERNAL, elem_t, comp_t>;

  // LCOV_EXCL_START
  // TODO: Unit test external memory variants?

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Wrapper for TPIE's internal binary heap.
  //////////////////////////////////////////////////////////////////////////////
  template<typename elem_t, typename comp_t>
  class priority_queue<memory_mode_t::EXTERNAL, elem_t, comp_t>
  {
  public:
    static constexpr size_t DATA_STRUCTURES = 1u;

  private:
    tpie::priority_queue<elem_t, comp_t> pq;

  public:
    priority_queue(size_t memory_bytes, size_t /*max_size*/) : pq(memory_bytes)
    {}

    elem_t top()
    { return pq.top(); }

    void pop()
    { pq.pop(); }

    void push(const elem_t &e)
    { pq.push(e); }

    size_t size() const
    { return pq.size(); }

    bool empty() const
    { return pq.empty(); }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Type alias for sorter for partial type application of the
  ///        'external' memory type.
  //////////////////////////////////////////////////////////////////////////////
  template <typename elem_t, typename comp_t = std::less<elem_t>>
  using external_priority_queue = priority_queue<memory_mode_t::EXTERNAL, elem_t, comp_t>;

  // LCOV_EXCL_STOP
}

#endif // ADIAR_INTERNAL_DATA_STRUCTURES_PRIORITY_QUEUE_H
