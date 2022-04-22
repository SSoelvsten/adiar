#include <adiar/bdd.h>
#include <adiar/bdd/bdd_policy.h>

#include <adiar/data.h>

#include <adiar/file.h>
#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/substitution.h>
#include <adiar/internal/util.h>

namespace adiar
{

  struct reorder_request
  {
    ptr_t source; // gemmer også is_high (brug flag, unflag, is_flagged)
    label_t child_level;
  };

  struct reorder_lt
  {
    // when doing the expensive comparison, we might need to make a lambda so the arc file is in scope
    bool operator()(const reorder_request &a, const reorder_request &b) const
    {
      return a.child_level < b.child_level;
    }
  };

  __bdd bdd_reorder(const bdd &dd)
  {
    // prøv levelized_priority_queue for UNLIMITED POWER
    external_priority_queue<reorder_request, reorder_lt> pq(memory::available(), 0); // 0 is pq external doesnt care

    /*
    Sådan pusher vi arcs (substitute.h i internal/substitution.h))):
    while(substitute_pq.can_pull() && substitute_pq.top().target == n_res.uid) {
      const arc_t parent_arc = substitute_pq.pull();

      if(!is_nil(parent_arc.source)) {
        aw.unsafe_push_node(parent_arc);
      }
    }
    */

    return bdd_sink(false);
  }

  void push_children(external_priority_queue<reorder_request, reorder_lt> &pq, const ptr_t source, arc_file &af, const bdd &f)
  {
    auto deal_with = [&](bool b)
    {
      assignment_file path = reverse_path(af, source);
      assignment_writer aw(path);
      aw.unsafe_push(assignment_t{label_of(source), b});

      bdd f_ikb = bdd_restrict(f, path);
      label_t label = min_label(f_ikb);

      bool is_leaf = label == -1;
      if (!is_leaf)
      {
        pq.push(reorder_request{source, label});
      }
      else
      {
        arc_writer aw(af);
        aw.unsafe_push(arc_t{source, create_sink_ptr(value_of(f_ikb))});
      }
    };

    deal_with(true);
    deal_with(false);
  }

  // N/B I/Os
  label_t min_label(const bdd &dd)
  {
    if (is_sink(dd))
    {
      return -1; // TODO: nodes are indexed from 0 - change
    }

    label_t result = UINT_MAX;
    adiar::node_stream<> fs(dd); // TODO: Brug levelinfostream i stedet (meget mindre)

    while (fs.can_pull())
    {
      node_t node = fs.pull();
      if (!is_sink(node))
      {
        label_t label = label_of(node);
        if (label < result)
        {
          result = label;
        }
      }
    }
    return result;
  }

  // T/B I/Os
  assignment_file reverse_path(const arc_file &af, ptr_t n)
  {
    assignment_file ass_file;
    assignment_writer aw(ass_file);

    // TODO: This node_arc_stream should read backwards as our reorder implementation
    //       will always write the arc of the root first in the file.
    adiar::node_arc_stream<false> fs(af); // true means that we read it backwards

    // Måske virker node_arc_stream ikke.. (Detach er ikke defineret)
    // Denne node_arc_stream er ikke beregnet til at at read, write, read, write
    // Vi tror, det er muligt, men måske render vi ind i problemer senere.
    ptr_t current = n; // Hacky solution - is this valid

    std::cout << "Starting arc: " << n << std::endl;

    while (fs.can_pull())
    {
      arc_t a = fs.pull();
      std::cout << "Looking  source: " << a.source << " - target: " << a.target << std::endl;
      if (a.target == current)
      {
        current = a.source;
        aw.unsafe_push(assignment{label_of(current), is_high(a)});
      }
    }

    fs.detach(); // TODO: Make issue about missing detach impl.
    return ass_file;
  }
}
