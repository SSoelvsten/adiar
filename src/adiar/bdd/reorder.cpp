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

  /*
  struct level_lt
  {
    std::function<bool(reorder_request, reorder_request)> op;
    level_lt(std::function<bool(reorder_request, reorder_request)> op){
      this->op = op;
    };

    bool operator()(const reorder_request &a, const reorder_request &b) const
    {
      return op(a, b);
    }
  };
  */

  __bdd bdd_reorder(const bdd &dd, const label_t permutation[])
  {
    // prøv levelized_priority_queue for UNLIMITED POWER
    external_priority_queue<reorder_request, reorder_lt> pq(memory::available(), 0); // 0 is pq external doesnt care

    arc_file af;
    ptr_t root = create_node_ptr(permutation[0], 0);
    push_children(pq, root, af, dd);

    while (!pq.empty())
    {
      //TODO https://github.com/Mortal/tpieex/blob/master/main.cc line 68-70
      //TODO add operator as argument in the constructor.

      auto pred = [&](const reorder_request &a, const reorder_request &b) -> bool {
        assignment_file path = reverse_path(af, a.source);
        bdd a_restrict = bdd_restrict(dd, path);
        
        assignment_file path = reverse_path(af, b.source);
        bdd b_restrict = bdd_restrict(dd, path);

        // TODO Add N/B less than operator.

        return false;
      };

      tpie::merge_sorter<reorder_request, false, decltype(pred)> m_sorter(pred);
      m_sorter.set_available_memory(memory::available());
      m_sorter.begin();

      reorder_request rr = pq.top();
      pq.pop();
      m_sorter.push(rr);
      while (pq.top().child_level == rr.child_level)
      {
        reorder_request next = pq.top();
        pq.pop();
        m_sorter.push(rr);
      }

    }

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

  void push_children(external_priority_queue<reorder_request, reorder_lt> &pq, const ptr_t source, const arc_file &af, const bdd &f)
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
  // TODO retrive from meta data
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
  // TODO reverse path of two arc at once
  assignment_file reverse_path(const arc_file &af, ptr_t n)
  {
    assignment_file ass_file;
    assignment_writer aw(ass_file);

    adiar::node_arc_stream<> fs(af); 

    // Måske virker node_arc_stream ikke.. (Detach er ikke defineret)
    // Denne node_arc_stream er ikke beregnet til at at read, write, read, write
    // Vi tror, det er muligt, men måske render vi ind i problemer senere.
    ptr_t current = n;

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

    return ass_file;
  }
}
