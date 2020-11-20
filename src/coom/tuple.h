#ifndef COOM_TUPLE_H
#define COOM_TUPLE_H

namespace coom {
  //////////////////////////////////////////////////////////////////////////////
  // Data structures
  struct tuple
  {
    ptr_t t1;
    ptr_t t2;
  };

  struct tuple_data
  {
    ptr_t t1;
    ptr_t t2;
    ptr_t data_low;
    ptr_t data_high;
  };

  //////////////////////////////////////////////////////////////////////////////
  // Priority queue functions
  struct tuple_queue_lt
  {
    bool operator()(const tuple &a, const tuple &b)
    {
      return std::min(a.t1, a.t2) < std::min(b.t1, b.t2) ||
            (std::min(a.t1, a.t2) == std::min(b.t1, b.t2) && std::max(a.t1, a.t2) < std::max(b.t1, b.t2));
    }
  };

  struct tuple_queue_label
  {
    label_t label_of(const tuple &t)
    {
      return coom::label_of(std::min(t.t1, t.t2));
    }
  };

  struct tuple_queue_data_lt
  {
    bool operator()(const tuple_data &a, const tuple_data &b)
    {
      return std::max(a.t1, a.t2) < std::max(b.t1, b.t2) ||
            (std::max(a.t1, a.t2) == std::max(b.t1, b.t2) && std::min(a.t1, a.t2) < std::min(b.t1, b.t2));
    }
  };
}
#endif // COOM_TUPLE_H
