#include <adiar/bdd.h>

#include <adiar/assignment.h>
#include <adiar/internal/io/file_stream.h>
#include <adiar/internal/io/file_writer.h>
#include <adiar/internal/io/levelized_file_stream.h>
#include <adiar/internal/algorithms/traverse.h>
#include <adiar/internal/data_types/level_info.h>

namespace adiar
{
  template<typename visitor, bool skipped_value>
  class bdd_sat_assignment_writer_visitor
  {
    visitor __visitor;

    shared_file<assignment_t> af;
    internal::assignment_writer aw;

    internal::level_info_stream<> lvls;

  public:
    bdd_sat_assignment_writer_visitor(const bdd& f)
      : aw(af), lvls(f)
    { }

    bdd::ptr_t visit(const bdd::node_t &n)
    {
      const bdd::ptr_t next_ptr = __visitor.visit(n);
      const bdd::label_t label = n.label();

      // set default to all skipped levels
      while (lvls.peek().label() < label) {
        aw << create_assignment(lvls.pull().label(), skipped_value);
      }
      adiar_debug(lvls.peek().label() == label,
                  "level given should exist in BDD");

      aw << create_assignment(lvls.pull().label(), next_ptr == n.high());
      return next_ptr;
    }

    void visit(const bool s)
    {
      __visitor.visit(s);

      while (lvls.can_pull()) {
        aw << create_assignment(lvls.pull().label(), skipped_value);
      }
    }

    const shared_file<assignment_t> get_result() const
    {
      return af;
    }
  };

  shared_file<assignment_t> bdd_satmin(const bdd &f)
  {
    bdd_sat_assignment_writer_visitor<internal::traverse_satmin_visitor, false> v(f);
    internal::traverse(f,v);
    return v.get_result();
  }

  shared_file<assignment_t> bdd_satmax(const bdd &f)
  {
    bdd_sat_assignment_writer_visitor<internal::traverse_satmax_visitor, true> v(f);
    internal::traverse(f,v);
    return v.get_result();
  }
}
