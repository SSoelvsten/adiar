#include <adiar/internal/dot.h>

go_bandit([]() {
  describe("adiar/dot.h", [&]() {
    it("can output .dot for a terminal-only BDD", [&]() {
      bdd terminal_T = bdd_terminal(true);
      output_dot(terminal_T, "dot_test_terminal.dot");
      int exit_value = system("dot -O -Tpng dot_test_terminal.dot");
      AssertThat(exit_value, Is().EqualTo(0));
    });

    it("can output .dot for a reduced BDD with internal nodes", [&]() {
      node_file reduced_bdd;

      { // Garbage collect writer early
        node_writer rw(reduced_bdd);

        rw << create_node(42,1, create_terminal_ptr(false), create_terminal_ptr(true))
           << create_node(42,0, create_terminal_ptr(true), create_terminal_ptr(false))
           << create_node(1,2, create_node_ptr(42,0), create_node_ptr(42,1))
           << create_node(0,1, create_node_ptr(1,2), create_terminal_ptr(false))
          ;
      }

      output_dot(reduced_bdd, "dot_test_reduced.dot");
      int exit_value = system("dot -O -Tpng dot_test_reduced.dot");
      AssertThat(exit_value, Is().EqualTo(0));
    });

    it("can output a reduced BDD with internal nodes", [&]() {
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
