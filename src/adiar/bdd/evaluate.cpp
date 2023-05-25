#include <adiar/bdd.h>

#include <functional>

#include <adiar/internal/cut.h>
#include <adiar/internal/assert.h>
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
    const std::function<bool(bdd::label_t)> &af;
    bool result = false;

  public:
    bdd_eval_func_visitor(const std::function<bool(bdd::label_t)>& f)
      : af(f)
    { }

    inline bdd::ptr_t visit(const bdd::node_t &n)
    {
      const bool a = af(n.label());
      return a ? n.high() : n.low();
    }

    inline void visit(const bool s)
    { result = s; }

    inline bool get_result()
    { return result; }
  };

  bool bdd_eval(const bdd &bdd, const std::function<bool(bdd::label_t)> &af)
  {
    bdd_eval_func_visitor v(af);
    traverse(bdd, v);
    return v.get_result();
  }

  //////////////////////////////////////////////////////////////////////////////
  class bdd_eval_file_visitor
  {
    internal::file_stream<map_pair<bdd::label_t, boolean>> mps;
    map_pair<bdd::label_t, boolean> mp;

    bool result = false;

  public:
    bdd_eval_file_visitor(const shared_file<map_pair<bdd::label_t, boolean>>& msf)
      : mps(msf)
    { if (mps.can_pull()) { mp = mps.pull(); } }

    inline bdd::ptr_t visit(const bdd::node_t &n)
    {
      const bdd::label_t level = n.label();
      while (mp.level() < level) {
        adiar_assert(mps.can_pull(),
                     "Assignment file is insufficient to traverse BDD");
        mp = mps.pull();
      }
      adiar_assert(mp.level() == level,
                   "Missing assignment for node visited in BDD");

      return mp.value() ? n.high() : n.low();
    }

    inline void visit(const bool s)
    { result = s; }

    inline bool get_result()
    { return result; }
  };

  bool bdd_eval(const bdd &bdd,
                const shared_file<map_pair<bdd::label_t, boolean>> &mpf)
  {
    bdd_eval_file_visitor v(mpf);
    internal::traverse(bdd, v);
    return v.get_result();
  }

  //////////////////////////////////////////////////////////////////////////////
  class bdd_sat_bdd_callback
  {
    using e = map_pair<bdd::label_t, boolean>;

    shared_file<e> ef;
    internal::file_writer<e> ew;

  public:
    bdd_sat_bdd_callback() : ew(ef)
    { }

    void operator() (bdd::label_t x, bool v)
    { ew << map_pair<bdd::label_t, boolean>(x, v); }

    bdd get_bdd()
    {
      assert(ew.size() > 0);
      ew.detach();
      internal::file_stream<e, true> es(ef);

      internal::shared_levelized_file<bdd::node_t> nf;
      internal::node_writer nw(nf);

      bdd::ptr_t latest = bdd::ptr_t(true);

      while (es.can_pull()) {
        const e kv = es.pull();

        const bdd::node_t n(kv.key(), bdd::MAX_ID,
                            kv.value() ? bdd::ptr_t(false) : latest,
                            kv.value() ? latest : bdd::ptr_t(false));

        nw.unsafe_push(n);
        nw.unsafe_push(internal::level_info(kv.key(), 1u));

        latest = n.uid();
      }

      nf->max_1level_cut[internal::cut_type::INTERNAL]       = 1u;
      nf->max_1level_cut[internal::cut_type::INTERNAL_FALSE] = nf->number_of_terminals[false];
      nf->max_1level_cut[internal::cut_type::INTERNAL_TRUE]  = 1u; // == nf->number_of_terminals[true]
      nf->max_1level_cut[internal::cut_type::ALL]            = nf->number_of_terminals[false] + 1u;

      return nf;
    }
  };

  class bdd_sat_lambda_callback
  {
    const std::function<void(bdd::label_t, bool)> &__lambda;

  public:
    bdd_sat_lambda_callback(const std::function<void(bdd::label_t, bool)> &lambda)
      : __lambda(lambda)
    { }

    void operator() (bdd::label_t x, bool v) const
    { __lambda(x,v); }
  };

  template<typename base_visitor, typename callback>
  class bdd_sat_visitor
  {
    base_visitor  __visitor;
    callback     &__callback;

    internal::level_info_stream<> lvls;

  public:
    bdd_sat_visitor(const bdd& f, callback &cb)
      : __callback(cb), lvls(f) // <-- TODO: use domain if available
    { }

    bdd::ptr_t visit(const bdd::node_t &n)
    {
      const bdd::ptr_t next_ptr = __visitor.visit(n);
      const bdd::label_t label = n.label();

      // set default to all skipped levels
      while (lvls.peek().label() < label) {
        __callback(lvls.pull().label(), base_visitor::default_direction);
      }
      adiar_debug(lvls.peek().label() == label,
                  "level given should exist in BDD");

      __callback(lvls.pull().label(), next_ptr == n.high());
      return next_ptr;
    }

    void visit(const bool s)
    {
      __visitor.visit(s);

      while (lvls.can_pull()) {
        __callback(lvls.pull().label(), base_visitor::default_direction);
      }
    }
  };

  bdd bdd_satmin(const bdd &f)
  {
    if (is_terminal(f)) { return f; }

    bdd_sat_bdd_callback __cb;
    bdd_sat_visitor<internal::traverse_satmin_visitor, bdd_sat_bdd_callback> v(f, __cb);
    internal::traverse(f,v);
    return __cb.get_bdd();
  }

  void bdd_satmin(const bdd &f,
                  const std::function<void(bdd::label_t, bool)> &cb)
  {
    bdd_sat_lambda_callback __cb(cb);
    bdd_sat_visitor<internal::traverse_satmin_visitor, bdd_sat_lambda_callback> v(f, __cb);
    internal::traverse(f,v);
  }

  bdd bdd_satmax(const bdd &f)
  {
    if (is_terminal(f)) { return f; }

    bdd_sat_bdd_callback __cb;
    bdd_sat_visitor<internal::traverse_satmax_visitor, bdd_sat_bdd_callback> v(f, __cb);
    internal::traverse(f,v);
    return __cb.get_bdd();
  }

  void bdd_satmax(const bdd &f,
                  const std::function<void(bdd::label_t, bool)> &cb)
  {
    bdd_sat_lambda_callback __cb(cb);
    bdd_sat_visitor<internal::traverse_satmax_visitor, bdd_sat_lambda_callback> v(f, __cb);
    internal::traverse(f,v);
  }
}
