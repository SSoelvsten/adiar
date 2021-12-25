#include "evaluate.h"

#include <adiar/file_stream.h>
#include <adiar/internal/traverse.h>

#include <adiar/assert.h>

namespace adiar
{
  class bdd_eval_func_visitor
  {
    const assignment_func &af;
    bool result;

  public:
    bdd_eval_func_visitor(const assignment_func& f) : af(f)
    { }

    inline bool visit(const node_t &n)
    {
      return af(label_of(n));
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

  //////////////////////////////////////////////////////////////////////////////
  class bdd_eval_file_visitor
  {
    assignment_stream<> as;
    assignment_t a;

    bool result;

  public:
    bdd_eval_file_visitor(const assignment_file& af) : as(af)
    { if (as.can_pull()) { a = as.pull(); } }

    inline bool visit(const node_t &n)
    {
      const label_t label = label_of(n);
      while (label_of(a) < label) {
        adiar_assert(as.can_pull(), "Given assignment file is insufficient to traverse BDD");
        a = as.pull();
      }
      adiar_assert(label_of(a) == label, "Missing assignment for node visited in BDD");

      return value_of(a);
    }

    inline void visit(const bool s)
    { result = s; }

    inline bool get_result()
    { return result; }
  };

  bool bdd_eval(const bdd &bdd, const assignment_file &af)
  {
    bdd_eval_file_visitor v(af);
    traverse(bdd, v);
    return v.get_result();
  }
}
