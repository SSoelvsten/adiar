#include "../../../test.h"

#include <filesystem>
#include <adiar/internal/io/node_arc_stream.h>

go_bandit([]() {
  describe("adiar/internal/io/arc_file.h , arc_stream.h , arc_writer.h", []() {
    describe("arc_writer", []() {
      it("can push level information", []() {
        levelized_file<arc> af;

        arc_writer aw(af);
        aw << (level_info{ 0,1 })
           << (level_info{ 1,2 });
        aw.detach();

        AssertThat(af.size(), Is().EqualTo(0u));
        AssertThat(af.levels(), Is().EqualTo(2u));

        // AssertThat(af.semi_transposed, Is().True());

        AssertThat(af.max_1level_cut, Is().EqualTo(MAX_CUT));

        AssertThat(af.number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(af.number_of_terminals[true],  Is().EqualTo(0u));
      });

      it("can push internal arcs", []() {
        levelized_file<arc> af;

        arc_writer aw(af);
        aw.push_internal(arc(arc::ptr_t(0,0), false, arc::ptr_t(1,0)));
        aw.push_internal(arc(arc::ptr_t(0,0), true,  arc::ptr_t(1,0)));
        aw.push_internal(arc(arc::ptr_t(1,0), false, arc::ptr_t(2,0)));
        aw.detach();

        AssertThat(af.size(),   Is().EqualTo(3u));
        AssertThat(af.size(0u), Is().EqualTo(3u));
        AssertThat(af.size(1u), Is().EqualTo(0u));
        AssertThat(af.size(2u), Is().EqualTo(0u));

        AssertThat(af.levels(), Is().EqualTo(0u));

        // AssertThat(af.semi_transposed, Is().True());

        AssertThat(af.max_1level_cut, Is().EqualTo(MAX_CUT));

        AssertThat(af.number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(af.number_of_terminals[true],  Is().EqualTo(0u));
      });

      it("can push terminal arcs [in-order]", []() {
        levelized_file<arc> af;

        arc_writer aw(af);
        aw.push_terminal(arc(arc::ptr_t(0,0), false, arc::ptr_t(true)));
        aw.push_terminal(arc(arc::ptr_t(0,0), true,  arc::ptr_t(true)));
        aw.detach();

        AssertThat(af.size(),   Is().EqualTo(2u));
        AssertThat(af.size(0u), Is().EqualTo(0u));
        AssertThat(af.size(1u), Is().EqualTo(2u));
        AssertThat(af.size(2u), Is().EqualTo(0u));

        AssertThat(af.levels(), Is().EqualTo(0u));

        AssertThat(af.number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(af.number_of_terminals[true],  Is().EqualTo(2u));

        // AssertThat(af.semi_transposed, Is().True());

        levelized_file_stream<arc> lfs(af);
        AssertThat(lfs.can_pull<0>(), Is().False());

        AssertThat(lfs.can_pull<1>(), Is().True());
        AssertThat(lfs.pull<1>(),     Is().EqualTo(arc(arc::ptr_t(0,0), false, arc::ptr_t(true))));
        AssertThat(lfs.can_pull<1>(), Is().True());
        AssertThat(lfs.pull<1>(),     Is().EqualTo(arc(arc::ptr_t(0,0), true,  arc::ptr_t(true))));
        AssertThat(lfs.can_pull<1>(), Is().False());

        AssertThat(lfs.can_pull<2>(), Is().False());
      });

      it("can push terminal arcs [out-of-order]", []() {
        levelized_file<arc> af;

        arc_writer aw(af);
        aw.push_terminal(arc(arc::ptr_t(0,0), true,  arc::ptr_t(true)));
        aw.push_terminal(arc(arc::ptr_t(0,0), false, arc::ptr_t(false)));
        aw.detach();

        AssertThat(af.size(),   Is().EqualTo(2u));
        AssertThat(af.size(0u), Is().EqualTo(0u));
        AssertThat(af.size(1u), Is().EqualTo(1u));
        AssertThat(af.size(2u), Is().EqualTo(1u));

        AssertThat(af.levels(), Is().EqualTo(0u));

        AssertThat(af.number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(af.number_of_terminals[true],  Is().EqualTo(1u));

        levelized_file_stream<arc> lfs(af);
        AssertThat(lfs.can_pull<0>(), Is().False());

        AssertThat(lfs.can_pull<1>(), Is().True());
        AssertThat(lfs.pull<1>(),     Is().EqualTo(arc(arc::ptr_t(0,0), true,  arc::ptr_t(true))));
        AssertThat(lfs.can_pull<1>(), Is().False());

        AssertThat(lfs.can_pull<2>(), Is().True());
        AssertThat(lfs.pull<2>(),     Is().EqualTo(arc(arc::ptr_t(0,0), false, arc::ptr_t(false))));
        AssertThat(lfs.can_pull<2>(), Is().False());
      });

      it("can push mixed arcs", []() {
        levelized_file<arc> af;

        arc_writer aw(af);

        aw << arc(arc::ptr_t(0,0), false, arc::ptr_t(1,0))
           << arc(arc::ptr_t(1,0), true,  arc::ptr_t(true))   // <-- in-order
           << arc(arc::ptr_t(1,1), false, arc::ptr_t(true))   // <-- in-order
           << arc(arc::ptr_t(0,0), true,  arc::ptr_t(1,1))
           << arc(arc::ptr_t(1,0), false, arc::ptr_t(2,0))
           << arc(arc::ptr_t(2,0), false, arc::ptr_t(false))  // <-- in-order
           << arc(arc::ptr_t(2,0), true,  arc::ptr_t(true))   // <-- in-order
           << arc(arc::ptr_t(1,1), true,  arc::ptr_t(true));  // <-- out-of-order

        aw.detach();

        AssertThat(af.size(),   Is().EqualTo(8u));
        AssertThat(af.size(0u), Is().EqualTo(3u));
        AssertThat(af.size(1u), Is().EqualTo(4u));
        AssertThat(af.size(2u), Is().EqualTo(1u));

        AssertThat(af.levels(), Is().EqualTo(0u));

        AssertThat(af.number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(af.number_of_terminals[true],  Is().EqualTo(4u));

        levelized_file_stream<arc> lfs(af);
        AssertThat(lfs.can_pull<0>(), Is().True());
        AssertThat(lfs.pull<0>(),     Is().EqualTo(arc(arc::ptr_t(0,0), false, arc::ptr_t(1,0))));
        AssertThat(lfs.can_pull<0>(), Is().True());
        AssertThat(lfs.pull<0>(),     Is().EqualTo(arc(arc::ptr_t(0,0), true,  arc::ptr_t(1,1))));
        AssertThat(lfs.can_pull<0>(), Is().True());
        AssertThat(lfs.pull<0>(),     Is().EqualTo(arc(arc::ptr_t(1,0), false, arc::ptr_t(2,0))));
        AssertThat(lfs.can_pull<0>(), Is().False());

        AssertThat(lfs.can_pull<1>(), Is().True());
        AssertThat(lfs.pull<1>(),     Is().EqualTo(arc(arc::ptr_t(1,0), true,  arc::ptr_t(true))));
        AssertThat(lfs.can_pull<1>(), Is().True());
        AssertThat(lfs.pull<1>(),     Is().EqualTo(arc(arc::ptr_t(1,1), false, arc::ptr_t(true))));
        AssertThat(lfs.can_pull<1>(), Is().True());
        AssertThat(lfs.pull<1>(),     Is().EqualTo(arc(arc::ptr_t(2,0), false, arc::ptr_t(false))));
        AssertThat(lfs.can_pull<1>(), Is().True());
        AssertThat(lfs.pull<1>(),     Is().EqualTo(arc(arc::ptr_t(2,0), true,  arc::ptr_t(true))));
        AssertThat(lfs.can_pull<1>(), Is().False());

        AssertThat(lfs.can_pull<2>(), Is().True());
        AssertThat(lfs.pull<2>(),     Is().EqualTo(arc(arc::ptr_t(1,1), true,  arc::ptr_t(true))));
        AssertThat(lfs.can_pull<2>(), Is().False());
      });
    });

    describe("arc_writer + arc_stream", []() {
      levelized_file<arc> af;
      /*
                     _0_     ---- x0
                    /   \
                    1   2    ---- x1
                   / \ / \
                   3 T | T
                  / \  |
                  F T  T
       */
      {
        arc_writer aw(af);
        aw << arc(arc::ptr_t(0,0), false, arc::ptr_t(1,0));
        aw << arc(arc::ptr_t(1,0), true,  arc::ptr_t(true));  // <-- in-order
        aw << arc(arc::ptr_t(1,1), false, arc::ptr_t(true));  // <-- in-order
        aw << arc(arc::ptr_t(0,0), true,  arc::ptr_t(1,1));
        aw << arc(arc::ptr_t(1,0), false, arc::ptr_t(2,0));
        aw << arc(arc::ptr_t(2,0), false, arc::ptr_t(false)); // <-- in-order
        aw << arc(arc::ptr_t(2,0), true,  arc::ptr_t(true));  // <-- in-order
        aw << arc(arc::ptr_t(1,1), true,  arc::ptr_t(true));  // <-- out-of-order
      }

      it("merges terminal arcs on 'pull' [default: backwards]", [&af]() {
        arc_stream<> as(af);

        AssertThat(as.can_pull_terminal(), Is().True());
        AssertThat(as.pull_terminal(),     Is().EqualTo(arc(arc::ptr_t(2,0), true,  arc::ptr_t(true))));
        AssertThat(as.can_pull_terminal(), Is().True());
        AssertThat(as.pull_terminal(),     Is().EqualTo(arc(arc::ptr_t(2,0), false, arc::ptr_t(false))));
        AssertThat(as.can_pull_terminal(), Is().True());
        AssertThat(as.pull_terminal(),     Is().EqualTo(arc(arc::ptr_t(1,1), true,  arc::ptr_t(true))));
        AssertThat(as.can_pull_terminal(), Is().True());
        AssertThat(as.pull_terminal(),     Is().EqualTo(arc(arc::ptr_t(1,1), false, arc::ptr_t(true))));
        AssertThat(as.can_pull_terminal(), Is().True());
        AssertThat(as.pull_terminal(),     Is().EqualTo(arc(arc::ptr_t(1,0), true,  arc::ptr_t(true))));
        AssertThat(as.can_pull_terminal(), Is().False());
      });

      it("merges terminal arcs on 'pull' [non-default: forwards]", [&af]() {
        arc_stream<true> as(af);

        AssertThat(as.can_pull_terminal(), Is().True());
        AssertThat(as.pull_terminal(),     Is().EqualTo(arc(arc::ptr_t(1,0), true,  arc::ptr_t(true))));
        AssertThat(as.can_pull_terminal(), Is().True());
        AssertThat(as.pull_terminal(),     Is().EqualTo(arc(arc::ptr_t(1,1), false, arc::ptr_t(true))));
        AssertThat(as.can_pull_terminal(), Is().True());
        AssertThat(as.pull_terminal(),     Is().EqualTo(arc(arc::ptr_t(1,1), true,  arc::ptr_t(true))));
        AssertThat(as.can_pull_terminal(), Is().True());
        AssertThat(as.pull_terminal(),     Is().EqualTo(arc(arc::ptr_t(2,0), false, arc::ptr_t(false))));
        AssertThat(as.can_pull_terminal(), Is().True());
        AssertThat(as.pull_terminal(),     Is().EqualTo(arc(arc::ptr_t(2,0), true,  arc::ptr_t(true))));
        AssertThat(as.can_pull_terminal(), Is().False());
      });

      it("merges terminal arcs on 'pull' [negated]", [&af]() {
        arc_stream<> as(af, true);

        AssertThat(as.can_pull_terminal(), Is().True());
        AssertThat(as.pull_terminal(),     Is().EqualTo(arc(arc::ptr_t(2,0), true,  arc::ptr_t(false))));
        AssertThat(as.can_pull_terminal(), Is().True());
        AssertThat(as.pull_terminal(),     Is().EqualTo(arc(arc::ptr_t(2,0), false, arc::ptr_t(true))));
        AssertThat(as.can_pull_terminal(), Is().True());
        AssertThat(as.pull_terminal(),     Is().EqualTo(arc(arc::ptr_t(1,1), true,  arc::ptr_t(false))));
        AssertThat(as.can_pull_terminal(), Is().True());
        AssertThat(as.pull_terminal(),     Is().EqualTo(arc(arc::ptr_t(1,1), false, arc::ptr_t(false))));
        AssertThat(as.can_pull_terminal(), Is().True());
        AssertThat(as.pull_terminal(),     Is().EqualTo(arc(arc::ptr_t(1,0), true,  arc::ptr_t(false))));
        AssertThat(as.can_pull_terminal(), Is().False());
      });

      it("reads internal arcs as given [non-default: forwards]", [&af]() {
        arc_stream<> as(af);

        AssertThat(as.can_pull_internal(), Is().True());
        AssertThat(as.pull_internal(),     Is().EqualTo(arc(arc::ptr_t(1,0), false, arc::ptr_t(2,0))));
        AssertThat(as.can_pull_internal(), Is().True());
        AssertThat(as.pull_internal(),     Is().EqualTo(arc(arc::ptr_t(0,0), true,  arc::ptr_t(1,1))));
        AssertThat(as.can_pull_internal(), Is().True());
        AssertThat(as.pull_internal(),     Is().EqualTo(arc(arc::ptr_t(0,0), false, arc::ptr_t(1,0))));
        AssertThat(as.can_pull_internal(), Is().False());
      });

      it("provides number of unread terminals [negate=false]", [&af]() {
        arc_stream<> as(af);

        AssertThat(as.unread_terminals(),      Is().EqualTo(5u));
        AssertThat(as.unread_terminals(false), Is().EqualTo(1u));
        AssertThat(as.unread_terminals(true),  Is().EqualTo(4u));

        AssertThat(as.can_pull_terminal(), Is().True());
        as.pull_terminal(); // 3 ---> T

        AssertThat(as.unread_terminals(),      Is().EqualTo(4u));
        AssertThat(as.unread_terminals(false), Is().EqualTo(1u));
        AssertThat(as.unread_terminals(true),  Is().EqualTo(3u));

        AssertThat(as.can_pull_terminal(), Is().True());
        as.pull_terminal(); // 3 - -> F

        AssertThat(as.unread_terminals(),      Is().EqualTo(3u));
        AssertThat(as.unread_terminals(false), Is().EqualTo(0u));
        AssertThat(as.unread_terminals(true),  Is().EqualTo(3u));

        AssertThat(as.can_pull_terminal(), Is().True());
        as.pull_terminal(); // 2 ---> T

        AssertThat(as.unread_terminals(),      Is().EqualTo(2u));
        AssertThat(as.unread_terminals(false), Is().EqualTo(0u));
        AssertThat(as.unread_terminals(true),  Is().EqualTo(2u));

        AssertThat(as.can_pull_terminal(), Is().True());
        as.pull_terminal(); // 2 - -> T

        AssertThat(as.unread_terminals(),      Is().EqualTo(1u));
        AssertThat(as.unread_terminals(false), Is().EqualTo(0u));
        AssertThat(as.unread_terminals(true),  Is().EqualTo(1u));

        AssertThat(as.can_pull_terminal(), Is().True());
        as.pull_terminal(); // 1 - -> T

        AssertThat(as.unread_terminals(),      Is().EqualTo(0u));
        AssertThat(as.unread_terminals(false), Is().EqualTo(0u));
        AssertThat(as.unread_terminals(true),  Is().EqualTo(0u));
      });

      it("provides number of unread terminals [negate=true]", [&af]() {
        arc_stream<> as(af, true);

        AssertThat(as.unread_terminals(),      Is().EqualTo(5u));
        AssertThat(as.unread_terminals(false), Is().EqualTo(4u));
        AssertThat(as.unread_terminals(true),  Is().EqualTo(1u));

        AssertThat(as.can_pull_terminal(), Is().True());
        as.pull_terminal(); // 3 ---> F

        AssertThat(as.unread_terminals(),      Is().EqualTo(4u));
        AssertThat(as.unread_terminals(false), Is().EqualTo(3u));
        AssertThat(as.unread_terminals(true),  Is().EqualTo(1u));

        AssertThat(as.can_pull_terminal(), Is().True());
        as.pull_terminal(); // 3 - -> T

        AssertThat(as.unread_terminals(),      Is().EqualTo(3u));
        AssertThat(as.unread_terminals(false), Is().EqualTo(3u));
        AssertThat(as.unread_terminals(true),  Is().EqualTo(0u));

        AssertThat(as.can_pull_terminal(), Is().True());
        as.pull_terminal(); // 2 ---> F

        AssertThat(as.unread_terminals(),      Is().EqualTo(2u));
        AssertThat(as.unread_terminals(false), Is().EqualTo(2u));
        AssertThat(as.unread_terminals(true),  Is().EqualTo(0u));

        AssertThat(as.can_pull_terminal(), Is().True());
        as.pull_terminal(); // 2 - -> F

        AssertThat(as.unread_terminals(),      Is().EqualTo(1u));
        AssertThat(as.unread_terminals(false), Is().EqualTo(1u));
        AssertThat(as.unread_terminals(true),  Is().EqualTo(0u));

        AssertThat(as.can_pull_terminal(), Is().True());
        as.pull_terminal(); // 1 - -> F

        AssertThat(as.unread_terminals(),      Is().EqualTo(0u));
        AssertThat(as.unread_terminals(false), Is().EqualTo(0u));
        AssertThat(as.unread_terminals(true),  Is().EqualTo(0u));
      });
    });

    describe("arc_writer + node_arc_stream", []() {
      it("can read single-node BDD [in-order]", []() {
        levelized_file<arc> af;

        {
          arc_writer aw(af);
          aw.push_terminal(arc(arc::ptr_t(0,0), false, arc::ptr_t(false)));
          aw.push_terminal(arc(arc::ptr_t(0,0), true,  arc::ptr_t(true)));

          aw.push(level_info(0,1));
          aw.detach();
        }

        node_arc_stream ns(af);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),     Is().EqualTo(node(0,0, node::ptr_t(false), node::ptr_t(true))));

        AssertThat(ns.can_pull(), Is().False());
      });

      it("can read single-node BDD [out-of-order]", []() {
        levelized_file<arc> af;

        {
          arc_writer aw(af);
          aw.push_terminal(arc(arc::ptr_t(0,0), true,  arc::ptr_t(true)));
          aw.push_terminal(arc(arc::ptr_t(0,0), false, arc::ptr_t(false)));

          aw.push(level_info(0,1));
          aw.detach();
        }

        node_arc_stream ns(af);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),     Is().EqualTo(node(0,0, node::ptr_t(false), node::ptr_t(true))));

        AssertThat(ns.can_pull(), Is().False());
      });

      it("can read larger BDD [internals already sorted]", []() {
        /*
        //             1        ---- x0
        //            / \
        //            2 |       ---- x1
        //           / \|
        //           F _3_      ---- x2
        //            /   \
        //            4   5     ---- x3
        //           / \ / \
        //           F T T F
        */
        levelized_file<arc> af;

        {
          arc_writer aw(af);

          aw.push(level_info(0,1));

          aw.push_internal(arc(arc::ptr_t(0,0), false,  arc::ptr_t(1,0)));
          aw.push_terminal(arc(arc::ptr_t(1,0), false,  arc::ptr_t(false)));

          aw.push(level_info(1,1));

          aw.push_internal(arc(arc::ptr_t(0,0), true,   arc::ptr_t(2,0)));
          aw.push_internal(arc(arc::ptr_t(1,0), true,   arc::ptr_t(2,0)));

          aw.push(level_info(2,1));

          aw.push_internal(arc(arc::ptr_t(2,0), false,  arc::ptr_t(3,0)));
          aw.push_terminal(arc(arc::ptr_t(3,0), false,  arc::ptr_t(false)));
          aw.push_terminal(arc(arc::ptr_t(3,0), true,   arc::ptr_t(true)));

          aw.push_internal(arc(arc::ptr_t(2,0), true,   arc::ptr_t(3,1)));
          aw.push_terminal(arc(arc::ptr_t(3,1), false,  arc::ptr_t(true)));
          aw.push_terminal(arc(arc::ptr_t(3,1), true,   arc::ptr_t(false)));

          aw.push(level_info(3,2));

          aw.detach();
        }

        node_arc_stream ns(af);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),     Is().EqualTo(node(0,0, node::ptr_t(1,0), node::ptr_t(2,0))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),     Is().EqualTo(node(1,0, node::ptr_t(false), node::ptr_t(2,0))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),     Is().EqualTo(node(2,0, node::ptr_t(3,0), node::ptr_t(3,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),     Is().EqualTo(node(3,0, node::ptr_t(false), node::ptr_t(true))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),     Is().EqualTo(node(3,1, node::ptr_t(true), node::ptr_t(false))));

        AssertThat(ns.can_pull(), Is().False());
      });

      it("can read larger BDD [internals need sorting]", []() {
        /*
        //       1     ---- x0
        //      / \
        //      | 2    ---- x1
        //      |/ \
        //      3  T   ---- x2
        //     / \
        //     F T
        */
        levelized_file<arc> af;

        {
          arc_writer aw(af);

          aw.push(level_info(0,1));

          aw.push_internal(arc(arc::ptr_t(0,0), true,   arc::ptr_t(1,0)));
          aw.push_terminal(arc(arc::ptr_t(1,0), true,   arc::ptr_t(true)));

          aw.push(level_info(1,1));

          aw.push_internal(arc(arc::ptr_t(0,0), false,  arc::ptr_t(2,0)));
          aw.push_internal(arc(arc::ptr_t(1,0), false,  arc::ptr_t(2,0)));
          aw.push_terminal(arc(arc::ptr_t(2,0), false,  arc::ptr_t(false)));
          aw.push_terminal(arc(arc::ptr_t(2,0), true,   arc::ptr_t(true)));

          aw.push(level_info(2,1));

          aw.detach();
        }

        node_arc_stream ns(af);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),     Is().EqualTo(node(0,0, node::ptr_t(2,0), node::ptr_t(1,0))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),     Is().EqualTo(node(1,0, node::ptr_t(2,0), node::ptr_t(true))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),     Is().EqualTo(node(2,0, node::ptr_t(false), node::ptr_t(true))));

        AssertThat(ns.can_pull(), Is().False());
      });

      it("can read single-node BDD [negated]", []() {
        levelized_file<arc> af;

        {
          arc_writer aw(af);
          aw.push_terminal(arc(arc::ptr_t(0,0), false, arc::ptr_t(false)));
          aw.push_terminal(arc(arc::ptr_t(0,0), true,  arc::ptr_t(true)));

          aw.push(level_info(0,1));
          aw.detach();
        }

        node_arc_stream ns(af, true);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),     Is().EqualTo(node(0,0, node::ptr_t(true), node::ptr_t(false))));

        AssertThat(ns.can_pull(), Is().False());
      });

      it("can read single-node BDD [reverse]", []() {
        levelized_file<arc> af;

        {
          arc_writer aw(af);
          aw.push_terminal(arc(arc::ptr_t(0,0), false, arc::ptr_t(false)));
          aw.push_terminal(arc(arc::ptr_t(0,0), true,  arc::ptr_t(true)));

          aw.push(level_info(0,1));
          aw.detach();
        }

        node_arc_stream<true> ns(af);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),     Is().EqualTo(node(0,0, node::ptr_t(false), node::ptr_t(true))));

        AssertThat(ns.can_pull(), Is().False());
      });

      it("can read larger BDD [reverse]", []() {
        /*
        //         1       ---- x0
        //        / \
        //       /  2      ---- x1
        //       | / \
        //       3 F 4     ---- x2
        //      / \ / \
        //      F  5  T    ---- x3
        //        / \
        //        F T
        */
        levelized_file<arc> af;

        {
          arc_writer aw(af);

          aw.push(level_info(0,1));

          aw.push_internal(arc(arc::ptr_t(0,0), true,   arc::ptr_t(1,0)));
          aw.push_terminal(arc(arc::ptr_t(1,0), false,  arc::ptr_t(false)));

          aw.push(level_info(1,1));

          aw.push_internal(arc(arc::ptr_t(0,0), false,  arc::ptr_t(2,0)));
          aw.push_terminal(arc(arc::ptr_t(2,0), false,  arc::ptr_t(false)));

          aw.push_internal(arc(arc::ptr_t(1,0), true,   arc::ptr_t(2,1)));
          aw.push_terminal(arc(arc::ptr_t(2,1), true,   arc::ptr_t(true)));

          aw.push(level_info(2,2));

          aw.push_internal(arc(arc::ptr_t(2,0), true,   arc::ptr_t(3,0)));
          aw.push_internal(arc(arc::ptr_t(2,1), false,  arc::ptr_t(3,0)));

          aw.push_terminal(arc(arc::ptr_t(3,0), false,  arc::ptr_t(false)));
          aw.push_terminal(arc(arc::ptr_t(3,0), true,   arc::ptr_t(true)));

          aw.detach();
        }

        node_arc_stream<true> ns(af, true);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),     Is().EqualTo(node(3,0, node::ptr_t(true), node::ptr_t(false))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),     Is().EqualTo(node(2,1, node::ptr_t(3,0),  node::ptr_t(false))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),     Is().EqualTo(node(2,0, node::ptr_t(true), node::ptr_t(3,0))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),     Is().EqualTo(node(1,0, node::ptr_t(true), node::ptr_t(2,1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),     Is().EqualTo(node(0,0, node::ptr_t(2,0),  node::ptr_t(1,0))));

        AssertThat(ns.can_pull(), Is().False());
      });
    });
  });
 });
