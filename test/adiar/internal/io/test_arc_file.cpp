#include "../../../test.h"

#include <filesystem>

go_bandit([]() {
  describe("adiar/internal/io/arc_file.h , arc_stream.h , arc_writer.h", []() {
    describe("arc_writer", []() {
      it(".push(level_info) / <<", []() {
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

      it(".push_internal(arc) / <<", []() {
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

      it(".push_terminal(arc) [in-order]", []() {
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

      it(".push_terminal(arc) [out-of-order]", []() {
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

      it(".push(arc) / <<", []() {
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

      it("Merges terminal arcs on 'pull' [default: backwards]", [&af]() {
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

      it("Merges terminal arcs on 'pull' [non-default: forwards]", [&af]() {
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

      it("Reads internal arcs as given [non-default: forwards]", [&af]() {
        arc_stream<> as(af);

        AssertThat(as.can_pull_internal(), Is().True());
        AssertThat(as.pull_internal(),     Is().EqualTo(arc(arc::ptr_t(1,0), false, arc::ptr_t(2,0))));
        AssertThat(as.can_pull_internal(), Is().True());
        AssertThat(as.pull_internal(),     Is().EqualTo(arc(arc::ptr_t(0,0), true,  arc::ptr_t(1,1))));
        AssertThat(as.can_pull_internal(), Is().True());
        AssertThat(as.pull_internal(),     Is().EqualTo(arc(arc::ptr_t(0,0), false, arc::ptr_t(1,0))));
        AssertThat(as.can_pull_internal(), Is().False());
      });

      it("Provides number of unread terminals", [&af]() {
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
    });

    describe("untranspose()", []() {
      // TODO
    });
  });
 });
