#include <adiar/bdd.h>

#include <adiar/domain.h>
#include <adiar/exception.h>
#include <adiar/internal/cut.h>
#include <adiar/internal/assert.h>
#include <adiar/internal/util.h>
#include <adiar/internal/algorithms/traverse.h>
#include <adiar/internal/data_types/level_info.h>
#include <adiar/internal/io/file_stream.h>
#include <adiar/internal/io/file_writer.h>
#include <adiar/internal/io/levelized_file_stream.h>
#include <adiar/internal/io/node_writer.h>

namespace adiar
{
  class bdd_eval_func_visitor
  {
    const predicate<bdd::label_type> &af;
    bool result = false;

  public:
    bdd_eval_func_visitor(const predicate<bdd::label_type>& f)
      : af(f)
    { }

    inline bdd::pointer_type visit(const bdd::node_type &n)
    {
      const bool a = af(n.label());
      return a ? n.high() : n.low();
    }

    inline void visit(const bool s)
    { result = s; }

    inline bool get_result()
    { return result; }
  };

  bool bdd_eval(const bdd &bdd, const predicate<bdd::label_type> &af)
  {
    bdd_eval_func_visitor v(af);
    traverse(bdd, v);
    return v.get_result();
  }

  //////////////////////////////////////////////////////////////////////////////
  class bdd_eval_file_visitor
  {
    internal::file_stream<map_pair<bdd::label_type, boolean>> mps;
    map_pair<bdd::label_type, boolean> mp;

    bool result = false;

  public:
    bdd_eval_file_visitor(const shared_file<map_pair<bdd::label_type, boolean>>& msf)
      : mps(msf)
    { if (mps.can_pull()) { mp = mps.pull(); } }

    inline bdd::pointer_type visit(const bdd::node_type &n)
    {
      const bdd::label_type level = n.label();
      while (mp.level() < level) {
        if (!mps.can_pull()) {
          throw out_of_range("Labels are insufficient to traverse BDD");
        }
        if (mps.peek().level() <= mp.level()) {
          throw invalid_argument("Labels are not in ascending order");
        }

        mp = mps.pull();
      }

      if (mp.level() != level) {
        throw invalid_argument("Missing assignment for node visited in BDD");
      }

      return mp.value() ? n.high() : n.low();
    }

    inline void visit(const bool s)
    { result = s; }

    inline bool get_result()
    { return result; }
  };

  bool bdd_eval(const bdd &bdd,
                const shared_file<map_pair<bdd::label_type, boolean>> &mpf)
  {
    bdd_eval_file_visitor v(mpf);
    internal::traverse(bdd, v);
    return v.get_result();
  }

  //////////////////////////////////////////////////////////////////////////////
  class bdd_sat_bdd_callback
  {
    // TODO: replace with TPIE's external stack
    using e = map_pair<bdd::label_type, boolean>;

    shared_file<e> ef;
    internal::file_writer<e> ew;

  public:
    bdd_sat_bdd_callback() : ew(ef)
    { }

    void operator() (bdd::label_type x, bool v)
    { ew << map_pair<bdd::label_type, boolean>(x, v); }

    bdd get_bdd()
    {
      assert(ew.size() > 0);
      ew.detach();
      internal::file_stream<e, true> es(ef);

      internal::shared_levelized_file<bdd::node_type> nf;
      internal::node_writer nw(nf);

      bdd::pointer_type latest = bdd::pointer_type(true);

      while (es.can_pull()) {
        const e kv = es.pull();

        const bdd::node_type n(kv.key(), bdd::max_id,
                            kv.value() ? bdd::pointer_type(false) : latest,
                            kv.value() ? latest : bdd::pointer_type(false));

        nw.unsafe_push(n);
        nw.unsafe_push(internal::level_info(kv.key(), 1u));

        latest = n.uid();
      }

      nf->max_1level_cut[internal::cut::Internal]       = 1u;
      nf->max_1level_cut[internal::cut::Internal_False] = nf->number_of_terminals[false];
      nf->max_1level_cut[internal::cut::Internal_True]  = 1u; // == nf->number_of_terminals[true]
      nf->max_1level_cut[internal::cut::All]            = nf->number_of_terminals[false] + 1u;

      return nf;
    }
  };

  class bdd_sat_lambda_callback
  {
    const consumer<bdd::label_type, bool> &_c;

  public:
    bdd_sat_lambda_callback(const consumer<bdd::label_type, bool> &lambda)
      : _c(lambda)
    { }

    void operator() (bdd::label_type x, bool v) const
    { _c(x,v); }
  };

  template<typename visitor_t, typename callback_t, typename lvl_stream_t, typename lvl_t>
  class bdd_sat_visitor
  {
  private:
    visitor_t   _visitor;
    callback_t& _callback;

    bdd::label_type _lvl = bdd::max_label+1u;
    lvl_stream_t _lvls;

  public:
    bdd_sat_visitor() = delete;
    bdd_sat_visitor(const bdd_sat_visitor&) = delete;
    bdd_sat_visitor(bdd_sat_visitor&&) = delete;

    bdd_sat_visitor(callback_t &cb, const lvl_t &lvl)
      : _callback(cb), _lvls(lvl)
    { }

    bdd::pointer_type visit(const bdd::node_type &n)
    {
      const bdd::pointer_type next_ptr = _visitor.visit(n);
      _lvl = n.label();

      // set default to all skipped levels
      while (_lvls.can_pull() && internal::level_of(_lvls.peek()) < _lvl) {
        _callback(internal::level_of(_lvls.pull()), visitor_t::default_direction);
      }
      // skip visited level (if exists)
      if (_lvls.can_pull() && internal::level_of(_lvls.peek()) == _lvl) {
        _lvls.pull();
      }

      _callback(_lvl, next_ptr == n.high());
      return next_ptr;
    }

    void visit(const bool s)
    {
      _visitor.visit(s);

      while (_lvls.can_pull()) {
        if (internal::level_of(_lvls.peek()) <= _lvl) { _lvls.pull(); continue; };
        _callback(internal::level_of(_lvls.pull()), visitor_t::default_direction);
      }
    }
  };

  template<typename visitor_t, typename callback_t>
  inline void __bdd_satX(const bdd &f, callback_t &_cb)
  {
    if (domain_isset()) {
      bdd_sat_visitor<visitor_t, callback_t, internal::file_stream<domain_var>, internal::shared_file<domain_var>>
        v(_cb, domain_get());
      internal::traverse(f,v);
    } else {
      bdd_sat_visitor<visitor_t, callback_t, internal::level_info_stream<>, bdd>
        v(_cb, f);
      internal::traverse(f,v);
    }
  }

  bdd bdd_satmin(const bdd &f)
  {
    if (bdd_isterminal(f)) { return f; }

    bdd_sat_bdd_callback _cb;
    __bdd_satX<internal::traverse_satmin_visitor>(f, _cb);
    return _cb.get_bdd();
  }

  void bdd_satmin(const bdd &f, const consumer<bdd::label_type, bool> &cb)
  {
    bdd_sat_lambda_callback _cb(cb);
    __bdd_satX<internal::traverse_satmin_visitor>(f, _cb);
  }

  bdd bdd_satmax(const bdd &f)
  {
    if (bdd_isterminal(f)) { return f; }

    bdd_sat_bdd_callback _cb;
    __bdd_satX<internal::traverse_satmax_visitor>(f, _cb);
    return _cb.get_bdd();
  }

  void bdd_satmax(const bdd &f, const consumer<bdd::label_type, bool> &cb)
  {
    bdd_sat_lambda_callback _cb(cb);
    __bdd_satX<internal::traverse_satmax_visitor>(f, _cb);
  }
}
