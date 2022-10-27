#ifndef ADIAR_INTERNAL_DATA_TYPES_TRIPLE_H
#define ADIAR_INTERNAL_DATA_TYPES_TRIPLE_H

#include <functional>

#include <adiar/internal/data_types/uid.h>

namespace adiar {
  //////////////////////////////////////////////////////////////////////////////
  // Data structures
  struct triple
  {
    ptr_uint64 t1;
    ptr_uint64 t2;
    ptr_uint64 t3;
  };

  //////////////////////////////////////////////////////////////////////////////
  // Ordered access
  inline ptr_uint64 fst(const ptr_uint64 t1, const ptr_uint64 t2, const ptr_uint64 t3)
  {
    return std::min({t1, t2, t3});
  }

  inline ptr_uint64 fst(const triple &t) { return fst(t.t1, t.t2, t.t3); }

  inline ptr_uint64 snd(const ptr_uint64 t1, const ptr_uint64 t2, const ptr_uint64 t3)
  {
    return std::max(std::min(t1, t2), std::min(std::max(t1,t2),t3));
  }

  inline ptr_uint64 snd(const triple &t) { return snd(t.t1, t.t2, t.t3); }

  inline ptr_uint64 trd(const ptr_uint64 t1, const ptr_uint64 t2, const ptr_uint64 t3)
  {
    return std::max({t1, t2, t3});
  }

  inline ptr_uint64 trd(const triple &t) { return trd(t.t1, t.t2, t.t3); }

  //////////////////////////////////////////////////////////////////////////////
  // Priority queue functions
  struct triple_label
  {
    static inline label_t label_of(const triple &t)
    {
      return fst(t).label();
    }
  };

  struct triple_lt
  {
    bool operator()(const triple &a, const triple &b)
    {
      return a.t1 < b.t1
        || (a.t1 == b.t1 && a.t2 < b.t2)
        || (a.t1 == b.t1 && a.t2 == b.t2 && a.t3 < b.t3);
    }
  };

  struct triple_fst_lt
  {
    bool operator()(const triple &a, const triple &b)
    {
      return fst(a) < fst(b)
        // If they are tied, sort them coordinate-wise
        || (fst(a) == fst(b) && triple_lt()(a,b));
    }
  };

  struct triple_snd_lt
  {
    bool operator()(const triple &a, const triple &b)
    {
      return snd(a) < snd(b)
        // If they are tied, sort them coordinate-wise
        || (snd(a) == snd(b) && triple_lt()(a,b));
    }
  };

  struct triple_trd_lt
  {
    bool operator()(const triple &a, const triple &b)
    {
      return trd(a) < trd(b)
        // If they are tied, sort them coordinate-wise
        || (trd(a) == trd(b) && triple_lt()(a,b));
    }
  };
}
#endif // ADIAR_INTERNAL_DATA_TYPES_TRIPLE_H
