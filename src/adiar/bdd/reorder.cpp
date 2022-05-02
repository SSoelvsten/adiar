#include <adiar/bdd.h>
#include <adiar/bdd/bdd_policy.h>

#include <adiar/data.h>

#include <adiar/file.h>
#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/substitution.h>
#include <adiar/internal/util.h>

#define PRINT 0

namespace adiar
{
  std::vector<label_t> perm;
  std::vector<label_t> perm_inv;

  struct reorder_request
  {
    ptr_t source;
    label_t child_level;
  };

  struct reorder_lt
  {
    bool operator()(const reorder_request &arc, const reorder_request &b) const
    {
      return arc.child_level < b.child_level;
    }
  };

  void debug_log(const std::string &msg, int tabs)
  {
#if PRINT
    for (int i = 0; i < tabs; i++)
      std::cout << "  ";
    std::cout << msg << std::endl;
#endif
  }

  // T/B I/Os
  // TODO reverse path of two arc at once
  assignment_file reverse_path(const arc_file &af, ptr_t n, const assignment_t ass_of_n, const bool extra_ass)
  {
    assignment_file ass_file;
    {
      assignment_writer aw(ass_file);

      adiar::node_arc_stream<> fs(af);

      // Måske virker node_arc_stream ikke.. (Detach er ikke defineret)
      // Denne node_arc_stream er ikke beregnet til at at read, write, read, write
      // Vi tror, det er muligt, men måske render vi ind i problemer senere.
      ptr_t current = n;

      debug_log("REVERSE PATH: Starting arc: " + std::to_string(n), 2);

      while (fs.can_pull())
      {
        arc_t arc = fs.pull();
        debug_log("REVERSE PATH: Looking @ source: " + std::to_string(arc.source) + " --" + std::to_string(is_flagged(arc.source)) + "--> target: " + std::to_string(arc.target), 2);
        if (arc.target == current)
        {
          current = unflag(arc.source);
          aw.unsafe_push(assignment{perm[label_of(current)], is_high(arc)});
        }
      }
      af.make_writeable();
      if (extra_ass)
      {
        aw.unsafe_push(ass_of_n);
      }
    }

    // TODO test if this is needed!
    // As the reverse_path no longer only takes T/B I/Os
    simple_file_sorter<assignment_t> sf_sorter;
    sf_sorter.sort(ass_file);

    return ass_file;
  }

  assignment_file reverse_path(const arc_file &af, ptr_t n)
  {
    return reverse_path(af, n, assignment_t{0, 0}, false);
  }

  assignment_file reverse_path(const arc_file &af, ptr_t n, const assignment_t ass_of_n)
  {
    return reverse_path(af, n, ass_of_n, true);
  }

  std::tuple<assignment_file, assignment_file> dual_reverse_path(const arc_file &af, ptr_t n, ptr_t m)
  {
    assignment_file ass_file_n;
    assignment_file ass_file_m;

    {
      assignment_writer aw_n(ass_file_n);
      assignment_writer aw_m(ass_file_m);

      adiar::node_arc_stream<> fs(af);

      ptr_t current_n = n;
      ptr_t current_m = m;

      debug_log("DUAL REVERSE PATH: Starting arc_n: " + std::to_string(n) + " arc_m: " + std::to_string(m), 2);

      while (fs.can_pull())
      {
        arc_t arc = fs.pull();
        debug_log("DUAL REVERSE PATH: Looking @ source: " + std::to_string(arc.source) + " --" + std::to_string(is_flagged(arc.source)) + "--> target: " + std::to_string(arc.target), 2);
        if (arc.target == current_n)
        {
          current_n = unflag(arc.source);
          aw_n.unsafe_push(assignment{perm[label_of(current_n)], is_high(arc)});
        }
        if (arc.target == current_m)
        {
          current_m = unflag(arc.source);
          aw_m.unsafe_push(assignment{perm[label_of(current_m)], is_high(arc)});
        }
      }
      af.make_writeable();
    }

    simple_file_sorter<assignment_t> sf_sorter;
    sf_sorter.sort(ass_file_n);
    sf_sorter.sort(ass_file_m);

    return {ass_file_n, ass_file_m};
  }

  // N/B I/Os
  // TODO retrive from meta data
  label_t min_label(const bdd &dd)
  {
    if (is_sink(dd))
    {
      return 0xffffffff;
    }

    label_t result = UINT_MAX;
    adiar::node_stream<> fs(dd); // TODO: Brug levelinfostream i stedet (meget mindre)

    while (fs.can_pull())
    {
      node_t node = fs.pull();
      if (!is_sink(node))
      {
        label_t label = perm_inv[label_of(node)];
        if (label < result)
        {
          result = label;
        }
      }
    }
    return result;
  }

  void push_children(external_priority_queue<reorder_request, reorder_lt> &pq, const ptr_t source, const arc_file &af, const bdd &f)
  {
    auto deal_with = [&](bool b)
    {
      assignment_file path = reverse_path(af, source, assignment_t{perm[label_of(source)], b});
      debug_log("PUSH-CHILDREN: reverse path done", 1);
#if PRINT
      {
        std::cout << "  PUSH-CHILDREN: reverse_path Assignment = (";
        assignment_stream<> as(path);
        while (as.can_pull())
        {
          assignment_t arc = as.pull();
          std::cout << arc.label << " = " << arc.value << ", ";
        }
        std::cout << ")" << std::endl;
      }
#endif

      bdd f_ikb = bdd_restrict(f, path);
      debug_log("PUSH-CHILDREN: restriction done", 1);
      label_t label = min_label(f_ikb);
      debug_log("PUSH-CHILDREN: min-label done", 1);

      bool is_leaf = label == 0xffffffff;
      if (!is_leaf)
      {
        debug_log("PUSH-CHILDREN: pushing non-leaf", 1);
        ptr_t src;
        if (b)
          src = flag(source);
        else
          src = unflag(source);
        pq.push(reorder_request{src, label});
        debug_log("RR: {" + std::to_string(src) + ", " + std::to_string(label) + "}", 1);
      }
      else
      {
        ptr_t src;
        if (b)
          src = flag(source);
        else
          src = unflag(source);
        debug_log("PUSH-CHILDREN: writing arc to leaf", 1);
        arc_writer aw(af);
        aw.unsafe_push(arc_t{src, create_sink_ptr(value_of(f_ikb))});
      }
    };

    deal_with(false);
    deal_with(true);
  }

  void init_permutation(const std::vector<label_t> permutation)
  {
    perm = permutation;
    perm_inv = std::vector<label_t>(permutation.size(), 0);
    for (unsigned long i = 0; i < permutation.size(); i++)
    {
      perm_inv[permutation[i]] = i;
    }
  }

  node_file convert_arc_file_to_node_file(const arc_file &af)
  {
    simple_file<arc_t> all_arcs;
    {
      simple_file_writer<arc_t> sfw(all_arcs);

      node_arc_stream<true> as(af);
      while (as.can_pull())
      {
        arc_t arc = as.pull();
        sfw.push(arc);
      }

      sink_arc_stream<true> ass(af);
      while (ass.can_pull())
      {
        arc_t arc = ass.pull();
        sfw.push(arc);
      }
    }

    auto pred_arc = [](const arc_t &arc, const arc_t &b) -> bool
    { return b.source < arc.source; };
    simple_file_sorter<arc_t, decltype(pred_arc)> sfs;
    sfs.sort(all_arcs, pred_arc);

    node_file nodes;
    node_writer nw(nodes);

    file_stream<arc_t> fs(all_arcs);
    while (fs.can_pull())
    {
      arc_t arc1 = fs.pull();
      arc_t arc2 = fs.pull();
      node n = node_of(arc2, arc1);
      nw.push(n);
    }
    return nodes;
  }

  __bdd bdd_reorder(const bdd &dd, const std::vector<label_t> permutation)
  {
    init_permutation(permutation);

    debug_log("Reorder started", 0);

    // prøv levelized_priority_queue for UNLIMITED POWER
    external_priority_queue<reorder_request, reorder_lt> pq(memory::available() / 2, 0); // 0 is pq external doesnt care

    arc_file af;
    ptr_t root = create_node_ptr(0, 0);
    push_children(pq, root, af, dd);

    debug_log("Initialization done", 0);

    while (!pq.empty())
    {
      debug_log("PQ loop", 0);

      // TODO https://github.com/Mortal/tpieex/blob/master/main.cc line 68-70
      // TODO add operator as argument in the constructor.
      auto pred = [&](const reorder_request &arc, const reorder_request &b) -> bool
      {
        assignment_file path_a, path_b;
        std::tie(path_a, path_b) = dual_reverse_path(af, arc.source, b.source);

        bdd a_restrict = bdd_restrict(dd, path_a);

        bdd b_restrict = bdd_restrict(dd, path_b);

        node_stream<> a_ns(a_restrict);
        node_stream<> b_ns(b_restrict);

        // Is A < B?
        while (a_ns.can_pull())
        {
          if (b_ns.can_pull())
          {
            node_t a_node = a_ns.pull();
            node_t b_node = b_ns.pull();

            if (a_node != b_node)
            {
              return a_node < b_node;
            }
            continue;
          }
          return false;
        }
        return true;
      };

      tpie::merge_sorter<reorder_request, false, decltype(pred)> m_sorter(pred);
      m_sorter.set_available_memory(memory::available() / 2);
      tpie::dummy_progress_indicator d_indicator;
      m_sorter.begin();

      reorder_request rr = pq.top();
      pq.pop();
      m_sorter.push(rr);

      while (!pq.empty() && pq.top().child_level == rr.child_level)
      {
        reorder_request next = pq.top();
        pq.pop();
        m_sorter.push(next);
      }
      m_sorter.end();
      m_sorter.calc(d_indicator);

      // The root of output can never be within arc restriction
      // Therefore r is assigned this variable, to ensure that
      // the following equality check will always fail in the first iteration!
      // ie. this is arc way of setting r to null
      bdd r = bdd_ithvar(perm[0]);
      bdd r_prime;
      uint64_t i = -1;
      while (m_sorter.can_pull())
      {
        debug_log("Merger loop", 0);

        reorder_request m_rr = m_sorter.pull();
        assignment_file path = reverse_path(af, m_rr.source, assignment{perm[label_of(m_rr.source)], is_flagged(m_rr.source)});

        r_prime = bdd_restrict(dd, path);
        debug_log("R_Prime restriction found", 0);

        if (bdd_equal(r, r_prime))
        {
          debug_log("R and R_Prime equal", 0);
          ptr_t old_node = create_node_ptr(m_rr.child_level, i);
          {
            arc_writer aw(af);
            aw.unsafe_push(arc_t{m_rr.source, old_node});
          }
        }
        else
        {
          i++;
          debug_log("R and R_Prime NOT equal", 0);
          ptr_t new_node = create_node_ptr(m_rr.child_level, i);
          {
            arc_writer aw(af);
            aw.unsafe_push(arc_t{m_rr.source, new_node});
          }
          push_children(pq, new_node, af, dd);
        }
        r = r_prime;
      }
    }
    debug_log("Reorder done", 0);
    node_file nodes = convert_arc_file_to_node_file(af);
    return __bdd(nodes);
  }
}