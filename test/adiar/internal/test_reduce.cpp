// Import Reduce algorithm
#include <adiar/internal/reduce.h>

// Import Decision Diagram policies
#include <adiar/bdd/bdd_policy.h>
#include <adiar/zdd/zdd_policy.h>

go_bandit([]() {
  describe("adiar/internal/reduce.h", [&]() {
    ptr_t sink_T = create_sink_ptr(true);
    ptr_t sink_F = create_sink_ptr(false);

    node_file x0x1_node_file;

    { // Garbage collect writer to free write-lock
      node_writer nw(x0x1_node_file);

      nw << create_node(1,MAX_ID, sink_F, sink_T)
         << create_node(0,MAX_ID, sink_F, create_node_ptr(1,MAX_ID));
    }

    it("preserves negation flag on reduced input [1]", [&]() {
      /*
               1                  1      ---- x0
              / \                / \
              F 2        =>      F 2     ---- x1
               / \                / \
               F T                F T
      */

      // Reduce it
      bdd in(x0x1_node_file, false);
      __bdd out = reduce<bdd_policy>(in);

      AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(x0x1_node_file._file_ptr));
      AssertThat(out.negate, Is().False());
    });

    it("preserves negation flag on reduced input [2]", [&]() {
      /*
               1                  1      ---- x0
              / \                / \
              F 2      = ~ =>    T 2     ---- x1
               / \                / \
               F T                T F
      */

      // Reduce it
      bdd in(x0x1_node_file, true);
      __bdd out = reduce<bdd_policy>(in);

      AssertThat(out.get<node_file>()._file_ptr, Is().EqualTo(x0x1_node_file._file_ptr));
      AssertThat(out.negate, Is().True());
    });

    describe("Reduction Rule 2", [&]() {
      it("applies to sink arcs [1]", [&]() {
        /*
                   1                  1      ---- x0
                  / \                / \
                  | 2_               | 2     ---- x1
                  | | \      =>      |/ \
                  3 4 T              4  T    ---- x2
                  |X|               / \
                  F T               F T
        */

        ptr_t n1 = create_node_ptr(0,0);
        ptr_t n2 = create_node_ptr(1,0);
        ptr_t n3 = create_node_ptr(2,0);
        ptr_t n4 = create_node_ptr(2,1);

        arc_file in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.unsafe_push_node({ flag(n1),n2 });
          aw.unsafe_push_node({ n1,n3 });
          aw.unsafe_push_node({ n2,n4 });

          aw.unsafe_push_sink({ flag(n2),sink_T });
          aw.unsafe_push_sink({ n3,sink_F });
          aw.unsafe_push_sink({ flag(n3),sink_T });
          aw.unsafe_push_sink({ n4,sink_F });
          aw.unsafe_push_sink({ flag(n4),sink_T });

          aw.unsafe_push(create_level_info(0,1u));
          aw.unsafe_push(create_level_info(1,1u));
          aw.unsafe_push(create_level_info(2,2u));
        }

        in->max_1level_cut = 2;

        // Reduce it

        bdd out = reduce<bdd_policy>(in);

        AssertThat(is_canonical(out), Is().True());

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        // n4
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(2, MAX_ID,
                                                              sink_F,
                                                              sink_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n2
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                              create_node_ptr(2,MAX_ID),
                                                              sink_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n1
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                              create_node_ptr(2,MAX_ID),
                                                              create_node_ptr(1,MAX_ID))));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(2,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(3u));

        AssertThat(out->number_of_sinks[0], Is().EqualTo(1u));
        AssertThat(out->number_of_sinks[1], Is().EqualTo(2u));
      });

      it("applies to sink arcs [2]", [&]() {
        /*
                    1      ---- x0            1
                   / \                       / \
                   | |     ---- x1           | |
                   | |                       | |
                   2 3     ---- x2    ==>    2 3
                  / X \                     /\ /\
                  F/ \T                     F | T
                   4 5     ---- x3            4
                  /| |\                      / \
                  FT FT                      F T
        */
        ptr_t n1 = create_node_ptr(0,0);
        ptr_t n2 = create_node_ptr(2,0);
        ptr_t n3 = create_node_ptr(2,1);
        ptr_t n4 = create_node_ptr(3,0);
        ptr_t n5 = create_node_ptr(3,1);

        arc_file in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.unsafe_push_node({ n1, n2 });
          aw.unsafe_push_node({ flag(n1), n3 });
          aw.unsafe_push_node({ n3, n4 });
          aw.unsafe_push_node({ flag(n2), n5 });

          aw.unsafe_push_sink({ n2, sink_F });
          aw.unsafe_push_sink({ flag(n3), sink_T });
          aw.unsafe_push_sink({ n4, sink_F });
          aw.unsafe_push_sink({ flag(n4), sink_T });
          aw.unsafe_push_sink({ n5, sink_F });
          aw.unsafe_push_sink({ flag(n5), sink_T });

          aw.unsafe_push(create_level_info(0,1u));
          aw.unsafe_push(create_level_info(2,2u));
          aw.unsafe_push(create_level_info(3,2u));
        }

        in->max_1level_cut = 2;

        // Reduce it
        bdd out = reduce<bdd_policy>(in);

        AssertThat(is_canonical(out), Is().True());

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(3, MAX_ID,
                                                              sink_F,
                                                              sink_T)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(2, MAX_ID,
                                                              create_node_ptr(3, MAX_ID),
                                                              sink_T)));
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(2, MAX_ID-1,
                                                              sink_F,
                                                              create_node_ptr(3, MAX_ID))));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                              create_node_ptr(2, MAX_ID-1),
                                                              create_node_ptr(2, MAX_ID))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(3,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(2,2u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(4u));

        AssertThat(out->number_of_sinks[0], Is().EqualTo(2u));
        AssertThat(out->number_of_sinks[1], Is().EqualTo(2u));
      });

      it("applies to node arcs", [&]() {
        /*
                    1                  1       ---- x0
                   / \                / \
                   | 2_               | 2      ---- x1
                   | | \      =>      |/ \
                   3 4 T              4  T     ---- x2
                   |X|               / \
                   5 6               5  6      ---- x3
                  / \ \\            / \/ \
                  F T T F           F  T F
        */

        ptr_t n1 = create_node_ptr(0,0);
        ptr_t n2 = create_node_ptr(1,0);
        ptr_t n3 = create_node_ptr(2,0);
        ptr_t n4 = create_node_ptr(2,1);
        ptr_t n5 = create_node_ptr(3,0);
        ptr_t n6 = create_node_ptr(3,1);

        arc_file in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.unsafe_push_node({ flag(n1),n2 });
          aw.unsafe_push_node({ n1,n3 });
          aw.unsafe_push_node({ n2,n4 });
          aw.unsafe_push_node({ n3,n5 });
          aw.unsafe_push_node({ n4,n5 });
          aw.unsafe_push_node({ flag(n3),n6 });
          aw.unsafe_push_node({ flag(n4),n6 });

          aw.unsafe_push_sink({ flag(n2),sink_T });
          aw.unsafe_push_sink({ n5,sink_F });
          aw.unsafe_push_sink({ flag(n5),sink_T });
          aw.unsafe_push_sink({ n6,sink_T });
          aw.unsafe_push_sink({ flag(n6),sink_F });

          aw.unsafe_push(create_level_info(0,1u));
          aw.unsafe_push(create_level_info(1,1u));
          aw.unsafe_push(create_level_info(2,2u));
          aw.unsafe_push(create_level_info(3,2u));
        }

        in->max_1level_cut = 4;

        // Reduce it
        bdd out = reduce<bdd_policy>(in);

        AssertThat(is_canonical(out), Is().True());

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        // n5
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(3, MAX_ID, sink_F, sink_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n6
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(3, MAX_ID-1, sink_T, sink_F)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n4
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(2, MAX_ID,
                                                              create_node_ptr(3, MAX_ID),
                                                              create_node_ptr(3, MAX_ID-1))));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n2
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                              create_node_ptr(2,MAX_ID),
                                                              sink_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n1
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                              create_node_ptr(2,MAX_ID),
                                                              create_node_ptr(1,MAX_ID))));
        AssertThat(out_nodes.can_pull(), Is().False());


        level_info_test_stream<node_t> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(3,2u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(2,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(5u));

        AssertThat(out->number_of_sinks[0], Is().EqualTo(2u));
        AssertThat(out->number_of_sinks[1], Is().EqualTo(3u));
      });

      it("applies to both node and sink arcs", [&]() {
        /*
                    1                  1     ---- x0
                   / \                / \
                   | 2_               | 2    ---- x1
                   | | \      =>      |/ \
                   3 4 T              4  T   ---- x2
                   |X|               / \
                   5 T               5 T     ---- x3
                  / \               / \
                  F T               F T
        */

        ptr_t n1 = create_node_ptr(0,0);
        ptr_t n2 = create_node_ptr(1,0);
        ptr_t n3 = create_node_ptr(2,0);
        ptr_t n4 = create_node_ptr(2,1);
        ptr_t n5 = create_node_ptr(3,0);

        arc_file in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.unsafe_push_node({ flag(n1),n2 });
          aw.unsafe_push_node({ n1,n3 });
          aw.unsafe_push_node({ n2,n4 });
          aw.unsafe_push_node({ n3,n5 });
          aw.unsafe_push_node({ n4,n5 });

          aw.unsafe_push_sink({ flag(n2),sink_T });
          aw.unsafe_push_sink({ flag(n3),sink_T });
          aw.unsafe_push_sink({ flag(n4),sink_T });
          aw.unsafe_push_sink({ n5,sink_F });
          aw.unsafe_push_sink({ flag(n5),sink_T });

          aw.unsafe_push(create_level_info(0,1u));
          aw.unsafe_push(create_level_info(1,1u));
          aw.unsafe_push(create_level_info(2,2u));
          aw.unsafe_push(create_level_info(3,1u));
        }

        in->max_1level_cut = 4;

        // Reduce it
        bdd out = reduce<bdd_policy>(in);

        AssertThat(is_canonical(out), Is().True());

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        // n5
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(3, MAX_ID, sink_F, sink_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n4
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(2, MAX_ID,
                                                              create_node_ptr(3,MAX_ID),
                                                              sink_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n2
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                              create_node_ptr(2,MAX_ID),
                                                              sink_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n1
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                              create_node_ptr(2,MAX_ID),
                                                              create_node_ptr(1,MAX_ID))));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(3,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(2,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(2u));

        // Over-approximation, since T-sink from level (3) is removed
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().LessThanOrEqualTo(5u));

        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));

        AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(4u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().LessThanOrEqualTo(5u));

        AssertThat(out->number_of_sinks[0], Is().EqualTo(1u));
        AssertThat(out->number_of_sinks[1], Is().EqualTo(3u));
      });

      it("applies to 'disjoint' branches", [&]() {
        /*
                       1                         1         ---- x0
                      / \                       / \
                     2   3                     2   3       ---- x1
                    / \ / \                    \\ / \
                   /   |   \                    \|___\     (2.low goes to 6)
                   4   5   6        =>           5   6     ---- x2
                  / \ / \ / \                   / \ / \
                  F T 7 T F T                   7 T F T    ---- x3
                     / \                       / \
                     F T                       F T
        */

        ptr_t n1 = create_node_ptr(0,0);
        ptr_t n2 = create_node_ptr(1,0);
        ptr_t n3 = create_node_ptr(1,1);
        ptr_t n4 = create_node_ptr(2,0);
        ptr_t n5 = create_node_ptr(2,1);
        ptr_t n6 = create_node_ptr(2,2);
        ptr_t n7 = create_node_ptr(3,0);

        arc_file in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.unsafe_push_node({ n1,n2 });
          aw.unsafe_push_node({ flag(n1),n3 });
          aw.unsafe_push_node({ n2,n4 });
          aw.unsafe_push_node({ flag(n2),n5 });
          aw.unsafe_push_node({ n3,n5 });
          aw.unsafe_push_node({ flag(n3),n6 });
          aw.unsafe_push_node({ n5,n7 });

          aw.unsafe_push_sink({ n4,sink_F });
          aw.unsafe_push_sink({ flag(n4),sink_T });
          aw.unsafe_push_sink({ flag(n5),sink_T });
          aw.unsafe_push_sink({ n6,sink_F });
          aw.unsafe_push_sink({ flag(n6),sink_T });
          aw.unsafe_push_sink({ n7,sink_F });
          aw.unsafe_push_sink({ flag(n7),sink_T });

          aw.unsafe_push(create_level_info(0,1u));
          aw.unsafe_push(create_level_info(1,2u));
          aw.unsafe_push(create_level_info(2,3u));
          aw.unsafe_push(create_level_info(3,1u));
        }

        in->max_1level_cut = 4;

        // Reduce it
        bdd out = reduce<bdd_policy>(in);

        AssertThat(is_canonical(out), Is().True());

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        // n7
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(3, MAX_ID,
                                                              sink_F,
                                                              sink_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n6
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(2, MAX_ID,
                                                              sink_F,
                                                              sink_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n5
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(2, MAX_ID-1,
                                                              create_node_ptr(3, MAX_ID),
                                                              sink_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n3
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                              create_node_ptr(2, MAX_ID-1),
                                                              create_node_ptr(2, MAX_ID))));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n2
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(1, MAX_ID-1,
                                                              create_node_ptr(2, MAX_ID),
                                                              create_node_ptr(2, MAX_ID-1))));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n1
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                              create_node_ptr(1,MAX_ID-1),
                                                              create_node_ptr(1, MAX_ID))));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(3,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(2,2u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(1,2u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(4u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(4u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(4u));

        // Over-approximation: since at level 3 the reductions of (4) and (6) is
        //                     yet not applied.
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(4u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().LessThanOrEqualTo(7u));

        AssertThat(out->number_of_sinks[0], Is().EqualTo(2u));
        AssertThat(out->number_of_sinks[1], Is().EqualTo(3u));
      });

      it("does forward the correct children [1]", [&]() {
        /*
                       1                1        ---- x0
                      / \              / \
                     2   3             | 3       ---- x1
                    / \ / \      =>    |/ \
                   4   5   6           5   6     ---- x2
                  / \ / \ / \         / \ / \
                  F T F T T F         F T T F
        */

        ptr_t n1 = create_node_ptr(0,0);
        ptr_t n2 = create_node_ptr(1,0);
        ptr_t n3 = create_node_ptr(1,1);
        ptr_t n4 = create_node_ptr(2,0);
        ptr_t n5 = create_node_ptr(2,1);
        ptr_t n6 = create_node_ptr(2,2);

        arc_file in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.unsafe_push_node({ n1,n2 });
          aw.unsafe_push_node({ flag(n1),n3 });
          aw.unsafe_push_node({ n2,n4 });
          aw.unsafe_push_node({ flag(n2),n5 });
          aw.unsafe_push_node({ n3,n5 });
          aw.unsafe_push_node({ flag(n3),n6 });

          aw.unsafe_push_sink({ n4,sink_F });
          aw.unsafe_push_sink({ flag(n4),sink_T });
          aw.unsafe_push_sink({ n5,sink_F });
          aw.unsafe_push_sink({ flag(n5),sink_T });
          aw.unsafe_push_sink({ n6,sink_T });
          aw.unsafe_push_sink({ flag(n6),sink_F });

          aw.unsafe_push(create_level_info(0,1u));
          aw.unsafe_push(create_level_info(1,2u));
          aw.unsafe_push(create_level_info(2,3u));
        }

        in->max_1level_cut = 4;

        // Reduce it
        bdd out = reduce<bdd_policy>(in);

        AssertThat(is_canonical(out), Is().True());

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True()); // 5
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(2, MAX_ID,
                                                              sink_F,
                                                              sink_T)));

        AssertThat(out_nodes.can_pull(), Is().True()); // 6
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(2, MAX_ID-1,
                                                              sink_T,
                                                              sink_F)));

        AssertThat(out_nodes.can_pull(), Is().True()); // 3
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                              create_node_ptr(2, MAX_ID),
                                                              create_node_ptr(2, MAX_ID-1))));

        AssertThat(out_nodes.can_pull(), Is().True()); // 1
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                              create_node_ptr(2, MAX_ID),
                                                              create_node_ptr(1, MAX_ID))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(2,2u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(4u));

        AssertThat(out->number_of_sinks[0], Is().EqualTo(2u));
        AssertThat(out->number_of_sinks[1], Is().EqualTo(2u));
      });

      it("does forward the correct children [2]", [&]() {
        /*
                       1                1        ---- x0
                      / \              / \
                     2   3             | 3       ---- x1
                    / \ / \      =>    |/ \
                   4   5   6           5   6     ---- x2
                  / \ / \ / \         / \ / \
                  T F T F F T         T F F T
        */

        ptr_t n1 = create_node_ptr(0,0);
        ptr_t n2 = create_node_ptr(1,0);
        ptr_t n3 = create_node_ptr(1,1);
        ptr_t n4 = create_node_ptr(2,0);
        ptr_t n5 = create_node_ptr(2,1);
        ptr_t n6 = create_node_ptr(2,2);

        arc_file in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.unsafe_push_node({ n1,n2 });
          aw.unsafe_push_node({ flag(n1),n3 });
          aw.unsafe_push_node({ n2,n4 });
          aw.unsafe_push_node({ flag(n2),n5 });
          aw.unsafe_push_node({ n3,n5 });
          aw.unsafe_push_node({ flag(n3),n6 });

          aw.unsafe_push_sink({ n4,sink_T });
          aw.unsafe_push_sink({ flag(n4),sink_F });
          aw.unsafe_push_sink({ n5,sink_T });
          aw.unsafe_push_sink({ flag(n5),sink_F });
          aw.unsafe_push_sink({ n6,sink_F });
          aw.unsafe_push_sink({ flag(n6),sink_T });

          aw.unsafe_push(create_level_info(0,1u));
          aw.unsafe_push(create_level_info(1,2u));
          aw.unsafe_push(create_level_info(2,3u));
        }

        in->max_1level_cut = 4;

        // Reduce it
        bdd out = reduce<bdd_policy>(in);

        AssertThat(is_canonical(out), Is().True());

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True()); // 6
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(2, MAX_ID,
                                                              sink_F,
                                                              sink_T)));

        AssertThat(out_nodes.can_pull(), Is().True()); // 5
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(2, MAX_ID-1,
                                                              sink_T,
                                                              sink_F)));

        AssertThat(out_nodes.can_pull(), Is().True()); // 3
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                              create_node_ptr(2, MAX_ID-1),
                                                              create_node_ptr(2, MAX_ID))));

        AssertThat(out_nodes.can_pull(), Is().True()); // 1
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                              create_node_ptr(2, MAX_ID-1),
                                                              create_node_ptr(1, MAX_ID))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(2,2u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(4u));

        AssertThat(out->number_of_sinks[0], Is().EqualTo(2u));
        AssertThat(out->number_of_sinks[1], Is().EqualTo(2u));
      });
    });

    describe("Reduction Rule 1: BDD", [&]() {
      it("applies to sink arcs", [&]() {
        /*
                    1                  1     ---- x0
                   / \                / \
                   | 2                | 2    ---- x1
                   |/ \      =>       |/ \
                   3  4               3  |   ---- x2
                  / \//               |\ /
                  F  T                F T
        */

        ptr_t n1 = create_node_ptr(0,0);
        ptr_t n2 = create_node_ptr(1,0);
        ptr_t n3 = create_node_ptr(2,0);
        ptr_t n4 = create_node_ptr(2,1);

        arc_file in;

        { // Garbage collect writer early
          arc_writer aw(in);

          aw.unsafe_push_node({ flag(n1),n2 });
          aw.unsafe_push_node({ n1,n3 });
          aw.unsafe_push_node({ n2,n3 });
          aw.unsafe_push_node({ flag(n2),n4 });

          aw.unsafe_push_sink({ n3,sink_F });
          aw.unsafe_push_sink({ flag(n3),sink_T });
          aw.unsafe_push_sink({ n4,sink_T });
          aw.unsafe_push_sink({ flag(n4),sink_T });

          aw.unsafe_push(create_level_info(0,1u));
          aw.unsafe_push(create_level_info(1,1u));
          aw.unsafe_push(create_level_info(2,2u));
        }

        in->max_1level_cut = 3;

        // Reduce it
        bdd out = reduce<bdd_policy>(in);

        AssertThat(is_canonical(out), Is().True());

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        // n3
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(2, MAX_ID,
                                                              sink_F,
                                                              sink_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n2
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                              create_node_ptr(2,MAX_ID),
                                                              sink_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n1
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                              create_node_ptr(2,MAX_ID),
                                                              create_node_ptr(1,MAX_ID))));
        AssertThat(out_nodes.can_pull(), Is().False());


        level_info_test_stream<node_t> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(2,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(3u));

        AssertThat(out->number_of_sinks[0], Is().EqualTo(1u));
        AssertThat(out->number_of_sinks[1], Is().EqualTo(2u));
      });

      it("applies to node arcs", [&]() {
        /*
                    1                  1        ---- x0
                   / \                / \
                   | 2                | 2       ---- x1
                   |/ \      =>       |/ \
                   3  4               3   \     ---- x2
                  / \ \\             / \  |
                  F T  5             F T  5     ---- x3
                      / \                / \
                      F T                F T
        */

        ptr_t n1 = create_node_ptr(0,0);
        ptr_t n2 = create_node_ptr(1,0);
        ptr_t n3 = create_node_ptr(2,0);
        ptr_t n4 = create_node_ptr(2,1);
        ptr_t n5 = create_node_ptr(3,0);

        arc_file in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.unsafe_push_node({ flag(n1),n2 });
          aw.unsafe_push_node({ n1,n3 });
          aw.unsafe_push_node({ n2,n3 });
          aw.unsafe_push_node({ flag(n2),n4 });
          aw.unsafe_push_node({ n4,n5 });
          aw.unsafe_push_node({ flag(n4),n5 });

          aw.unsafe_push_sink({ n3,sink_F });
          aw.unsafe_push_sink({ flag(n3),sink_T });
          aw.unsafe_push_sink({ n5,sink_F });
          aw.unsafe_push_sink({ flag(n5),sink_T });

          aw.unsafe_push(create_level_info(0,1u));
          aw.unsafe_push(create_level_info(1,1u));
          aw.unsafe_push(create_level_info(2,2u));
          aw.unsafe_push(create_level_info(3,1u));
        }

        in->max_1level_cut = 3;

        // Reduce it
        bdd out = reduce<bdd_policy>(in);

        AssertThat(is_canonical(out), Is().True());

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        // n5
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(3, MAX_ID, sink_F, sink_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n3
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(2, MAX_ID, sink_F, sink_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n2
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                              create_node_ptr(2,MAX_ID),
                                                              create_node_ptr(3,MAX_ID))));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n1
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                              create_node_ptr(2,MAX_ID),
                                                              create_node_ptr(1,MAX_ID))));
        AssertThat(out_nodes.can_pull(), Is().False());


        level_info_test_stream<node_t> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(3,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(2,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(4u));

        AssertThat(out->number_of_sinks[0], Is().EqualTo(2u));
        AssertThat(out->number_of_sinks[1], Is().EqualTo(2u));
      });

      it("works with two reductions on the same level", [&]() {
        /*
                    1                  1        ---- x0
                   / \                / \
                  2   3              |   |      ---- x1
                  ||  ||     =>      |   |
                  4   5              4   5      ---- x2
                 / \ / \            / \ / \
                 F T T F            F T T F

        */

        ptr_t n1 = create_node_ptr(0,0);
        ptr_t n2 = create_node_ptr(1,0);
        ptr_t n3 = create_node_ptr(1,1);
        ptr_t n4 = create_node_ptr(2,0);
        ptr_t n5 = create_node_ptr(2,1);

        arc_file in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.unsafe_push_node({ n1,n2 });
          aw.unsafe_push_node({ flag(n1),n3 });
          aw.unsafe_push_node({ n2,n4 });
          aw.unsafe_push_node({ flag(n2),n4 });
          aw.unsafe_push_node({ n3,n5 });
          aw.unsafe_push_node({ flag(n3),n5 });

          aw.unsafe_push_sink({ n4,sink_F });
          aw.unsafe_push_sink({ flag(n4),sink_T });
          aw.unsafe_push_sink({ n5,sink_T });
          aw.unsafe_push_sink({ flag(n5),sink_F });

          aw.unsafe_push(create_level_info(0,1u));
          aw.unsafe_push(create_level_info(1,2u));
          aw.unsafe_push(create_level_info(2,2u));
        }

        in->max_1level_cut = 4;

        // Reduce it
        bdd out = reduce<bdd_policy>(in);

        AssertThat(is_canonical(out), Is().True());

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        // n5
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(2, MAX_ID, sink_F, sink_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n4
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(2, MAX_ID-1, sink_T, sink_F)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n1
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                              create_node_ptr(2,MAX_ID),
                                                              create_node_ptr(2,MAX_ID-1))));
        AssertThat(out_nodes.can_pull(), Is().False());


        level_info_test_stream<node_t> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(2,2u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(4u));

        AssertThat(out->number_of_sinks[0], Is().EqualTo(2u));
        AssertThat(out->number_of_sinks[1], Is().EqualTo(2u));
      });

      it("can be applied together with reduction rule 2 [1]", [&]() {
        /*
                    1                  1     ---- x0
                   / \                / \
                   2 T                | T    ---- x1
                  / \        =>       |
                  3 4                 4      ---- x2
                  |X|                / \
                  F T                F T
        */

        ptr_t n1 = create_node_ptr(0,0);
        ptr_t n2 = create_node_ptr(1,0);
        ptr_t n3 = create_node_ptr(2,0);
        ptr_t n4 = create_node_ptr(2,1);

        arc_file in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.unsafe_push_node({ n1,n2 });
          aw.unsafe_push_node({ n2,n3 });
          aw.unsafe_push_node({ flag(n2),n4 });

          aw.unsafe_push_sink({ flag(n1),sink_T });
          aw.unsafe_push_sink({ n3,sink_F });
          aw.unsafe_push_sink({ flag(n3),sink_T });
          aw.unsafe_push_sink({ n4,sink_F });
          aw.unsafe_push_sink({ flag(n4),sink_T });

          aw.unsafe_push(create_level_info(0,1u));
          aw.unsafe_push(create_level_info(1,1u));
          aw.unsafe_push(create_level_info(2,2u));
        }

        in->max_1level_cut = 2;

        // Reduce it
        bdd out = reduce<bdd_policy>(in);

        AssertThat(is_canonical(out), Is().True());

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        // n4
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(2, MAX_ID,
                                                              sink_F,
                                                              sink_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n1
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                              create_node_ptr(2,MAX_ID),
                                                              sink_T)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(2,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(3u));

        AssertThat(out->number_of_sinks[0], Is().EqualTo(1u));
        AssertThat(out->number_of_sinks[1], Is().EqualTo(2u));
      });

      it("can be applied together with reduction rule 2 [2]", [&]() {
          /*
                   1                   1       ---- 0
                  / \                 / \
                 2   3               2  |      ---- 1
                / \ / \     =>      / \ /
               /   4   5           /   4       ---- 2
              /   / \ / \         /   / \
              6   F T F T         6   F T      ---- 3
             / \                 / \
             F T                 F T
          */

          ptr_t n1 = create_node_ptr(0,0);
          ptr_t n2 = create_node_ptr(1,0);
          ptr_t n3 = create_node_ptr(1,1);
          ptr_t n4 = create_node_ptr(2,0);
          ptr_t n5 = create_node_ptr(2,1);
          ptr_t n6 = create_node_ptr(3,0);

          arc_file in;

          { // Garbage collect writer to free write-lock
            arc_writer aw(in);

            aw.unsafe_push_node({ n1, n2 });
            aw.unsafe_push_node({ flag(n1), n3 });
            aw.unsafe_push_node({ flag(n2), n4 });
            aw.unsafe_push_node({ n3, n4 });
            aw.unsafe_push_node({ flag(n3), n5 });
            aw.unsafe_push_node({ n2, n6 });

            aw.unsafe_push_sink({ n4, sink_F });
            aw.unsafe_push_sink({ flag(n4), sink_T });
            aw.unsafe_push_sink({ n5, sink_F });
            aw.unsafe_push_sink({ flag(n5), sink_T });
            aw.unsafe_push_sink({ n6, sink_F });
            aw.unsafe_push_sink({ flag(n6), sink_T });

            aw.unsafe_push(create_level_info(0,1u));
            aw.unsafe_push(create_level_info(1,2u));
            aw.unsafe_push(create_level_info(2,2u));
            aw.unsafe_push(create_level_info(3,1u));
          }

          in->max_1level_cut = 4;

          // Reduce it
          bdd out = reduce<bdd_policy>(in);

          AssertThat(is_canonical(out), Is().True());

          // Check it looks all right
          node_test_stream out_nodes(out);

          // n6
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(create_node(3, MAX_ID,
                                                                sink_F,
                                                                sink_T)));

          // n4
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(create_node(2, MAX_ID,
                                                                sink_F,
                                                                sink_T)));

          // n2
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                                create_node_ptr(3, MAX_ID),
                                                                create_node_ptr(2, MAX_ID))));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                                create_node_ptr(1,MAX_ID),
                                                                create_node_ptr(2, MAX_ID))));
          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream<node_t> out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(3,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(2,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(1,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(0,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(3u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(3u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(3u));

          // Over-approximation: at the lowest level we don't know yet, that
          //                     nodes (4) and (5) will be merged.
          //
          //                     Currently, returns 5, i.e. maximum DiCut of '.size() + 1'.
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(4u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().LessThanOrEqualTo(5u));

          AssertThat(out->number_of_sinks[0], Is().EqualTo(2u));
          AssertThat(out->number_of_sinks[1], Is().EqualTo(2u));
        });

      it("can reduce the root", [&]() {
        /*
                   1                         ---- x0
                  / \
                  | 2                        ---- x1
                  |/|         =>
                  3 4                 4      ---- x2
                  |X|                / \
                  F T                F T
        */

        ptr_t n1 = create_node_ptr(0,0);
        ptr_t n2 = create_node_ptr(1,0);
        ptr_t n3 = create_node_ptr(2,0);
        ptr_t n4 = create_node_ptr(2,1);

        arc_file in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.unsafe_push_node({ flag(n1),n2 });
          aw.unsafe_push_node({ n1,n3 });
          aw.unsafe_push_node({ n2,n3 });
          aw.unsafe_push_node({ flag(n2),n4 });

          aw.unsafe_push_sink({ n3,sink_F });
          aw.unsafe_push_sink({ flag(n3),sink_T });
          aw.unsafe_push_sink({ n4,sink_F });
          aw.unsafe_push_sink({ flag(n4),sink_T });

          aw.unsafe_push(create_level_info(0,1u));
          aw.unsafe_push(create_level_info(1,1u));
          aw.unsafe_push(create_level_info(2,2u));
        }

        in->max_1level_cut = 3;

        // Reduce it
        bdd out = reduce<bdd_policy>(in);

        AssertThat(is_canonical(out), Is().True());

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        // n4
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(2, MAX_ID, sink_F, sink_T)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(2,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(out->number_of_sinks[0], Is().EqualTo(1u));
        AssertThat(out->number_of_sinks[1], Is().EqualTo(1u));
      });

      it("can apply reduction rule 1 to a single node", [&]() {
        /*
                   1                 F       ---- x0
                  / \      =>
                  F F
        */

        ptr_t n1 = create_node_ptr(0,0);

        arc_file in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.unsafe_push_sink({ n1,sink_F });
          aw.unsafe_push_sink({ flag(n1),sink_F });

          aw.unsafe_push(create_level_info(0,1u));
        }

        in->max_1level_cut = 0;

        // Reduce it
        bdd out = reduce<bdd_policy>(in);

        AssertThat(is_canonical(out), Is().True());

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        // F
        AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(false)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> out_meta(out);
        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out->number_of_sinks[0], Is().EqualTo(1u));
        AssertThat(out->number_of_sinks[1], Is().EqualTo(0u));
      });

      it("can propagate reduction rule 1 up to a sink", [&]() {
        /*
                   1                  T
                  / \
                  | 2         =>
                  |/ \
                  T  T
        */

        ptr_t n1 = create_node_ptr(0,0);
        ptr_t n2 = create_node_ptr(1,0);

        arc_file in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.unsafe_push_node({ flag(n1),n2 });

          aw.unsafe_push_sink({ n1,sink_T });
          aw.unsafe_push_sink({ n2,sink_T });
          aw.unsafe_push_sink({ flag(n2),sink_T });

          aw.unsafe_push(create_level_info(0,1u));
          aw.unsafe_push(create_level_info(1,1u));
        }

        in->max_1level_cut = 1;

        // Reduce it
        bdd out = reduce<bdd_policy>(in);

        AssertThat(is_canonical(out), Is().True());

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> out_meta(out);
        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out->number_of_sinks[0], Is().EqualTo(0u));
        AssertThat(out->number_of_sinks[1], Is().EqualTo(1u));
      });

      it("can return non-reducible single-node variable with MAX_ID", [&]() {
        /*
                   1                 1       ---- x0
                  / \      =>       / \
                  F T               F T
        */

        ptr_t n1 = create_node_ptr(0,0);

        arc_file in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.unsafe_push_sink({ n1,sink_F });
          aw.unsafe_push_sink({ flag(n1),sink_T });

          aw.unsafe_push(create_level_info(0u,1u));
        }

        in->max_1level_cut = 0;

        // Reduce it
        bdd out = reduce<bdd_policy>(in);

        AssertThat(is_canonical(out), Is().True());

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(0, MAX_ID, sink_F, sink_T)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(0u,1u)));
        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(out->number_of_sinks[0], Is().EqualTo(1u));
        AssertThat(out->number_of_sinks[1], Is().EqualTo(1u));
      });
    });

    describe("Reduction Rule 1: ZDD", [&]() {
      it("applies to sink arcs", [&]() {
        /*
                   1                  1     ---- x0
                  / \                / \
                  | 2         =>     T T    ---- x1
                  |/ \
                  T  F
        */

        ptr_t n1 = create_node_ptr(0,0);
        ptr_t n2 = create_node_ptr(1,0);

        arc_file in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.unsafe_push_node({ flag(n1),n2 });

          aw.unsafe_push_sink({ n1,sink_T });
          aw.unsafe_push_sink({ n2,sink_T });
          aw.unsafe_push_sink({ flag(n2),sink_F });

          aw.unsafe_push(create_level_info(0,1u));
          aw.unsafe_push(create_level_info(1,1u));
        }

        in->max_1level_cut = 1;

        // Reduce it
        zdd out = reduce<zdd_policy>(in);

        AssertThat(is_canonical(out), Is().True());

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(0, MAX_ID, sink_T, sink_T)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(out->number_of_sinks[0], Is().EqualTo(0u));
        AssertThat(out->number_of_sinks[1], Is().EqualTo(2u));
      });

      it("applies to node arcs", [&]() {
        /*
                    1                  1        ---- x0
                   / \                / \
                   | 2                | 2       ---- x1
                   |/ \      =>       |/ \
                   3   4              3   \     ---- x2
                  / \ / \            / \  |
                  F T 5 F            F T  5     ---- x3
                     / \                 / \
                     F T                 F T
        */

        ptr_t n1 = create_node_ptr(0,0);
        ptr_t n2 = create_node_ptr(1,0);
        ptr_t n3 = create_node_ptr(2,0);
        ptr_t n4 = create_node_ptr(2,1);
        ptr_t n5 = create_node_ptr(3,0);

        arc_file in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.unsafe_push_node({ flag(n1),n2 });
          aw.unsafe_push_node({ n1,n3 });
          aw.unsafe_push_node({ n2,n3 });
          aw.unsafe_push_node({ flag(n2),n4 });
          aw.unsafe_push_node({ n4,n5 });

          aw.unsafe_push_sink({ n3,sink_F });
          aw.unsafe_push_sink({ flag(n3),sink_T });
          aw.unsafe_push_sink({ flag(n4),sink_F });
          aw.unsafe_push_sink({ n5,sink_F });
          aw.unsafe_push_sink({ flag(n5),sink_T });

          aw.unsafe_push(create_level_info(0,1u));
          aw.unsafe_push(create_level_info(1,1u));
          aw.unsafe_push(create_level_info(2,2u));
          aw.unsafe_push(create_level_info(3,1u));
        }

        in->max_1level_cut = 3;

        // Reduce it
        zdd out = reduce<zdd_policy>(in);

        AssertThat(is_canonical(out), Is().True());

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        // n5
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(3, MAX_ID, sink_F, sink_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n3
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(2, MAX_ID, sink_F, sink_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n2
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                              create_node_ptr(2,MAX_ID),
                                                              create_node_ptr(3,MAX_ID))));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n1
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                              create_node_ptr(2,MAX_ID),
                                                              create_node_ptr(1,MAX_ID))));
        AssertThat(out_nodes.can_pull(), Is().False());


        level_info_test_stream<node_t> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(3,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(2,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(3u));

        // Over-approximation: at (5) we do not know the merge of (3) and (4).
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(4u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().LessThanOrEqualTo(5u));

        AssertThat(out->number_of_sinks[0], Is().EqualTo(2u));
        AssertThat(out->number_of_sinks[1], Is().EqualTo(2u));
      });

      it("can be applied together with reduction rule 2", [&]() {
        /*
                      1                  1     ---- x0
                     / \                ||
                    2   3                3     ---- x1
                   / \ / \              / \
                   F T F 4              F T
                        / \
                        T F
        */

        ptr_t n1 = create_node_ptr(0,0);
        ptr_t n2 = create_node_ptr(1,0);
        ptr_t n3 = create_node_ptr(1,1);
        ptr_t n4 = create_node_ptr(2,0);

        arc_file in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.unsafe_push_node({ n1,n2 });
          aw.unsafe_push_node({ flag(n1),n3 });
          aw.unsafe_push_node({ flag(n3),n4 });

          aw.unsafe_push_sink({ n2,sink_F });
          aw.unsafe_push_sink({ flag(n2),sink_T });
          aw.unsafe_push_sink({ n3,sink_F });
          aw.unsafe_push_sink({ n4,sink_T });
          aw.unsafe_push_sink({ flag(n4),sink_F });

          aw.unsafe_push(create_level_info(0,1u));
          aw.unsafe_push(create_level_info(1,2u));
          aw.unsafe_push(create_level_info(2,1u));
        }

        in->max_1level_cut = 2;

        // Reduce it
        zdd out = reduce<zdd_policy>(in);

        AssertThat(is_canonical(out), Is().True());

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        // n3
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(1, MAX_ID,
                                                              sink_F,
                                                              sink_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n1
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                              create_node_ptr(1,MAX_ID),
                                                              create_node_ptr(1,MAX_ID))));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(0,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(2u));

        // Over-approximation: after (4) is removed we don't know (2) and (3)
        //                     will be merged.
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().LessThanOrEqualTo(4u));

        AssertThat(out->number_of_sinks[0], Is().EqualTo(1u));
        AssertThat(out->number_of_sinks[1], Is().EqualTo(1u));
      });

      it("applies to a single node", [&]() {
        /*
                   1                 T       ---- x0
                  / \      =>
                  T F
        */

        ptr_t n1 = create_node_ptr(0,0);

        arc_file in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.unsafe_push_sink({ n1,sink_T });
          aw.unsafe_push_sink({ flag(n1),sink_F });

          aw.unsafe_push(create_level_info(0,1u));
        }

        in->max_1level_cut = 0;

        // Reduce it
        zdd out = reduce<zdd_policy>(in);

        AssertThat(is_canonical(out), Is().True());

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        // F
        AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> out_meta(out);
        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out->number_of_sinks[0], Is().EqualTo(0u));
        AssertThat(out->number_of_sinks[1], Is().EqualTo(1u));
      });

      it("can reduce the root", [&]() {
        /*
                    1                        ---- x0
                   / \
                   2 F        =>      2      ---- x1
                  / \                / \
                  F T                F T
        */

        ptr_t n1 = create_node_ptr(0,0);
        ptr_t n2 = create_node_ptr(1,0);

        arc_file in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.unsafe_push_node({ n1,n2 });

          aw.unsafe_push_sink({ flag(n1),sink_F });
          aw.unsafe_push_sink({ n2,sink_F });
          aw.unsafe_push_sink({ flag(n2),sink_T });

          aw.unsafe_push(create_level_info(0,1u));
          aw.unsafe_push(create_level_info(1,1u));
        }

        in->max_1level_cut = 1;

        // Reduce it
        zdd out = reduce<zdd_policy>(in);

        AssertThat(is_canonical(out), Is().True());

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        // n2
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(1, MAX_ID, sink_F, sink_T)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(1,1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(out->number_of_sinks[0], Is().EqualTo(1u));
        AssertThat(out->number_of_sinks[1], Is().EqualTo(1u));
      });

      it("can propagate reduction rule 1 up to a sink", [&]() {
        /*
                   1                  F
                  / \
                  | 2         =>
                  |/ \
                  F  F
        */

        ptr_t n1 = create_node_ptr(0,0);
        ptr_t n2 = create_node_ptr(1,0);

        arc_file in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.unsafe_push_node({ flag(n1),n2 });

          aw.unsafe_push_sink({ n1,sink_F });
          aw.unsafe_push_sink({ n2,sink_F });
          aw.unsafe_push_sink({ flag(n2),sink_F });

          aw.unsafe_push(create_level_info(0,1u));
          aw.unsafe_push(create_level_info(1,1u));
        }

        in->max_1level_cut = 1;

        // Reduce it
        zdd out = reduce<zdd_policy>(in);

        AssertThat(out->canonical, Is().True());

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(false)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> out_meta(out);
        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(1u));

        AssertThat(out->number_of_sinks[0], Is().EqualTo(1u));
        AssertThat(out->number_of_sinks[1], Is().EqualTo(0u));
      });

      it("can return non-reducible single-node variable with MAX_ID [1]", [&]() {
        /*
                   1                 1       ---- x42
                  / \      =>       / \
                  F T               F T
        */

        ptr_t n1 = create_node_ptr(42,0);

        arc_file in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.unsafe_push_sink({ n1,sink_F });
          aw.unsafe_push_sink({ flag(n1),sink_T });

          aw.unsafe_push(create_level_info(42,1u));
        }

        in->max_1level_cut = 0;

        // Reduce it
        zdd out = reduce<zdd_policy>(in);

        AssertThat(is_canonical(out), Is().True());

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(42, MAX_ID, sink_F, sink_T)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(42u,1u)));
        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(out->number_of_sinks[0], Is().EqualTo(1u));
        AssertThat(out->number_of_sinks[1], Is().EqualTo(1u));
      });

      it("can return non-reducible single-node variable with MAX_ID [2]", [&]() {
        /*
                   1                 1       ---- x12
                  / \      =>       / \
                  T T               T T
        */

        ptr_t n1 = create_node_ptr(12,0);

        arc_file in;

        { // Garbage collect writer to free write-lock
          arc_writer aw(in);

          aw.unsafe_push_sink({ n1,sink_T });
          aw.unsafe_push_sink({ flag(n1),sink_T });

          aw.unsafe_push(create_level_info(12,1u));
        }

        in->max_1level_cut = 0;

        // Reduce it
        zdd out = reduce<zdd_policy>(in);

        AssertThat(is_canonical(out), Is().True());

        // Check it looks all right
        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(create_node(12, MAX_ID, sink_T, sink_T)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream<node_t> out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(create_level_info(12u,1u)));
        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut_type::ALL], Is().EqualTo(2u));

        AssertThat(out->number_of_sinks[0], Is().EqualTo(0u));
        AssertThat(out->number_of_sinks[1], Is().EqualTo(2u));
      });
    });
  });
 });
