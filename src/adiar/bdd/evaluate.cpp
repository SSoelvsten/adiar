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
  template<typename Visitor>
  class bdd_satX__stack
  {
  private:
    Visitor _visitor;

    // Reserve an internal memory vector (of up to 8 MiB) for the result.
    // TODO: Abstract the stack into <adiar/internal/data_structures/stack.h>.
    using value_type = pair<bdd::label_type, bool>;
    using stack_type = tpie::internal_stack<value_type>;

    stack_type _stack;

  public:
    bdd_satX__stack(const size_t stack_size)
      : _stack(stack_size)
    { }

    bdd::pointer_type visit(const bdd::node_type &n)
    {
      const bdd::pointer_type next_ptr = _visitor.visit(n);
      _stack.push({n.label(), next_ptr == n.low()});
      return next_ptr;
    }

    void visit(const bool s)
    {
      _visitor.visit(s);
    }

  public:
    bdd build_bdd()
    {
      adiar_assert(!_stack.empty());

      // TODO (optimisation): Use 'internal::build_chain' directly to inline
      //                      popping values from the stack.
      return bdd_cube([this]() {
        if (this->_stack.empty()) { return make_optional<value_type>(); }

        const value_type top = this->_stack.top();
        this->_stack.pop();
        return make_optional<value_type>(top);
      });
    }
  };

  template<typename Visitor>
  class bdd_satX__functional
  {
  private:
    Visitor _visitor;
    const consumer<bdd::label_type, bool> &_consumer;

  public:
    bdd_satX__functional(const consumer<bdd::label_type, bool> &c)
      : _consumer(c)
    { }

    bdd::pointer_type visit(const bdd::node_type &n)
    {
      const bdd::pointer_type next_ptr = _visitor.visit(n);
      _consumer(n.label(), next_ptr == n.high());
      return next_ptr;
    }

    void visit(const bool s)
    {
      _visitor.visit(s);
    }
  };

  template<typename Visitor>
  bdd __bdd_satX(const bdd &f)
  {
    if (bdd_isterminal(f)) { return f; }

    bdd_satX__stack<Visitor> v(f->levels());
    internal::traverse(f, v);

    return v.build_bdd();
  }

  template<typename Visitor>
  void __bdd_satX(const bdd &f, const consumer<bdd::label_type, bool> &c)
  {
    bdd_satX__functional<Visitor> v(c);
    internal::traverse(f, v);
  }

  bdd bdd_satmin(const bdd &f)
  {
    return __bdd_satX<internal::traverse_satmin_visitor>(f);
  }

  void bdd_satmin(const bdd &f, const consumer<bdd::label_type, bool> &c)
  {
    return __bdd_satX<internal::traverse_satmin_visitor>(f, c);
  }

  bdd bdd_satmax(const bdd &f)
  {
    return __bdd_satX<internal::traverse_satmax_visitor>(f);
  }

  void bdd_satmax(const bdd &f, const consumer<bdd::label_type, bool> &c)
  {
    return __bdd_satX<internal::traverse_satmax_visitor>(f, c);
  }
}
