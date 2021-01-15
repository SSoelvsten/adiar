#ifndef ADIAR_TUPLE_H
#define ADIAR_TUPLE_H

namespace adiar {
  //////////////////////////////////////////////////////////////////////////////
  // Data structures
  struct tuple
  {
    ptr_t t1;
    ptr_t t2;
  };

  struct tuple_data : tuple
  {
    ptr_t data_low;
    ptr_t data_high;
  };

  //////////////////////////////////////////////////////////////////////////////
  // Priority queue functions
  struct tuple_queue_label
  {
    label_t label_of(const tuple &t)
    {
      return adiar::label_of(std::min(t.t1, t.t2));
    }
  };

  struct tuple_queue_1_lt
  {
    bool operator()(const tuple &a, const tuple &b)
    {
      return std::min(a.t1, a.t2) < std::min(b.t1, b.t2) ||
        (std::min(a.t1, a.t2) == std::min(b.t1, b.t2) && std::max(a.t1, a.t2) < std::max(b.t1, b.t2));
    }
  };

  struct tuple_queue_2_lt
  {
    bool operator()(const tuple &a, const tuple &b)
    {
      return std::max(a.t1, a.t2) < std::max(b.t1, b.t2) ||
            (std::max(a.t1, a.t2) == std::max(b.t1, b.t2) && std::min(a.t1, a.t2) < std::min(b.t1, b.t2));
    }
  };
}
#endif // ADIAR_TUPLE_H
