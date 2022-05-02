#include <adiar/bdd.h>
#include <adiar/bdd/bdd_policy.h>

#include <adiar/data.h>

#include <adiar/file.h>
#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/substitution.h>

#define PRINT 0
#define HASHING 1

namespace adiar
{
  std::vector<label_t> perm;
  std::vector<label_t> perm_inv;

#if HASHING
  typedef uint64_t hash_t;

  struct reorder_request
  {
    ptr_t source;
    label_t child_level;
    hash_t hash;
  };

  struct reorder_request_lt
  {
    bool operator()(const reorder_request &a, const reorder_request &b) const
    {
      if (a.child_level == b.child_level)
      {
        return a.hash < b.hash;
      }
      return a.child_level < b.child_level;
    }
  };
#else
  struct reorder_request
  {
    ptr_t source;
    label_t child_level;
  };

  struct reorder_request_lt
  {
    bool operator()(const reorder_request &a, const reorder_request &b) const
    {
      return a.child_level < b.child_level;
    }
  };
#endif

  void debug_log(const std::string &msg, int tabs)
  {
#if PRINT
    for (int i = 0; i < tabs; i++)
      std::cout << "  ";
    std::cout << msg << std::endl;
#endif
  }

#if HASHING
  hash_t hash_of(const bdd &dd)
  {
    node_stream<> ns(dd);
    hash_t hash = 0;
    while (ns.can_pull())
    {
      node_t node = ns.pull();
      hash = hash ^ (node.uid * node.low * node.high);
    }
    return hash;
  }
#endif

  // T/B I/Os
  assignment_file reverse_path(const arc_file &af, ptr_t node_ptr, const assignment_t assignment_of_node, const bool extra_assignment)
  {
    assignment_file assign_file;
    {
      assignment_writer aw(assign_file);
      node_arc_stream<> nas(af);
      ptr_t current = node_ptr;

      debug_log("REVERSE PATH: Starting arc: " + std::to_string(node_ptr), 2);

      while (nas.can_pull())
      {
        arc_t arc = nas.pull();
        debug_log("REVERSE PATH: Looking @ source: " + std::to_string(arc.source) + " --" + std::to_string(is_flagged(arc.source)) + "--> target: " + std::to_string(arc.target), 2);
        if (arc.target == current)
        {
          current = unflag(arc.source);
          aw.unsafe_push(assignment{perm[label_of(current)], is_high(arc)});
        }
      }
      af.make_writeable();
      if (extra_assignment)
      {
        aw.unsafe_push(assignment_of_node);
      }
    }

    // Reverse_path no longer only takes T/B I/Os
    simple_file_sorter<assignment_t> sfsorter;
    sfsorter.sort(assign_file);

    return assign_file;
  }

  assignment_file reverse_path(const arc_file &af, ptr_t node_ptr)
  {
    return reverse_path(af, node_ptr, assignment_t{0, 0}, false);
  }

  assignment_file reverse_path(const arc_file &af, ptr_t node_ptr, const assignment_t assignment_of_node)
  {
    return reverse_path(af, node_ptr, assignment_of_node, true);
  }

  std::tuple<assignment_file, assignment_file> dual_reverse_path(const arc_file &af, ptr_t node_ptr_n, ptr_t node_ptr_m)
  {
    assignment_file assignment_file_n;
    assignment_file assignment_file_m;

    {
      assignment_writer aw_n(assignment_file_n);
      assignment_writer aw_m(assignment_file_m);

      adiar::node_arc_stream<> nas(af);

      ptr_t current_n = node_ptr_n;
      ptr_t current_m = node_ptr_m;

      debug_log("DUAL REVERSE PATH: Starting arc_n: " + std::to_string(node_ptr_n) + " arc_m: " + std::to_string(node_ptr_m), 2);

      while (nas.can_pull())
      {
        arc_t arc = nas.pull();
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

    simple_file_sorter<assignment_t> sfsorter;
    sfsorter.sort(assignment_file_n);
    sfsorter.sort(assignment_file_m);

    return {assignment_file_n, assignment_file_m};
  }

  // N/B I/Os
  label_t min_label(const bdd &dd)
  {
    if (is_sink(dd))
    {
      return 0xffffffff;
    }

    label_t result = UINT_MAX;
    level_info_stream<node_t> lis(dd);

    while (lis.can_pull())
    {
      level_info_t li = lis.pull();
      label_t label = perm_inv[li.label];
      if (label < result && li.width > 0)
      {
        result = label;
      }
    }
    return result;
  }

  void push_children(external_priority_queue<reorder_request, reorder_request_lt> &pq, const ptr_t source_ptr, const arc_file &af, const bdd &F)
  {
    auto push_children_with_source_assignment = [&](bool source_assignment)
    {
      assignment_file path = reverse_path(af, source_ptr, assignment_t{perm[label_of(source_ptr)], source_assignment});
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

      bdd F_ikb = bdd_restrict(F, path);
      debug_log("PUSH-CHILDREN: restriction done", 1);
      label_t label = min_label(F_ikb);
      debug_log("PUSH-CHILDREN: min-label done - found: " + std::to_string(label), 1);

      bool is_leaf = label == 0xffffffff;
      if (!is_leaf)
      {
        debug_log("PUSH-CHILDREN: pushing non-leaf", 1);
        ptr_t src;
        if (source_assignment)
          src = flag(source_ptr);
        else
          src = unflag(source_ptr);

#if HASHING
        pq.push(reorder_request{src, label, hash_of(F_ikb)});
#else
        pq.push(reorder_request{src, label});
#endif
        debug_log("RR: {" + std::to_string(src) + ", " + std::to_string(label) + "}", 1);
      }
      else
      {
        ptr_t src;
        if (source_assignment)
          src = flag(source_ptr);
        else
          src = unflag(source_ptr);
        debug_log("PUSH-CHILDREN: writing arc to leaf", 1);
        arc_writer aw(af);
        aw.unsafe_push(arc_t{src, create_sink_ptr(value_of(F_ikb))});
      }
    };

    push_children_with_source_assignment(false);
    push_children_with_source_assignment(true);
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

      node_arc_stream<true> nas(af);
      while (nas.can_pull())
      {
        arc_t arc = nas.pull();
        sfw.push(arc);
      }

      sink_arc_stream<true> sas(af);
      while (sas.can_pull())
      {
        arc_t arc = sas.pull();
        sfw.push(arc);
      }
    }

    auto arc_gt = [](const arc_t &arc, const arc_t &source_assignment) -> bool
    { return source_assignment.source < arc.source; };
    simple_file_sorter<arc_t, decltype(arc_gt)> sfs;
    sfs.sort(all_arcs, arc_gt);

    node_file nodes;
    node_writer nw(nodes);

    file_stream<arc_t> fs(all_arcs);
    while (fs.can_pull())
    {
      arc_t high_arc = fs.pull();
      arc_t low_arc = fs.pull();
      node n = node_of(low_arc, high_arc);
      nw.push(n);
    }
    return nodes;
  }
#if HASHING
  __bdd bdd_reorder(const bdd &F, const std::vector<label_t> permutation)
  {
    init_permutation(permutation);

    debug_log("Reorder started", 0);

    // prøv levelized_priority_queue for UNLIMITED POWER
    // spørg Steffan - how to do dis?
    external_priority_queue<reorder_request, reorder_request_lt> pq(memory::available() / 2, 0); // 0 is pq external doesnt care

    arc_file af;
    ptr_t root = create_node_ptr(0, 0);
    push_children(pq, root, af, F);

    debug_log("Initialization done", 0);

    // The root of output can never be within arc restriction
    // Therefore r is assigned this variable, to ensure that
    // the following equality check will always fail in the first iteration!
    // ie. this is arc way of setting r to null
    bdd r = bdd_ithvar(perm[0]);
    bdd r_prime;
    reorder_request last_rr = {0, 0, 1}; // dummy reorder request

    // i is set to -1, as it is incremented before the first iteration
    uint64_t i = -1;
    while (!pq.empty())
    {
      debug_log("PQ loop", 0);

      reorder_request m_rr = pq.top();
      pq.pop();
      debug_log("R_Prime restriction found", 0);
      if (m_rr.hash == last_rr.hash)
      {
        assignment_file path = reverse_path(af, m_rr.source, assignment{perm[label_of(m_rr.source)], is_flagged(m_rr.source)});
        r_prime = bdd_restrict(F, path);
      }
      if (m_rr.hash == last_rr.hash && bdd_equal(r, r_prime))
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
        push_children(pq, new_node, af, F);
      }
      r = r_prime;
      last_rr = m_rr;
    }

    debug_log("Reorder done", 0);
    node_file nodes = convert_arc_file_to_node_file(af);
    return __bdd(nodes);
  }
#else
  __bdd bdd_reorder(const bdd &F, const std::vector<label_t> permutation)
  {
    init_permutation(permutation);

    debug_log("Reorder started", 0);

    // prøv levelized_priority_queue for UNLIMITED POWER
    // spørg Steffan - how to do dis?
    external_priority_queue<reorder_request, reorder_request_lt> pq(memory::available() / 2, 0); // 0 is pq external doesnt care

    arc_file af;
    ptr_t root = create_node_ptr(0, 0);
    push_children(pq, root, af, F);

    debug_log("Initialization done", 0);

    while (!pq.empty())
    {
      debug_log("PQ loop", 0);
      auto bdd_lt = [&](const reorder_request &a, const reorder_request &b) -> bool
      {
        assignment_file path_a, path_b;
        std::tie(path_a, path_b) = dual_reverse_path(af, a.source, b.source);

        bdd a_restrict = bdd_restrict(F, path_a);
        bdd b_restrict = bdd_restrict(F, path_b);

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
      tpie::merge_sorter<reorder_request, false, decltype(bdd_lt)> msorter(bdd_lt);
      msorter.set_available_memory(memory::available() / 2);
      tpie::dummy_progress_indicator dummy_indicator;
      msorter.begin();

      reorder_request rr = pq.top();
      pq.pop();
      msorter.push(rr);

      while (!pq.empty() && pq.top().child_level == rr.child_level)
      {
        reorder_request next = pq.top();
        pq.pop();
        msorter.push(next);
      }
      msorter.end();
      msorter.calc(dummy_indicator);

      // The root of output can never be within arc restriction
      // Therefore r is assigned this variable, to ensure that
      // the following equality check will always fail in the first iteration!
      // ie. this is arc way of setting r to null
      bdd r = bdd_ithvar(perm[0]);
      bdd r_prime;

      // i is set to -1, as it is incremented before the first iteration
      uint64_t i = -1;
      while (msorter.can_pull())
      {
        debug_log("Merger loop", 0);

        reorder_request m_rr = msorter.pull();
        assignment_file path = reverse_path(af, m_rr.source, assignment{perm[label_of(m_rr.source)], is_flagged(m_rr.source)});

        r_prime = bdd_restrict(F, path);
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
          push_children(pq, new_node, af, F);
        }
        r = r_prime;
      }
    }
    debug_log("Reorder done", 0);
    node_file nodes = convert_arc_file_to_node_file(af);
    return __bdd(nodes);
  }
#endif
}