#ifndef ADIAR_INTERNAL_PRIORITY_QUEUE_H
#define ADIAR_INTERNAL_PRIORITY_QUEUE_H

#include <functional>

#include <tpie/tpie.h>
#include <tpie/priority_queue.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/memory.h>

namespace adiar
{
  template <memory::memory_mode mem_mode, typename elem_t, typename comp_t = std::less<elem_t>>
  class priority_queue;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Wrapper for TPIE's internal binary heap.
  //////////////////////////////////////////////////////////////////////////////
  template<typename elem_t, typename comp_t>
  class priority_queue<memory::EXTERNAL, elem_t, comp_t>
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
  /// \brief Wrapper for TPIE's internal binary heap.
  //////////////////////////////////////////////////////////////////////////////
  template<typename elem_t, typename comp_t>
  class priority_queue<memory::INTERNAL, elem_t, comp_t>
  {
  public:
    static tpie::memory_size_type memory_usage(tpie::memory_size_type no_elements)
    {
      return tpie::internal_priority_queue<elem_t, comp_t>::memory_usage(no_elements);
    }

    static tpie::memory_size_type memory_fits(tpie::memory_size_type memory_bytes)
    {
      return tpie::internal_priority_queue<elem_t, comp_t>::memory_fits(memory_bytes);
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
  using internal_priority_queue = priority_queue<memory::INTERNAL, elem_t, comp_t>;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Type alias for sorter for partial type application of the
  ///        'external' memory type.
  //////////////////////////////////////////////////////////////////////////////
  template <typename elem_t, typename comp_t = std::less<elem_t>>
  using external_priority_queue = priority_queue<memory::EXTERNAL, elem_t, comp_t>;
}

#endif // ADIAR_INTERNAL_PRIORITY_QUEUE_H
