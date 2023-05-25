#include <adiar/bdd.h>

#include <functional>

#include <adiar/assignment.h>
#include <adiar/internal/assert.h>
#include <adiar/internal/algorithms/traverse.h>
#include <adiar/internal/io/file_stream.h>
#include <adiar/internal/io/file_writer.h>
#include <adiar/internal/io/levelized_file_stream.h>
#include <adiar/internal/data_types/level_info.h>

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
  class bdd_sat_file_callback
  {
    shared_file<map_pair<bdd::label_t, boolean>> ef;
    internal::file_writer<map_pair<bdd::label_t, boolean>> ew;

  public:
    bdd_sat_file_callback() : ew(ef)
    { }

    void operator() (bdd::label_t x, bool v)
    { ew << map_pair<bdd::label_t, boolean>(x, v); }

    // TODO: replace with 'get_bdd()' conversion into BDD
    const shared_file<map_pair<bdd::label_t, boolean>> get_file() const
    { return ef; }
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

  shared_file<map_pair<bdd::label_t, boolean>> bdd_satmin(const bdd &f)
  {
    bdd_sat_file_callback __cb;
    bdd_sat_visitor<internal::traverse_satmin_visitor, bdd_sat_file_callback> v(f, __cb);
    internal::traverse(f,v);
    return __cb.get_file();
  }

  void bdd_satmin(const bdd &f,
                  const std::function<void(bdd::label_t, bool)> &cb)
  {
    bdd_sat_lambda_callback __cb(cb);
    bdd_sat_visitor<internal::traverse_satmin_visitor, bdd_sat_lambda_callback> v(f, __cb);
    internal::traverse(f,v);
  }

  shared_file<map_pair<bdd::label_t, boolean>> bdd_satmax(const bdd &f)
  {
    bdd_sat_file_callback __cb;
    bdd_sat_visitor<internal::traverse_satmax_visitor, bdd_sat_file_callback> v(f, __cb);
    internal::traverse(f,v);
    return __cb.get_file();
  }

  void bdd_satmax(const bdd &f,
                  const std::function<void(bdd::label_t, bool)> &cb)
  {
    bdd_sat_lambda_callback __cb(cb);
    bdd_sat_visitor<internal::traverse_satmax_visitor, bdd_sat_lambda_callback> v(f, __cb);
    internal::traverse(f,v);
  }
}
