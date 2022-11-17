#ifndef ADIAR_INTERNAL_DATA_TYPES_REQUEST_H
#define ADIAR_INTERNAL_DATA_TYPES_REQUEST_H

#include<adiar/internal/data_types/node.h>
#include<adiar/internal/data_types/tuple.h>

namespace adiar
{
  // TODO (template): node type, with_source
  template<uint8_t cardinality = 1, uint8_t children_carried = 0>
  class request
  {
  public:
    static_assert(children_carried < cardinality,
                  "'children_carried' ought not to hold more than 'cardinality' number of nodes' children");

    typedef node::label_t label_t;

    typedef node::ptr_t ptr_t;

    typedef tuple<ptr_t, cardinality> target_t;

    /* ============================== VARIABLES ============================= */
  public:
    target_t target;

    ptr_t children[children_carried][node::OUTDEGREE];

    /* ============================ CONSTRUCTORS ============================ */
  public:
    // Provide 'default' constructors to ensure it being a 'POD' inside of TPIE.
    request() = default;
    request(const request &r) = default;
    ~request() = default;

    /* ======================== DERIVATED INFORMATION ======================= */
    request(const target_t &t,
            const ptr_t (& c) [children_carried][node::OUTDEGREE])
      : target(t), children(c)
    { }
  };

  //////////////////////////////////////////////////////////////////////////////
  // Priority queue functions
  template<class request_t>
  struct request_label
  {
    static inline typename request_t::label_t
    label_of(const request_t &r)
    {
      return r.target.fst().label();
    }
  };

  template<class request_t>
  struct request_fst_lt
  {
    // TODO (DEBUG): sort by source, if it exists
    inline bool operator()(const request_t &a, const request_t &b)
    {
      return tuple_fst_lt<typename request_t::target_t>()(a.target, b.target);
    }
  };

  template<class request_t>
  struct request_snd_lt
  {
    // TODO (DEBUG): sort by source, if it exists
    inline bool operator()(const request_t &a, const request_t &b)
    {
      return tuple_snd_lt<typename request_t::target_t>()(a.target, b.target);
    }
  };
}

#endif // ADIAR_INTERNAL_DATA_TYPES_REQUEST_H
