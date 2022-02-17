#ifndef ADIAR_INTERNAL_TUPLE_H
#define ADIAR_INTERNAL_TUPLE_H

#include <functional>

#include <adiar/data.h>

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

  struct triple : tuple
  {
    ptr_t t3;
  };

  //////////////////////////////////////////////////////////////////////////////
  // Ordered access
  inline ptr_t fst(const ptr_t t1, const ptr_t t2)
  {
    return std::min(t1, t2);
  }

  inline ptr_t fst(const ptr_t t1, const ptr_t t2, const ptr_t t3)
  {
    return std::min({t1, t2, t3});
  }

  inline ptr_t fst(const tuple &t) { return fst(t.t1, t.t2); }
  inline ptr_t fst(const triple &t) { return fst(t.t1, t.t2, t.t3); }

  inline ptr_t snd(const ptr_t t1, const ptr_t t2)
  {
    return std::max(t1, t2);
  }

  inline ptr_t snd(const ptr_t t1, const ptr_t t2, const ptr_t t3)
  {
    return std::max(std::min(t1, t2), std::min(std::max(t1,t2),t3));
  }

  inline ptr_t snd(const tuple &t) { return snd(t.t1, t.t2); }
  inline ptr_t snd(const triple &t) { return snd(t.t1, t.t2, t.t3); }

  inline ptr_t trd(const ptr_t t1, const ptr_t t2, const ptr_t t3)
  {
    return std::max({t1, t2, t3});
  }

  inline ptr_t trd(const triple &t) { return trd(t.t1, t.t2, t.t3); }

  //////////////////////////////////////////////////////////////////////////////
  // Priority queue functions
  struct tuple_label
  {
    static inline label_t label_of(const tuple &t)
    {
      return adiar::label_of(std::min(t.t1, t.t2));
    }
  };

  struct tuple_lt : public std::binary_function<tuple, tuple, bool>
  {
    bool operator()(const tuple &a, const tuple &b)
    {
      // Use of bit-operators to omit branching behaviour and so improve CPU pipelining
      return (a.t1 < b.t1) | ((a.t1 == b.t1) & (a.t2 < b.t2));
    }
  };

  struct tuple_fst_lt : public std::binary_function<tuple, tuple, bool>
  {
    bool operator()(const tuple &a, const tuple &b)
    {
      // Sort primarily by the element to be encountered first
      // Use of bit-operators to omit branching behaviour and so improve CPU pipelining
      return (fst(a) < fst(b)) |
        // Group requests to the same tuple together by sorting on the coordinates
        ((fst(a) == fst(b)) & tuple_lt()(a,b));
    }
  };

  struct tuple_snd_lt : public std::binary_function<tuple, tuple, bool>
  {
    bool operator()(const tuple &a, const tuple &b)
    {
      // Sort primarily by the element to be encountered second
      // Use of bit-operators to omit branching behaviour and so improve CPU pipelining
      return (snd(a) < snd(b)) |
        // Group requests to the same tuple together by sorting on the coordinates
        ((snd(a) == snd(b)) & tuple_lt()(a,b));
    }
  };

  struct triple_label
  {
    static inline label_t label_of(const triple &t)
    {
      return adiar::label_of(fst(t));
    }
  };

  struct triple_lt : public std::binary_function<triple, triple, bool>
  {
    bool operator()(const triple &a, const triple &b)
    {
      // Use of bit-operators to omit branching behaviour and so improve CPU pipelining
      return (a.t1 < b.t1)
        | ((a.t1 == b.t1) & (a.t2 < b.t2))
        | ((a.t1 == b.t1) & (a.t2 == b.t2) & (a.t3 < b.t3));
    }
  };

  struct triple_fst_lt : public std::binary_function<triple, triple, bool>
  {
    bool operator()(const triple &a, const triple &b)
    {
      // Use of bit-operators to omit branching behaviour and so improve CPU pipelining
      return (fst(a) < fst(b))
        // If they are tied, sort them coordinate-wise
        | ((fst(a) == fst(b)) & triple_lt()(a,b));
    }
  };

  struct triple_snd_lt : public std::binary_function<triple, triple, bool>
  {
    bool operator()(const triple &a, const triple &b)
    {
      // Use of bit-operators to omit branching behaviour and so improve CPU pipelining
      return (snd(a) < snd(b))
        // If they are tied, sort them coordinate-wise
        | ((snd(a) == snd(b)) & triple_lt()(a,b));
    }
  };

  struct triple_trd_lt : public std::binary_function<triple, triple, bool>
  {
    bool operator()(const triple &a, const triple &b)
    {
      // Use of bit-operators to omit branching behaviour and so improve CPU pipelining
      return (trd(a) < trd(b))
        // If they are tied, sort them coordinate-wise
        | ((trd(a) == trd(b)) & triple_lt()(a,b));
    }
  };
}
#endif // ADIAR_INTERNAL_TUPLE_H
