#ifndef ADIAR_TEST_H
#define ADIAR_TEST_H

#include <tpie/tpie.h>
#include <bandit/bandit.h>

using namespace snowhouse;
using namespace bandit;

#include <adiar/adiar.h>

////////////////////////////////////////////////////////////////////////////////
// To improve the relationship between the tests and the algorithms, we will not
// reverse the order in which we read the output.
//
// We also do a few little hacks, to read unreduced output from the file_union
// class.
#include <adiar/internal/io/arc_stream.h>
#include <adiar/internal/io/arc_writer.h>
#include <adiar/internal/io/node_stream.h>
#include <adiar/internal/io/node_writer.h>

using namespace adiar;
using namespace adiar::internal;

using level_info_test_stream = level_info_stream<true>;

class node_test_stream: public node_stream<true>
{
public:
  node_test_stream(shared_levelized_file<dd::node_t> &f)
    : node_stream<true>(f)
  { }

  node_test_stream(bdd &f)
    : node_stream<true>(f)
  { }

  node_test_stream(__bdd &f)
    : node_stream<true>(f.get<__bdd::shared_nodes_t>(), f.negate)
  { }

  node_test_stream(zdd &f)
    : node_stream<true>(f)
  { }

  node_test_stream(__zdd &f)
    : node_stream<true>(f.get<__zdd::shared_nodes_t>(), f.negate)
  { }
};

class arc_test_stream: public arc_stream<true>
{
public:
  arc_test_stream(shared_levelized_file<arc> &f)
    : arc_stream<true>(f)
  { }

  arc_test_stream(__bdd &bdd)
    : arc_stream<true>(bdd.get<__bdd::shared_arcs_t>())
  { }

  arc_test_stream(__zdd &zdd)
    : arc_stream<true>(zdd.get<__zdd::shared_arcs_t>())
  { }
};

////////////////////////////////////////////////////////////////////////////////
// To improve the error messages
//
// TODO: move into '<<'/'toString' methods in each respective data type.
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
  struct Stringizer<level_info>
  {
    static std::string ToString(const adiar::internal::level_info& m)
    {
      std::stringstream stream;
      stream << "level_info: ("
             << "x" << m.label()
             << " (" << m.level() << ")"
             << ", #nodes = " << m.width()
             << ")";
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
// For unit testing levelized_file<int> (local stack and shared heap variants).
namespace adiar::internal
{
  template <>
  struct file_traits<int>
  {
    static constexpr size_t files = 2u;

    struct stats
    { /* No extra 'int' specific variables */ };
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
