#ifndef ADIAR_TEST_H
#define ADIAR_TEST_H

#include <tpie/tpie.h>
#include <bandit/bandit.h>

using namespace snowhouse;
using namespace bandit;

#include <adiar/adiar.h>

using namespace adiar;
using namespace adiar::internal;

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

class node_arcest_stream: public node_arc_stream<true>
{
public:
  node_arcest_stream(arc_file &f): node_arc_stream<true>(f) { }
  node_arcest_stream(__bdd &bdd): node_arc_stream<true>(bdd.get<arc_file>()) { }
  node_arcest_stream(__zdd &zdd): node_arc_stream<true>(zdd.get<arc_file>()) { }
};

class terminal_arcest_stream: public terminal_arc_stream<true>
{
public:
  terminal_arcest_stream(arc_file &f): terminal_arc_stream<true>(f) { }
  terminal_arcest_stream(__bdd &bdd): terminal_arc_stream<true>(bdd.get<arc_file>()) { }
  terminal_arcest_stream(__zdd &zdd): terminal_arc_stream<true>(zdd.get<arc_file>()) { }
};

////////////////////////////////////////////////////////////////////////////////
// To improve the error messages
namespace snowhouse
{
  std::string string_of_adiar_ptr(adiar::internal::ptr_uint64 p)
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

  std::string string_of_adiar_uid(adiar::internal::uid_uint64 u)
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
  struct Stringizer<arc>
  {
    static std::string ToString(const adiar::internal::arc& a)
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
    static std::string ToString(const adiar::internal::node& n)
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
    static std::string ToString(const adiar::internal::level_info_t& m)
    {
      std::stringstream stream;
      stream << "level_info: (x" << label_of(m) << ", #nodes = " << width_of(m) << ")";
      return stream.str();
    }
  };

  template<>
  struct Stringizer<assignment_t>
  {
    static std::string ToString(const adiar::assignment_t& a)
    {
      std::stringstream stream;
      stream << "assignment: [x" << label_of(a) << "|->" << value_of(a) << "]";
      return stream.str();
    }
  };
}

////////////////////////////////////////////////////////////////////////////////
// Main
int main(int argc, char* argv[]) {
#ifdef NDEBUG
  std::cerr << "Warning: Internal assertions are not enabled!\n\n";
#endif

  // Initialize ADIAR and TPIE
  adiar_init(1024 * 1024 * 1024);

  // Run tests
  auto bandit_ret = bandit::run(argc, argv);

  // Output statistics
#ifdef ADIAR_TEST_PRINT_STATS
  adiar::adiar_printstat();
#endif

  // Close all of TPIE down again
  adiar_deinit();

  if (bandit_ret != 0) exit(bandit_ret);
  exit(0);
}

#endif // ADIAR_TEST_H
