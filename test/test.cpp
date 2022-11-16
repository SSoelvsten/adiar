#include <tpie/tpie.h>
#include <bandit/bandit.h>

using namespace snowhouse;
using namespace bandit;

#include <adiar/adiar.h>

using namespace adiar;

////////////////////////////////////////////////////////////////////////////////
// To improve the relationship between the tests and the algorithms, we will not
// reverse the order in which we read the output.
//
// We also do a few little hacks, to read unreduced output from the file_union
// class.
template <typename T>
using level_info_test_stream = level_info_stream<T, true>;

class nodeest_stream: public node_stream<true>
{
public:
  nodeest_stream(node_file &f): node_stream<true>(f) { }
  nodeest_stream(bdd &f): node_stream<true>(f) { }
  nodeest_stream(__bdd &f): node_stream<true>(f.get<node_file>(), f.negate) { }
  nodeest_stream(zdd &f): node_stream<true>(f) { }
  nodeest_stream(__zdd &f): node_stream<true>(f.get<node_file>(), f.negate) { }
};

class node_arc_test_stream: public node_arc_stream<true>
{
public:
  node_arc_test_stream(arc_file &f): node_arc_stream<true>(f) { }
  node_arc_test_stream(__bdd &bdd): node_arc_stream<true>(bdd.get<arc_file>()) { }
  node_arc_test_stream(__zdd &zdd): node_arc_stream<true>(zdd.get<arc_file>()) { }
};

class terminal_arc_test_stream: public terminal_arc_stream<true>
{
public:
  terminal_arc_test_stream(arc_file &f): terminal_arc_stream<true>(f) { }
  terminal_arc_test_stream(__bdd &bdd): terminal_arc_stream<true>(bdd.get<arc_file>()) { }
  terminal_arc_test_stream(__zdd &zdd): terminal_arc_stream<true>(zdd.get<arc_file>()) { }
};
////////////////////////////////////////////////////////////////////////////////
// To improve the error messages

namespace snowhouse
{
  std::string string_of_adiar_ptr(adiar::ptr_uint64 p)
  {
    std::stringstream stream;
    if (p.is_nil()) {
      stream << "NIL";
    } else if (p.is_terminal()) {
      stream << p.value();
    } else { // p.is_node()
      stream << "(x" << p.label() << ", " << p.id() << ")" ;
    }
    return stream.str();
  }

  std::string string_of_adiar_uid(adiar::uid_uint64 u)
  {
    std::stringstream stream;
    if (u.is_terminal()) {
      stream << u.value();
    } else { // u.is_node()
      stream << "(x" << u.label() << ", " << u.id() << ")" ;
    }
    return stream.str();
  }

  template<>
  struct Stringizer<arc_t>
  {
    static std::string ToString(const arc_t& a)
    {
      std::stringstream stream;
      stream << "arc: "
             << string_of_adiar_uid(a.source())
             << " " << (a.is_high() ? "--->" : "- ->") << " "
             << string_of_adiar_uid(a.target())
        ;
      return stream.str();
    }
  };

  template<>
  struct Stringizer<node>
  {
    static std::string ToString(const node& n)
    {
      std::stringstream stream;
      if (n.is_terminal()) {
        stream << "node: " << n.value();
      } else {
        stream << "node: ("
               << string_of_adiar_uid(n.uid())
               << ", "
               << string_of_adiar_uid(n.low())
               << ", "
               << string_of_adiar_uid(n.high())
               << ")"
          ;
      }
      return stream.str();
    }
  };

  template<>
  struct Stringizer<level_info_t>
  {
    static std::string ToString(const level_info_t& m)
    {
      std::stringstream stream;
      stream << "level_info: (x" << label_of(m) << ", #nodes = " << width_of(m) << ")";
      return stream.str();
    }
  };

  template<>
  struct Stringizer<assignment_t>
  {
    static std::string ToString(const assignment_t& a)
    {
      std::stringstream stream;
      stream << "assignment: [x" << label_of(a) << "|->" << value_of(a) << "]";
      return stream.str();
    }
  };
}

////////////////////////////////////////////////////////////////////////////////
// ADIAR Core unit tests
#include "adiar/test_assignment.cpp"
#include "adiar/test_bool_op.cpp"

#include "adiar/test_file.cpp"

#include "adiar/test_domain.cpp"
#include "adiar/test_builder.cpp"

////////////////////////////////////////////////////////////////////////////////
// ADIAR Internal unit tests
#include "adiar/internal/test_dot.cpp"
#include "adiar/internal/test_util.cpp"

#include "adiar/internal/algorithms/test_convert.cpp"
#include "adiar/internal/algorithms/test_isomorphism.cpp"
#include "adiar/internal/algorithms/test_reduce.cpp"

#include "adiar/internal/data_structures/test_levelized_priority_queue.cpp"

#include "adiar/internal/data_types/test_ptr.cpp"
#include "adiar/internal/data_types/test_uid.cpp"
#include "adiar/internal/data_types/test_node.cpp"
#include "adiar/internal/data_types/test_arc.cpp"
#include "adiar/internal/data_types/test_convert.cpp"

////////////////////////////////////////////////////////////////////////////////
// ADIAR BDD unit tests
#include "adiar/bdd/test_bdd.cpp"

#include "adiar/bdd/test_apply.cpp"
#include "adiar/bdd/test_assignment.cpp"
#include "adiar/bdd/test_build.cpp"
#include "adiar/bdd/test_count.cpp"
#include "adiar/bdd/test_evaluate.cpp"
#include "adiar/bdd/test_if_then_else.cpp"
#include "adiar/bdd/test_negate.cpp"
#include "adiar/bdd/test_quantify.cpp"
#include "adiar/bdd/test_restrict.cpp"

////////////////////////////////////////////////////////////////////////////////
// ADIAR ZDD unit tests
#include "adiar/zdd/test_zdd.cpp"

#include "adiar/zdd/test_binop.cpp"
#include "adiar/zdd/test_build.cpp"
#include "adiar/zdd/test_change.cpp"
#include "adiar/zdd/test_complement.cpp"
#include "adiar/zdd/test_contains.cpp"
#include "adiar/zdd/test_count.cpp"
#include "adiar/zdd/test_elem.cpp"
#include "adiar/zdd/test_expand.cpp"
#include "adiar/zdd/test_pred.cpp"
#include "adiar/zdd/test_project.cpp"
#include "adiar/zdd/test_subset.cpp"

int main(int argc, char* argv[]) {
#ifdef NDEBUG
  std::cerr << "Warning: Internal assertions are not enabled!\n\n";
#endif

  // Initialize ADIAR and TPIE
  adiar_init(1024 * 1024 * 1024);

  // Run tests
  auto bandit_ret = bandit::run(argc, argv);

  // Output statistics
  adiar::adiar_printstat();

  // Close all of TPIE down again
  adiar_deinit();

  if (bandit_ret != 0) exit(bandit_ret);
  exit(0);
}

