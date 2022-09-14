#include <adiar/bdd.h>

#include <adiar/data.h>

#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/algorithms/traverse.h>

namespace adiar
{
  template<typename visitor, bool skipped_value>
  class bdd_sat_assignment_writer_visitor
  {
    visitor __visitor;

    assignment_file af;
    assignment_writer aw;

    level_info_stream<node_t> ms;

  public:
    bdd_sat_assignment_writer_visitor(const bdd& f) : aw(af), ms(f) { }

    ptr_t visit(const node_t &n)
    {
      const ptr_t next_ptr = __visitor.visit(n);
      const label_t label = label_of(n);

      // set default to all skipped levels
      while (label_of(ms.peek()) < label) {
        aw << create_assignment(label_of(ms.pull()), skipped_value);
      }
      adiar_debug(label_of(ms.peek()) == label,
                  "level given should exist in BDD");

      aw << create_assignment(label_of(ms.pull()), next_ptr == n.high);
      return next_ptr;
    }

    void visit(const bool s)
    {
      __visitor.visit(s);

      while (ms.can_pull()) {
        aw << create_assignment(label_of(ms.pull()), skipped_value);
      }
    }

    const assignment_file get_result() const
    {
      return af;
    }
  };

  assignment_file bdd_satmin(const bdd &f)
  {
    bdd_sat_assignment_writer_visitor<traverse_satmin_visitor, false> v(f);
    traverse(f,v);
    return v.get_result();
  }

  assignment_file bdd_satmax(const bdd &f)
  {
    bdd_sat_assignment_writer_visitor<traverse_satmax_visitor, true> v(f);
    traverse(f,v);
    return v.get_result();
  }
}
