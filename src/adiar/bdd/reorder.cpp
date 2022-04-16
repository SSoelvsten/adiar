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
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_reorder(const bdd &dd)
  {
    return bdd_sink(false);
  }

  struct assignment
  {
    label_t label;
    bool assignment;
  };

  std::vector<assignment> reverse_path(const arc_file &af, node_t &n)
  {
    std::vector<assignment> assignments = std::vector<assignment>();
    adiar::node_arc_stream<> fs(af);
    ptr_t current = high_arc_of(n).source;
    
    while (fs.can_pull())
    {
      arc_t a = fs.pull();
      if (a.target == current) {
        current = a.source;
        assignments.push_back(assignment{label_of(current), is_high(a)});
      }
    }

    fs.detach();
    return assignments;
  }
}
