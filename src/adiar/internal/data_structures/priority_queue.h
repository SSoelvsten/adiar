#ifndef ADIAR_INTERNAL_DATA_STRUCTURES_PRIORITY_QUEUE_H
#define ADIAR_INTERNAL_DATA_STRUCTURES_PRIORITY_QUEUE_H

#include <functional>

#include <tpie/tpie.h>
#include <tpie/priority_queue.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/memory.h>

namespace adiar::internal
{
  template <memory_mode mem_mode, typename T, typename Comp = std::less<T>>
  class priority_queue;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Wrapper for TPIE's internal binary heap.
  //////////////////////////////////////////////////////////////////////////////
  template<typename T, typename Comp>
  class priority_queue<memory_mode::Internal, T, Comp>
  {
  private:
    using pq_type = tpie::internal_priority_queue<T, Comp>;
    pq_type pq;

  public:
    static tpie::memory_size_type memory_usage(tpie::memory_size_type no_elements)
    {
      return pq_type::memory_usage(no_elements);
    }

    static tpie::memory_size_type memory_fits(tpie::memory_size_type memory_bytes)
    {
      const tpie::memory_size_type ret = pq_type::memory_fits(memory_bytes);

      adiar_assert(memory_usage(ret) <= memory_bytes,
                   "memory_fits and memory_usage should agree.");
      return ret;
    }

    static constexpr size_t data_structures = 1u;

    using value_type = T;

  public:
    priority_queue([[maybe_unused]] size_t memory_bytes, size_t max_size)
      : pq(max_size)
    {
      adiar_assert(max_size <= memory_fits(memory_bytes),
                   "Must be instantiated with enough memory.");
    }

    value_type top() const
    { return pq.top(); }

    void pop()
    { pq.pop(); }

    void push(const value_type &v)
    { pq.push(v); }

    size_t size() const
    { return pq.size(); }

    bool empty() const
    { return pq.empty(); }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Type alias for sorter for partial type application of the
  ///        'internal' memory type.
  //////////////////////////////////////////////////////////////////////////////
  template <typename T, typename Comp = std::less<T>>
  using internal_priority_queue = priority_queue<memory_mode::Internal, T, Comp>;

  // LCOV_EXCL_START
  // TODO: Unit test external memory variants?

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Wrapper for TPIE's internal binary heap.
  //////////////////////////////////////////////////////////////////////////////
  template<typename T, typename Comp>
  class priority_queue<memory_mode::External, T, Comp>
  {
  public:
    static constexpr size_t data_structures = 1u;

    using value_type = T;

  private:
    using pq_type = tpie::priority_queue<value_type, Comp>;
    pq_type pq;

  public:
    priority_queue(size_t memory_bytes, size_t /*max_size*/) 
      : pq(memory_bytes)
    {}

    value_type top()
    { return pq.top(); }

    void pop()
    { pq.pop(); }

    void push(const value_type &v)
    { pq.push(v); }

    size_t size() const
    { return pq.size(); }

    bool empty() const
    { return pq.empty(); }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Type alias for sorter for partial type application of the
  ///        'external' memory type.
  //////////////////////////////////////////////////////////////////////////////
  template <typename T, typename Comp = std::less<T>>
  using external_priority_queue = priority_queue<memory_mode::External, T, Comp>;

  // LCOV_EXCL_STOP
}

#endif // ADIAR_INTERNAL_DATA_STRUCTURES_PRIORITY_QUEUE_H
