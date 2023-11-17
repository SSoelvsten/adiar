#include <adiar/bdd.h>

#include <tpie/tpie.h>
#include <tpie/internal_stack.h>

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
  class bdd_eval_generator_visitor
  {
    const generator<pair<bdd::label_type, bool>> &_generator;

    // The `_next_pair` value is only queried if there actually are BDD nodes.
    // Hence, we may as well unwrap the `optional<...>`.
    pair<bdd::label_type, bool> _next_pair;

    bool result = false;

  public:
    bdd_eval_generator_visitor(const generator<pair<bdd::label_type, bool>>& g)
      : _generator(g)
    {
      const optional<pair<bdd::label_type, bool>> p = _generator();
      if (p) { _next_pair = p.value(); }
    }

    inline bdd::pointer_type visit(const bdd::node_type &n)
    {
      const bdd::label_type level = n.label();

      while (_next_pair.first < level) {
        const optional<pair<bdd::label_type, bool>> p = _generator();

        if (!p) {
          throw out_of_range("Labels are insufficient to traverse BDD");
        }

        if (p.value().first <= _next_pair.first) {
          throw invalid_argument("Labels are not in ascending order");
        }

        _next_pair = p.value();
      }

      if (_next_pair.first != level) {
        throw invalid_argument("Missing assignment for node visited in BDD");
      }

      return n.child(_next_pair.second);
    }

    inline void visit(const bool s)
    { result = s; }

    inline bool get_result()
    { return result; }
  };

  bool bdd_eval(const bdd &bdd, const generator<pair<bdd::label_type, bool>> &xs)
  {
    bdd_eval_generator_visitor v(xs);
    internal::traverse(bdd, v);
    return v.get_result();
  }

  //////////////////////////////////////////////////////////////////////////////
  class bdd_sat_bdd_callback
  {
    // Reserve an internal memory vector (of up to 8 MiB) for the result.
    // TODO: Abstract the stack into <adiar/internal/data_structures/stack.h>.
    using value_type = pair<bdd::label_type, bool>;
    using stack_type = tpie::internal_stack<value_type>;

    stack_type _stack;

  public:
    bdd_sat_bdd_callback(const size_t stack_size)
      : _stack(std::min<size_t>(stack_size, bdd::max_label+1))
    { }

    void operator() (bdd::label_type x, bool v)
    {
      _stack.push({x, !v});
    }

    bdd get_bdd()
    {
      adiar_assert(!_stack.empty());

      return bdd_cube([this]() {
        if (this->_stack.empty()) { return make_optional<value_type>(); }

        const value_type top = this->_stack.top();
        this->_stack.pop();
        return make_optional<value_type>(top);
      });
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

    bdd_sat_bdd_callback _cb(f->levels() + domain_size());
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

    bdd_sat_bdd_callback _cb(f->levels() + domain_size());
    __bdd_satX<internal::traverse_satmax_visitor>(f, _cb);
    return _cb.get_bdd();
  }

  void bdd_satmax(const bdd &f, const consumer<bdd::label_type, bool> &cb)
  {
    bdd_sat_lambda_callback _cb(cb);
    __bdd_satX<internal::traverse_satmax_visitor>(f, _cb);
  }
}
