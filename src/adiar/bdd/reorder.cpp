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

  struct pq_elem
  {
    int k;
    int i;
    bool b;
    int t;
  };

  bool operator<(const pq_elem& a, const pq_elem& b){
    a.k < b.k;
  }

  __bdd bdd_reorder(const bdd &dd)
  {
    external_priority_queue<pq_elem, pq_elem> pq(10, 100); //??

    return bdd_sink(false);
  }

  // N/B I/Os
  label_t min_label(const bdd &dd)
  {
    if (is_sink(dd))
    {
      return 0; // TODO: nodes are indexed from 0 - change
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
  std::vector<assignment> reverse_path(const arc_file &af, node_t &n)
  {
    std::vector<assignment> assignments = std::vector<assignment>();
    adiar::node_arc_stream<> fs(af);
    ptr_t current = high_arc_of(n).source; // Hacky solution - is this valid

    while (fs.can_pull())
    {
      arc_t a = fs.pull();
      if (a.target == current)
      {
        current = a.source;
        assignments.push_back(assignment{label_of(current), is_high(a)});
      }
    }

    fs.detach();
    return assignments;
  }
}
