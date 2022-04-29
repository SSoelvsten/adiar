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
  std::vector<label_t> perm;
  std::vector<label_t> perm_inv;

  struct reorder_request
  {
    ptr_t source; // gemmer også is_high (brug flag, unflag, is_flagged)
    label_t child_level;
  };

  struct reorder_lt
  {
    bool operator()(const reorder_request &a, const reorder_request &b) const
    {
      return a.child_level < b.child_level;
    }
  };

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

      std::cout << "REVERSE PATH: Starting arc: " << n << std::endl;

      while (fs.can_pull())
      {
        arc_t a = fs.pull();
        std::cout << "REVERSE PATH: Looking @ source: " << a.source << " --" << is_flagged(a.source) << "--> target: " << a.target << std::endl;
        if (a.target == current)
        {
          current = unflag(a.source);
          aw.unsafe_push(assignment{perm[label_of(current)], is_high(a)});
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
      std::cout << "PUSH-CHILDREN: reverse path done" << std::endl;
      {
        std::cout << "PUSH-CHILDREN: reverse_path Assignment = (";
        assignment_stream<> as(path);
        while (as.can_pull())
        {
          assignment_t a = as.pull();
          std::cout << a.label << " = " << a.value << ", ";
        }
        std::cout << ")" << std::endl;
      }

      bdd f_ikb = bdd_restrict(f, path);
      std::cout << "PUSH-CHILDREN: restriction done" << std::endl;
      label_t label = min_label(f_ikb);
      std::cout << "PUSH-CHILDREN: min-label found: " << label << std::endl;

      bool is_leaf = label == 0xffffffff;
      if (!is_leaf)
      {
        std::cout << "PUSH-CHILDREN: pushing non-leaf" << std::endl;
        ptr_t src;
        if (b)
          src = flag(source);
        else
          src = unflag(source);
        pq.push(reorder_request{src, label});
        std::cout << "RR: {" << src << " , " << label << "}" << std::endl;
      }
      else
      {
        std::cout << "PUSH-CHILDREN: writing arc to leaf" << std::endl;
        arc_writer aw(af);
        aw.unsafe_push(arc_t{source, create_sink_ptr(value_of(f_ikb))});
      }
    };

    std::cout << "PUSH-CHILDREN: Called" << std::endl;

    deal_with(true);
    deal_with(false);
  }

  void initPermutation(const std::vector<label_t> permutation)
  {
    perm = permutation;
    perm_inv = std::vector<label_t>(permutation.size(), 0);
    for (unsigned long i = 0; i < permutation.size(); i++)
    {
      perm_inv[permutation[i]] = i;
    }
  }

  __bdd bdd_reorder(const bdd &dd, const std::vector<label_t> permutation)
  {
    initPermutation(permutation);

    std::cout << "Reorder started" << std::endl;

    // prøv levelized_priority_queue for UNLIMITED POWER
    external_priority_queue<reorder_request, reorder_lt> pq(memory::available() / 2, 0); // 0 is pq external doesnt care

    arc_file af;
    ptr_t root = create_node_ptr(0, 0);
    std::cout << "Now pushing children" << std::endl;
    push_children(pq, root, af, dd);

    std::cout << "Initialization done" << std::endl;

    while (!pq.empty())
    {
      std::cout << "PQ loop" << std::endl;

      // TODO https://github.com/Mortal/tpieex/blob/master/main.cc line 68-70
      // TODO add operator as argument in the constructor.
      auto pred = [&](const reorder_request &a, const reorder_request &b) -> bool
      {
        assignment_file path_a = reverse_path(af, a.source);
        bdd a_restrict = bdd_restrict(dd, path_a);

        assignment_file path_b = reverse_path(af, b.source);
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
      std::cout << "First element from PQ removed" << std::endl;

      while (!pq.empty() && pq.top().child_level == rr.child_level)
      {
        reorder_request next = pq.top();
        pq.pop();
        m_sorter.push(next);
      }
      m_sorter.end();
      m_sorter.calc(d_indicator);

      // The root of output can never be within a restriction
      // Therefore r is assigned this variable, to ensure that
      // the following equality check will always fail in the first iteration!
      // ie. this is a way of setting r to null
      bdd r = bdd_ithvar(perm[0]);
      bdd r_prime;
      uint64_t i = -1;
      while (m_sorter.can_pull())
      {
        std::cout << "Merger loop" << std::endl;

        reorder_request m_rr = m_sorter.pull();
        assignment_file path = reverse_path(af, m_rr.source, assignment{perm[label_of(m_rr.source)], is_flagged(m_rr.source)});

        r_prime = bdd_restrict(dd, path);
        std::cout << "R_Prime restriction found" << std::endl;

        if (bdd_equal(r, r_prime))
        {
          std::cout << "R and R_Prime equal" << std::endl;
          ptr_t old_node = create_node_ptr(m_rr.child_level, i);
          {
            arc_writer aw(af);
            aw.unsafe_push(arc_t{m_rr.source, old_node});
          }
        }
        else
        {
          i++;
          std::cout << "R and R_Prime NOT equal" << std::endl;
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

    std::cout << "Finished reordering BDD" << std::endl;
    // TODO change arc file to node file to create an bdd
    // maybe do bdd(af);

    // TODO traverse all arcs, and map them using the perm array
    // This makes the BDD go from identity order to permutation order
    return __bdd(af);
  }
}
