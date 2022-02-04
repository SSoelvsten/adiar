#ifndef ADIAR_INTERNAL_PRIORITY_QUEUE_H
#define ADIAR_INTERNAL_PRIORITY_QUEUE_H

#include <functional>

#include <tpie/priority_queue.h>

namespace adiar
{
  template<typename elem_t, typename comp_t = std::less<elem_t>>
  class external_priority_queue
  {
  private:
    tpie::priority_queue<elem_t, comp_t> pq;

  public:
    external_priority_queue(size_t memory_bytes, size_t /*max_size*/) : pq(memory_bytes)
    {}

    elem_t top()
    { return pq.top(); }

    void pop()
    { pq.pop(); }

    void push(const elem_t &e)
    { pq.push(e); }

    size_t size()
    { return pq.size(); }

    bool empty()
    { return pq.empty(); }
  };

  template<typename elem_t, typename comp_t = std::less<elem_t>>
  class internal_priority_queue
  {
  public:
    static constexpr tpie::memory_size_type memory_usage(tpie::memory_size_type no_elements)
    {
      return tpie::internal_priority_queue<elem_t, comp_t>::memory_usage(no_elements);
    }

    static constexpr tpie::memory_size_type memory_fits(tpie::memory_size_type memory_bytes)
    {
      return tpie::internal_priority_queue<elem_t, comp_t>::memory_fits(memory_bytes);
    }

  private:
    tpie::internal_priority_queue<elem_t, comp_t> pq;

  public:
    internal_priority_queue(size_t /*memory_bytes*/, size_t max_size) : pq(max_size)
    {}

    elem_t top()
    { return pq.top(); }

    void pop()
    { pq.pop(); }

    void push(const elem_t &e)
    { pq.push(e); }

    size_t size()
    { return pq.size(); }

    bool empty()
    { return pq.empty(); }
  };
}

#endif // ADIAR_INTERNAL_PRIORITY_QUEUE_H
