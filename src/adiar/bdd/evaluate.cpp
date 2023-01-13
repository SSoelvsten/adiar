#include <adiar/bdd.h>

#include <adiar/assignment.h>
#include <adiar/evaluation.h>
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
    const assignment_func &af;
    bool result = false;;

  public:
    bdd_eval_func_visitor(const assignment_func& f) : af(f)
    { }

    inline bdd::ptr_t visit(const bdd::node_t &n)
    {
      const assignment_value a = af(n.label());
      adiar_assert(a != assignment_value::NONE,
                   "Missing assignment for node visited in BDD");

      return a == assignment_value::TRUE ? n.high() : n.low();
    }

    inline void visit(const bool s)
    { result = s; }

    inline bool get_result()
    { return result; }
  };

  bool bdd_eval(const bdd &bdd, const assignment_func &af)
  {
    bdd_eval_func_visitor v(af);
    traverse(bdd, v);
    return v.get_result();
  }

  class bdd_eval_file_visitor
  {
    internal::file_stream<assignment> as;
    assignment a;

    bool result = false;

  public:
    bdd_eval_file_visitor(const shared_file<assignment>& af) : as(af)
    { if (as.can_pull()) { a = as.pull(); } }

    inline bdd::ptr_t visit(const bdd::node_t &n)
    {
      const bdd::label_t level = n.label();
      while (a.level() < level) {
        adiar_assert(as.can_pull(),
                     "Assignment file is insufficient to traverse BDD");
        a = as.pull();
      }
      adiar_assert(a.level() == level,
                   "Missing assignment for node visited in BDD");
      adiar_assert(a.value() != assignment_value::NONE,
                   "Missing assignment for node visited in BDD");

      return a.is_true() ? n.high() : n.low();
    }

    inline void visit(const bool s)
    { result = s; }

    inline bool get_result()
    { return result; }
  };

  bool bdd_eval(const bdd &bdd, const shared_file<assignment> &af)
  {
    bdd_eval_file_visitor v(af);
    internal::traverse(bdd, v);
    return v.get_result();
  }

  //////////////////////////////////////////////////////////////////////////////
  template<typename visitor, evaluation_value skipped_value>
  class bdd_sat_evaluation_writer_visitor
  {
    visitor __visitor;

    shared_file<evaluation> ef;
    internal::file_writer<evaluation> ew;

    internal::level_info_stream<> lvls;

  public:
    bdd_sat_evaluation_writer_visitor(const bdd& f)
      : ew(ef), lvls(f)
    { }

    bdd::ptr_t visit(const bdd::node_t &n)
    {
      const bdd::ptr_t next_ptr = __visitor.visit(n);
      const bdd::label_t label = n.label();

      // set default to all skipped levels
      while (lvls.peek().label() < label) {
        ew << evaluation(lvls.pull().label(), skipped_value);
      }
      adiar_debug(lvls.peek().label() == label,
                  "level given should exist in BDD");

      ew << evaluation(lvls.pull().label(), next_ptr == n.high());
      return next_ptr;
    }

    void visit(const bool s)
    {
      __visitor.visit(s);

      while (lvls.can_pull()) {
        ew << evaluation(lvls.pull().label(), skipped_value);
      }
    }

    const shared_file<evaluation> get_result() const
    {
      return ef;
    }
  };

  shared_file<evaluation> bdd_satmin(const bdd &f)
  {
    bdd_sat_evaluation_writer_visitor<internal::traverse_satmin_visitor, evaluation_value::FALSE> v(f);
    internal::traverse(f,v);
    return v.get_result();
  }

  shared_file<evaluation> bdd_satmax(const bdd &f)
  {
    bdd_sat_evaluation_writer_visitor<internal::traverse_satmax_visitor, evaluation_value::TRUE> v(f);
    internal::traverse(f,v);
    return v.get_result();
  }
}
