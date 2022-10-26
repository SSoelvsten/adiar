#include <fstream>
#include <adiar/internal/dot.h>

go_bandit([]() {
  describe("adiar/dot.h", [&]() {
    it("can output .dot for a terminal-only BDD", [&]() {
      bdd terminal_T = bdd_terminal(true);

      bdd_printdot(terminal_T, "dot_test_terminal_T.dot");
      int exit_value = system("dot -O -Tpng dot_test_terminal_T.dot");
      AssertThat(exit_value, Is().EqualTo(0));
    });

    it("can output .dot for a BDD with internal nodes", [&]() {
      node_file reduced_bdd;

      { // Garbage collect writer early
        node_writer rw(reduced_bdd);

        rw << node(42,1, create_terminal_ptr(false), create_terminal_ptr(true))
           << node(42,0, create_terminal_ptr(true), create_terminal_ptr(false))
           << node(1,2, create_node_ptr(42,0), create_node_ptr(42,1))
           << node(0,1, create_node_ptr(1,2), create_terminal_ptr(false))
          ;
      }

      bdd_printdot(reduced_bdd, "dot_test_bdd.dot");
      int exit_value = system("dot -O -Tpng dot_test_bdd.dot");
      AssertThat(exit_value, Is().EqualTo(0));
    });

    it("can output .dot of a BDD into an output stream", [&]() {
      bdd f = bdd_terminal(false);

      std::ofstream out;
      out.open("dot_test_bdd_ostream.dot");

      bdd_printdot(f, out);

      out.close();

      int exit_value = system("dot -O -Tpng dot_test_bdd_ostream.dot");
      AssertThat(exit_value, Is().EqualTo(0));
    });

    ////////////////////////////////////////////////////////////////////////////
    it("can output .dot for a terminal-only ZDD", [&]() {
      zdd terminal_F = zdd_empty();

      zdd_printdot(terminal_F, "dot_test_terminal_F.dot");
      int exit_value = system("dot -O -Tpng dot_test_terminal_F.dot");
      AssertThat(exit_value, Is().EqualTo(0));
    });

    it("can output .dot for a ZDD with internal nodes", [&]() {
      node_file reduced_zdd;

      { // Garbage collect writer early
        node_writer rw(reduced_zdd);

        rw << node(42,0, create_terminal_ptr(false), create_terminal_ptr(true))
           << node(1,2, create_node_ptr(42,0), create_node_ptr(42,0))
           << node(0,1, create_node_ptr(1,2), create_terminal_ptr(true))
          ;
      }

      zdd_printdot(reduced_zdd, "dot_test_zdd.dot");
      int exit_value = system("dot -O -Tpng dot_test_zdd.dot");
      AssertThat(exit_value, Is().EqualTo(0));
    });

    it("can output .dot of a ZDD into an output stream", [&]() {
      zdd A = zdd_null();

      std::ofstream out;
      out.open("dot_test_zdd_ostream.dot");

      zdd_printdot(A, out);

      out.close();

      int exit_value = system("dot -O -Tpng dot_test_zdd_ostream.dot");
      AssertThat(exit_value, Is().EqualTo(0));
    });

    ////////////////////////////////////////////////////////////////////////////
    it("can output an unreduced diagram [internal API]", [&]() {
      arc_file unreduced_bdd;

      { // Garbage collect writer early
        arc_writer uw(unreduced_bdd);

        uw.unsafe_push_terminal({ flag(create_node_ptr(2,0)), create_terminal_ptr(true) });
        uw.unsafe_push_terminal({ create_node_ptr(2,0), create_terminal_ptr(true) });
        uw.unsafe_push_terminal({ create_node_ptr(1,1), create_terminal_ptr(true) });

        uw.unsafe_push_node({ create_node_ptr(0,0), create_node_ptr(1,0) });
        uw.unsafe_push_node({ create_node_ptr(0,0), create_node_ptr(1,1) });
        uw.unsafe_push_node({ create_node_ptr(1,0), create_node_ptr(2,0) });
        uw.unsafe_push_node({ flag(create_node_ptr(1,0)), create_node_ptr(2,0) });
        uw.unsafe_push_node({ flag(create_node_ptr(1,1)), create_node_ptr(2,0) });
      }

      output_dot(unreduced_bdd, "dot_test_unreduced.dot");
      int exit_value = system("dot -O -Tpng dot_test_unreduced.dot");
      AssertThat(exit_value, Is().EqualTo(0));
    });
  });
 });
