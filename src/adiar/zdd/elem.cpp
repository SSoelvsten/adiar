#include <adiar/zdd.h>

#include <adiar/internal/algorithms/traverse.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/data_types/uid.h>
#include <adiar/internal/io/file.h>
#include <adiar/internal/io/file_stream.h>
#include <adiar/internal/io/file_writer.h>

namespace adiar
{
  class zdd_sat_zdd_callback
  {
    shared_file<zdd::label_t> lf;
    internal::label_writer lw;

  public:
    zdd_sat_zdd_callback()
      : lw(lf)
    { }

    void operator() (zdd::label_t x)
    { lw << x; }

    zdd get_zdd()
    {
      lw.detach();
      return zdd_vars(lf);
    }
  };

  class zdd_sat_lambda_callback
  {
    const std::function<void(bdd::label_t)> &__lambda;

  public:
    zdd_sat_lambda_callback(const std::function<void(zdd::label_t)> &lambda)
      : __lambda(lambda)
    { }

    void operator() (zdd::label_t x) const
    { __lambda(x); }
  };

  template<typename visitor_t, typename callback_t>
  class zdd_sat_visitor
  {
  private:
    visitor_t   _visitor;
    callback_t& _callback;

  public:
    zdd_sat_visitor() = delete;
    zdd_sat_visitor(const zdd_sat_visitor&) = delete;
    zdd_sat_visitor(zdd_sat_visitor&&) = delete;

    zdd_sat_visitor(callback_t &cb)
      : _callback(cb)
    { }

    zdd::ptr_t visit(const zdd::node_t n)
    {
      adiar_assert(!n.high().is_terminal() || n.high().value(), "high terminals are never false");
      const zdd::ptr_t next_ptr = _visitor.visit(n);

      if (next_ptr == n.high() && (next_ptr != n.low() || visitor_t::default_direction)) {
        _callback(n.label());
      }

      return next_ptr;
    }

    void visit(const bool s)
    { _visitor.visit(s); }
  };

  zdd zdd_minelem(const zdd &A)
  {
    zdd_sat_zdd_callback _cb;
    zdd_sat_visitor<internal::traverse_satmin_visitor, zdd_sat_zdd_callback> v(_cb);
    internal::traverse(A, v);
    return _cb.get_zdd();
  }

  void zdd_minelem(const zdd &A,
                  const std::function<void(zdd::label_t)> &cb)
  {
    zdd_sat_lambda_callback _cb(cb);
    zdd_sat_visitor<internal::traverse_satmin_visitor, zdd_sat_lambda_callback> v(_cb);
    internal::traverse(A, v);
  }

  zdd zdd_maxelem(const zdd &A)
  {
    zdd_sat_zdd_callback _cb;
    zdd_sat_visitor<internal::traverse_satmax_visitor, zdd_sat_zdd_callback> v(_cb);
    internal::traverse(A, v);
    return _cb.get_zdd();
  }

  void zdd_maxelem(const zdd &A,
                   const std::function<void(zdd::label_t)> &cb)
  {
    zdd_sat_lambda_callback _cb(cb);
    zdd_sat_visitor<internal::traverse_satmax_visitor, zdd_sat_lambda_callback> v(_cb);
    internal::traverse(A, v);
  }
}
