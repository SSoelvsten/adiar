#include "../../test.h"

go_bandit([]() {
  describe("adiar/zdd/project.cpp", []() {
    //////////////////////
    // Sink cases

    // Ø
    shared_levelized_file<zdd::node_t> zdd_empty;
    {
      node_writer nw(zdd_empty);
      nw << node(false);
    }

    // { Ø }
    shared_levelized_file<zdd::node_t> zdd_null;
    {
      node_writer nw(zdd_null);
      nw << node(true);
    }

    //////////////////////
    // Non-terminal edge cases
    ptr_uint64 terminal_F = ptr_uint64(false);
    ptr_uint64 terminal_T = ptr_uint64(true);

    // { Ø, {0}, {1}, {1,2}, {1,3}, {1,3,4} }
    /*
    //                     1         ---- x0
    //                    / \
    //                    2 T        ---- x1
    //                   ||
    //                    3          ---- x2
    //                   / \
    //                   4 T         ---- x3
    //                  / \
    //                  F 5          ---- x4
    //                   / \
    //                   T T
    */
    shared_levelized_file<zdd::node_t> zdd_1;
    {
      node_writer nw(zdd_1);
      nw << node(4, node::MAX_ID, terminal_T, terminal_T)
         << node(3, node::MAX_ID, terminal_F, ptr_uint64(4, ptr_uint64::MAX_ID))
         << node(2, node::MAX_ID, ptr_uint64(3, ptr_uint64::MAX_ID), terminal_T)
         << node(1, node::MAX_ID, ptr_uint64(2, ptr_uint64::MAX_ID), ptr_uint64(2, ptr_uint64::MAX_ID))
         << node(0, node::MAX_ID, ptr_uint64(1, ptr_uint64::MAX_ID), terminal_T)
        ;
    }

    // { {0}, {2}, {0,3}, {2,4} }
    /*
    //                    1         ---- x0
    //                   / \
    //                   |  \       ---- x1
    //                   |   \
    //                   2   |      ---- x2
    //                  / \  |
    //                  F |  3      ---- x3
    //                    | / \
    //                    4 T T     ---- x4
    //                   / \
    //                   T T
    */
    shared_levelized_file<zdd::node_t> zdd_2;
    {
      node_writer nw(zdd_2);
      nw << node(4, node::MAX_ID, terminal_T, terminal_T)
         << node(3, node::MAX_ID, terminal_T, terminal_T)
         << node(2, node::MAX_ID, terminal_F, ptr_uint64(4, ptr_uint64::MAX_ID))
         << node(0, node::MAX_ID, ptr_uint64(2, ptr_uint64::MAX_ID), ptr_uint64(3, ptr_uint64::MAX_ID))
        ;
    }

    // { {0}, {2}, {1,2}, {0,2} }
    /*
    //                    1      ---- x0
    //                   / \
    //                   2 |     ---- x1
    //                  / \|
    //                  3  4     ---- x2
    //                 / \/ \
    //                 F T  T
    */
    shared_levelized_file<zdd::node_t> zdd_3;
    {
      node_writer nw(zdd_3);
      nw << node(2, node::MAX_ID,   terminal_T, terminal_T)
         << node(2, node::MAX_ID-1, terminal_F, terminal_T)
         << node(1, node::MAX_ID,   ptr_uint64(2, ptr_uint64::MAX_ID-1), ptr_uint64(2, ptr_uint64::MAX_ID))
         << node(0, node::MAX_ID,   ptr_uint64(1, ptr_uint64::MAX_ID), ptr_uint64(2, ptr_uint64::MAX_ID))
        ;
    }

    // { {4}, {0,2}, {0,4}, {2,4}, {0,2,4} }
    /*
    //                    _1_      ---- x0
    //                   /   \
    //                   2   3     ---- x2
    //                   \\ / \
    //                     4  5    ---- x4
    //                    / \/ \
    //                    F  T T
    */
    shared_levelized_file<zdd::node_t> zdd_4;
    {
      node_writer nw(zdd_4);
      nw << node(4, node::MAX_ID,   terminal_T, terminal_T)
         << node(4, node::MAX_ID-1, terminal_F, terminal_T)
         << node(2, node::MAX_ID,   ptr_uint64(4, ptr_uint64::MAX_ID-1), ptr_uint64(4, ptr_uint64::MAX_ID))
         << node(2, node::MAX_ID-1, ptr_uint64(4, ptr_uint64::MAX_ID-1), ptr_uint64(4, ptr_uint64::MAX_ID-1))
         << node(0, node::MAX_ID,   ptr_uint64(2, ptr_uint64::MAX_ID-1), ptr_uint64(2, ptr_uint64::MAX_ID))
        ;
    }

    // { ... }
    //
    // See BDD 15 in test/adiar/bdd/test_quantify.cpp for more information. It
    // has been changed to be a valid ZDD.
    //
    // - The nodes 16, 17, n, and o have been changed to have the children
    //   (F,T), (T,T) instead. This removes arcs from (17,o) and (17,n) to (o)
    //   and (n), but not any of the nodes.
    //
    // - The high child of (3) and (c) have been turned into T. Since both
    //   diagrams converge at (4) and (d) then we do not loose a subtree. Yet,
    //   the node (6) is NOT copied over as it only is created at the tuple
    //   (4,c).
    //
    // That is, with i = 5 (similar to BDD 15) we should be expecting 57
    // unreduced nodes after the first sweep. This is still 1.63 of the original
    // 35 nodes.
    /*
    //                        __r1__                    ---- x0
    //                       /      \
    //                       r2     r3                  ---- x1
    //                      /  \   /  \
    //                      1  a   5  d
    //
    //                                   a              ---- x2
    //                                  / \
    //                   _1_            | |             ---- x3
    //                  /   \           | |
    //                  2   3           | |             ---- x4
    //                 / \ / \          | |
    //                 F _4_ T        __b c__           ---- x5
    //                  /   \        /  \ /  \
    //                  5   6        F   d   T          ---- x6
    //                 / \ / \          / \
    //                 |  7  |          | |             ---- x7
    //                 \ / \ /          | |
    //                  8   9           f g             ---- x8
    //                  |\ _X          /| X
    //                  | X  \         |\ |\
    //                  |/ \ /         \ X /
    //                  10 11           h i             ---- x9
    //                  |\ _X          /| X
    //                  | X  \         |\ |\
    //                  |/ \ /         \ X /
    //                  12 13           j k             ---- x10
    //                  |\ _X          /| X
    //                  | X  \         |\ |\
    //                  |/ \ /         \ X /
    //                  14 15           l m             ---- x11
    //                  |\ _X          /| X
    //                  | X  \         |\ |\
    //                  |/ \ /         | X |
    //                  16 17          |/ \|            ---- x12
    //                 / | | \         \\ //
    //                 F T T T          n o             ---- x13
    //                                 /| |\
    //                                F T T T
    */
    const node n5_o  = node(13, node::MAX_ID,   ptr_uint64(true),  ptr_uint64(true));
    const node n5_n  = node(13, node::MAX_ID-1, ptr_uint64(false), ptr_uint64(true));

    const node n5_17 = node(12, node::MAX_ID,   ptr_uint64(true),  ptr_uint64(true));
    const node n5_16 = node(12, node::MAX_ID-1, ptr_uint64(false), ptr_uint64(true));

    const node n5_m  = node(11, node::MAX_ID,   n5_o.uid(),        n5_n.uid());
    const node n5_l  = node(11, node::MAX_ID-1, n5_n.uid(),        n5_o.uid());
    const node n5_15 = node(11, node::MAX_ID-2, n5_17.uid(),       n5_16.uid());
    const node n5_14 = node(11, node::MAX_ID-3, n5_16.uid(),       n5_17.uid());

    const node n5_k  = node(10, node::MAX_ID,   n5_m.uid(),        n5_l.uid());
    const node n5_j  = node(10, node::MAX_ID-1, n5_l.uid(),        n5_m.uid());
    const node n5_13 = node(10, node::MAX_ID-2, n5_15.uid(),       n5_14.uid());
    const node n5_12 = node(10, node::MAX_ID-3, n5_14.uid(),       n5_15.uid());

    const node n5_i  = node(9,  node::MAX_ID,   n5_k.uid(),        n5_j.uid());
    const node n5_h  = node(9,  node::MAX_ID-1, n5_j.uid(),        n5_k.uid());
    const node n5_11 = node(9,  node::MAX_ID-2, n5_13.uid(),       n5_12.uid());
    const node n5_10 = node(9,  node::MAX_ID-3, n5_12.uid(),       n5_13.uid());

    const node n5_g  = node(8,  node::MAX_ID,   n5_i.uid(),        n5_h.uid());
    const node n5_f  = node(8,  node::MAX_ID-1, n5_h.uid(),        n5_i.uid());
    const node n5_9  = node(8,  node::MAX_ID-2, n5_11.uid(),       n5_10.uid());
    const node n5_8  = node(8,  node::MAX_ID-3, n5_10.uid(),       n5_11.uid());

    const node n5_7  = node(7,  node::MAX_ID,   n5_8.uid(),        n5_9.uid());

    const node n5_d  = node(6,  node::MAX_ID,   n5_f.uid(),        n5_g.uid());
    const node n5_6  = node(6,  node::MAX_ID-1, n5_7.uid(),        n5_9.uid());
    const node n5_5  = node(6,  node::MAX_ID-2, n5_8.uid(),        n5_7.uid());

    const node n5_c  = node(5,  node::MAX_ID,   n5_d.uid(),        ptr_uint64(true));
    const node n5_b  = node(5,  node::MAX_ID-1, ptr_uint64(false), n5_d.uid());
    const node n5_4  = node(5,  node::MAX_ID-2, n5_5.uid(),        n5_6.uid());

    const node n5_3  = node(4,  node::MAX_ID,   n5_4.uid(),        ptr_uint64(true));
    const node n5_2  = node(4,  node::MAX_ID-1, ptr_uint64(false), n5_4.uid());

    const node n5_1  = node(3,  node::MAX_ID,   n5_2.uid(),        n5_3.uid());

    const node n5_a  = node(2,  node::MAX_ID,   n5_b.uid(),        n5_c.uid());

    const node n5_r3 = node(1,  node::MAX_ID,   n5_5.uid(),        n5_d.uid());
    const node n5_r2 = node(1,  node::MAX_ID-1, n5_1.uid(),        n5_a.uid());

    const node n5_r1 = node(0,  node::MAX_ID,   n5_r2.uid(),       n5_r3.uid());

    shared_levelized_file<zdd::node_t> zdd_5;
    {
      node_writer nw(zdd_5);
      nw << n5_o  << n5_n
         << n5_17 << n5_16
         << n5_m  << n5_l  << n5_15 << n5_14
         << n5_k  << n5_j  << n5_13 << n5_12
         << n5_i  << n5_h  << n5_11 << n5_10
         << n5_g  << n5_f  << n5_9  << n5_8
         << n5_7
         << n5_d  << n5_6  << n5_5
         << n5_c  << n5_b  << n5_4
         << n5_3  << n5_2
         << n5_1
         << n5_a
         << n5_r3 << n5_r2
         << n5_r1
        ;
    }

    // TODO: Turn 'GreaterThanOrEqualTo' in max 1-level cuts below into an
    // 'EqualTo'.

    describe("zdd_project(const zdd&, const predicate<zdd::label_t>)", [&]() {
      it("returns same file for Ø with dom = {1,3,5,...} [const &]", [&](){
        zdd out = zdd_project(zdd_empty, [](zdd::label_t x) { return x % 2; });
        AssertThat(out.file_ptr(), Is().EqualTo(zdd_empty));
      });

      it("returns same file for { Ø } with dom = {0,2,4,...} [&&]", [&](){
        zdd out = zdd_project(zdd(zdd_null), [](zdd::label_t x) { return !(x % 2); });
        AssertThat(out.file_ptr(), Is().EqualTo(zdd_null));
      });

      describe("quantify_mode == SINGLETON", [&]() {
        quantify_mode = quantify_mode_t::SINGLETON;

        it("computes with dom = Ø to be { Ø } for non-empty input [zdd_1] [const &]", [&](){
          adiar::shared_file<zdd::label_t> dom;

          const zdd in = zdd_1;
          zdd out = zdd_project(in, [](zdd::label_t) { return false; });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("computes with dom = Ø to be { Ø } for non-empty input [zdd_2] [&&]", [&](){
          adiar::shared_file<zdd::label_t> dom;

          zdd out = zdd_project(zdd(zdd_2), [](zdd::label_t) { return false; });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("computes with disjoint dom = { x | x > 2 } to be { Ø } [zdd_3] [&&]", [&](){
          zdd out = zdd_project(zdd(zdd_3), [](zdd::label_t x) { return x > 2; });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("computes zdd_3 with dom = { x | x % 2 == 0 }", [&](){
          /* Expected: { {0}, {2}, {0,2} }
          //
          //                           1    ---- x0
          //                          / \
          //                          | |   ---- x1
          //                          \ /
          //                           2    ---- x2
          //                          / \
          //                          T T
          */

          zdd out = zdd_project(zdd_3, [](zdd::label_t x) { return !(x % 2); });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                         terminal_T,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                         ptr_uint64(2, ptr_uint64::MAX_ID),
                                                         ptr_uint64(2, ptr_uint64::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(2,1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(2u));
        });

        quantify_mode = quantify_mode_t::AUTO;
      });

      describe("quantify_mode == PARTIAL", [&]() {
        quantify_mode = quantify_mode_t::PARTIAL;

        it("computes zdd_3 in a single sweep with dom = { x | x % 2 == 0 }", [&](){
          /* Expected: { {0}, {2}, {0,2} }
          //
          //                           1    ---- x0
          //                          / \
          //                          | |   ---- x1
          //                          \ /
          //                           2    ---- x2
          //                          / \
          //                          T T
          */

          std::vector<zdd::label_t> call_history;
          zdd out = zdd_project(zdd_3, [&call_history](zdd::label_t x) {
            call_history.push_back(x);
            return (x % 2) == 0;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                         terminal_T,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                         ptr_uint64(2, ptr_uint64::MAX_ID),
                                                         ptr_uint64(2, ptr_uint64::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(2,1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(2u));

          // Check call history
          //
          // NOTE: Test failure does NOT indicate a bug, but only indicates a
          //       change. Please verify that this change makes sense and is as
          //       intended.
          AssertThat(call_history.size(), Is().EqualTo(6u));

          // - First check for at least one variable NOT satisfying the predicate.
          AssertThat(call_history.at(0), Is().EqualTo(2u));
          AssertThat(call_history.at(1), Is().EqualTo(1u));

          // - First top-down sweep (root call)
          AssertThat(call_history.at(2), Is().EqualTo(0u));

          // - First top-down sweep
          AssertThat(call_history.at(3),  Is().EqualTo(0u));
          AssertThat(call_history.at(4),  Is().EqualTo(1u));
          AssertThat(call_history.at(5),  Is().EqualTo(2u));
        });

        it("quantifies exploding ZDD 5", [&]() {
          std::vector<zdd::label_t> call_history;
          zdd out = zdd_project(zdd_5, [&call_history](const zdd::label_t x) -> bool {
            call_history.push_back(x);
            return 1 < x;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (o) / (n,o)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(13, node::MAX_ID,
                                                         node::ptr_t(true),
                                                         node::ptr_t(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (n)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(13, node::MAX_ID-1,
                                                         node::ptr_t(false),
                                                         node::ptr_t(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (17)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(12, node::MAX_ID,
                                                         node::ptr_t(true),
                                                         node::ptr_t(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (16)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(12, node::MAX_ID-1,
                                                         node::ptr_t(false),
                                                         node::ptr_t(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (16,o) / (16,o,n)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(12, node::MAX_ID-2,
                                                         node::ptr_t(13, node::MAX_ID),
                                                         node::ptr_t(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (16,n)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(12, node::MAX_ID-3,
                                                         node::ptr_t(13, node::MAX_ID-1),
                                                         node::ptr_t(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (l)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(11, node::MAX_ID,
                                                         node::ptr_t(13, node::MAX_ID-1),
                                                         node::ptr_t(13, node::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (m)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(11, node::MAX_ID-1,
                                                         node::ptr_t(13, node::MAX_ID),
                                                         node::ptr_t(13, node::MAX_ID-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (14)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(11, node::MAX_ID-2,
                                                         node::ptr_t(12, node::MAX_ID-1),
                                                         node::ptr_t(12, node::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (15)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(11, node::MAX_ID-3,
                                                         node::ptr_t(12, node::MAX_ID),
                                                         node::ptr_t(12, node::MAX_ID-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (TODO)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(11, node::MAX_ID-4,
                                                         node::ptr_t(12, node::MAX_ID-2),
                                                         node::ptr_t(12, node::MAX_ID-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (14,l)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(11, node::MAX_ID-5,
                                                         node::ptr_t(12, node::MAX_ID-3),
                                                         node::ptr_t(12, node::MAX_ID-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (15,m)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(11, node::MAX_ID-6,
                                                         node::ptr_t(12, node::MAX_ID-2),
                                                         node::ptr_t(12, node::MAX_ID-3))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (k)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(10, node::MAX_ID,
                                                         node::ptr_t(11, node::MAX_ID-1),
                                                         node::ptr_t(11, node::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (j)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(10, node::MAX_ID-1,
                                                         node::ptr_t(11, node::MAX_ID),
                                                         node::ptr_t(11, node::MAX_ID-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (13)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(10, node::MAX_ID-2,
                                                         node::ptr_t(11, node::MAX_ID-3),
                                                         node::ptr_t(11, node::MAX_ID-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (12)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(10, node::MAX_ID-3,
                                                         node::ptr_t(11, node::MAX_ID-2),
                                                         node::ptr_t(11, node::MAX_ID-3))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (TODO)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(10, node::MAX_ID-4,
                                                         node::ptr_t(11, node::MAX_ID-4),
                                                         node::ptr_t(11, node::MAX_ID-4))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (13,k)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(10, node::MAX_ID-5,
                                                         node::ptr_t(11, node::MAX_ID-6),
                                                         node::ptr_t(11, node::MAX_ID-5))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (12,j)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(10, node::MAX_ID-6,
                                                         node::ptr_t(11, node::MAX_ID-5),
                                                         node::ptr_t(11, node::MAX_ID-6))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (h)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(9, node::MAX_ID,
                                                         node::ptr_t(10, node::MAX_ID-1),
                                                         node::ptr_t(10, node::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (i)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(9, node::MAX_ID-1,
                                                         node::ptr_t(10, node::MAX_ID),
                                                         node::ptr_t(10, node::MAX_ID-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (10)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(9, node::MAX_ID-2,
                                                         node::ptr_t(10, node::MAX_ID-3),
                                                         node::ptr_t(10, node::MAX_ID-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (11)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(9, node::MAX_ID-3,
                                                         node::ptr_t(10, node::MAX_ID-2),
                                                         node::ptr_t(10, node::MAX_ID-3))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (TODO)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(9, node::MAX_ID-4,
                                                         node::ptr_t(10, node::MAX_ID-4),
                                                         node::ptr_t(10, node::MAX_ID-4))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (10,h)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(9, node::MAX_ID-5,
                                                         node::ptr_t(10, node::MAX_ID-6),
                                                         node::ptr_t(10, node::MAX_ID-5))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (11,i)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(9, node::MAX_ID-6,
                                                         node::ptr_t(10, node::MAX_ID-5),
                                                         node::ptr_t(10, node::MAX_ID-6))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (g)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(8, node::MAX_ID,
                                                         node::ptr_t(9, node::MAX_ID-1),
                                                         node::ptr_t(9, node::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (f)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(8, node::MAX_ID-1,
                                                         node::ptr_t(9, node::MAX_ID),
                                                         node::ptr_t(9, node::MAX_ID-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (9)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(8, node::MAX_ID-2,
                                                         node::ptr_t(9, node::MAX_ID-3),
                                                         node::ptr_t(9, node::MAX_ID-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (8)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(8, node::MAX_ID-3,
                                                         node::ptr_t(9, node::MAX_ID-2),
                                                         node::ptr_t(9, node::MAX_ID-3))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (8,g)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(8, node::MAX_ID-4,
                                                         node::ptr_t(9, node::MAX_ID-4),
                                                         node::ptr_t(9, node::MAX_ID-4))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (8,f)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(8, node::MAX_ID-5,
                                                         node::ptr_t(9, node::MAX_ID-5),
                                                         node::ptr_t(9, node::MAX_ID-6))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (7)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(7, node::MAX_ID,
                                                         node::ptr_t(8, node::MAX_ID-3),
                                                         node::ptr_t(8, node::MAX_ID-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (7,g)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(7, node::MAX_ID-1,
                                                         node::ptr_t(8, node::MAX_ID-4),
                                                         node::ptr_t(8, node::MAX_ID-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (d)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(6, node::MAX_ID,
                                                         node::ptr_t(8, node::MAX_ID-1),
                                                         node::ptr_t(8, node::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (6)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(6, node::MAX_ID-1,
                                                         node::ptr_t(7, node::MAX_ID),
                                                         node::ptr_t(8, node::MAX_ID-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(6, node::MAX_ID-2,
                                                         node::ptr_t(8, node::MAX_ID-3),
                                                         node::ptr_t(7, node::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5,d)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(6, node::MAX_ID-3,
                                                         node::ptr_t(8, node::MAX_ID-5),
                                                         node::ptr_t(7, node::MAX_ID-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (c)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::MAX_ID,
                                                         node::ptr_t(6, node::MAX_ID),
                                                         node::ptr_t(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (b,5,d)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::MAX_ID-1,
                                                         node::ptr_t(6, node::MAX_ID-3),
                                                         node::ptr_t(6, node::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (4,b,5,d)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::MAX_ID-2,
                                                         node::ptr_t(6, node::MAX_ID-2),
                                                         node::ptr_t(6, node::MAX_ID-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (3,b,5,d)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::MAX_ID,
                                                         node::ptr_t(5, node::MAX_ID-2),
                                                         node::ptr_t(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (2,b,5,d)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::MAX_ID-1,
                                                         node::ptr_t(5, node::MAX_ID-1),
                                                         node::ptr_t(5, node::MAX_ID-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1,b,5,d)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                         node::ptr_t(4, node::MAX_ID-1),
                                                         node::ptr_t(4, node::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1,a,5,d)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                         node::ptr_t(3, node::MAX_ID),
                                                         node::ptr_t(5, node::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(13u,2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(12u,4u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(11u,7u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(10u,7u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(9u,7u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(8u,6u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(7u,2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(6u,4u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5u,3u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4u,2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...

          // Check call history
          //
          // NOTE: Test failure does NOT indicate a bug, but only indicates a
          //       change. Please verify that this change makes sense and is as
          //       intended.
          AssertThat(call_history.size(), Is().EqualTo(42u));

          // - First check for at least one variable satisfying the predicate.
          AssertThat(call_history.at(0),  Is().EqualTo(13u));
          AssertThat(call_history.at(1),  Is().EqualTo(12u));
          AssertThat(call_history.at(2),  Is().EqualTo(11u));
          AssertThat(call_history.at(3),  Is().EqualTo(10u));
          AssertThat(call_history.at(4),  Is().EqualTo(9u));
          AssertThat(call_history.at(5),  Is().EqualTo(8u));
          AssertThat(call_history.at(6),  Is().EqualTo(7u));
          AssertThat(call_history.at(7),  Is().EqualTo(6u));
          AssertThat(call_history.at(8),  Is().EqualTo(5u));
          AssertThat(call_history.at(9),  Is().EqualTo(4u));
          AssertThat(call_history.at(10), Is().EqualTo(3u));
          AssertThat(call_history.at(11), Is().EqualTo(2u));
          AssertThat(call_history.at(12), Is().EqualTo(1u));

          // - First top-down sweep (root call)
          AssertThat(call_history.at(13), Is().EqualTo(0u));

          // - First top-down sweep
          AssertThat(call_history.at(14), Is().EqualTo(0u));
          AssertThat(call_history.at(15), Is().EqualTo(1u));
          AssertThat(call_history.at(16), Is().EqualTo(2u));
          AssertThat(call_history.at(17), Is().EqualTo(3u));
          AssertThat(call_history.at(18), Is().EqualTo(4u));
          AssertThat(call_history.at(19), Is().EqualTo(5u));
          AssertThat(call_history.at(20), Is().EqualTo(6u));
          AssertThat(call_history.at(21), Is().EqualTo(7u));
          AssertThat(call_history.at(22), Is().EqualTo(8u));
          AssertThat(call_history.at(23), Is().EqualTo(9u));
          AssertThat(call_history.at(24), Is().EqualTo(10u));
          AssertThat(call_history.at(25), Is().EqualTo(11u));
          AssertThat(call_history.at(26), Is().EqualTo(12u));
          AssertThat(call_history.at(27), Is().EqualTo(13u));

          // - Second top-down sweep (root call)
          AssertThat(call_history.at(28), Is().EqualTo(1u));

          // - Second top-down sweep
          AssertThat(call_history.at(29), Is().EqualTo(1u));
          AssertThat(call_history.at(30), Is().EqualTo(2u));
          AssertThat(call_history.at(31), Is().EqualTo(3u));
          AssertThat(call_history.at(32), Is().EqualTo(4u));
          AssertThat(call_history.at(33), Is().EqualTo(5u));
          AssertThat(call_history.at(34), Is().EqualTo(6u));
          AssertThat(call_history.at(35), Is().EqualTo(7u));
          AssertThat(call_history.at(36), Is().EqualTo(8u));
          AssertThat(call_history.at(37), Is().EqualTo(9u));
          AssertThat(call_history.at(38), Is().EqualTo(10u));
          AssertThat(call_history.at(39), Is().EqualTo(11u));
          AssertThat(call_history.at(40), Is().EqualTo(12u));
          AssertThat(call_history.at(41), Is().EqualTo(13u));
        });

        quantify_mode = quantify_mode_t::AUTO;
      });

      describe("quantify_mode == NESTED", [&]() {
        quantify_mode = quantify_mode_t::NESTED;

        it("computes with dom = Ø to be { Ø } for non-empty input [zdd_1]", [&](){
          adiar::shared_file<zdd::label_t> dom;

          const zdd in = zdd_1;
          zdd out = zdd_project(in, [](zdd::label_t) { return false; });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("computes with disjoint dom = { x | x > 2 } to be { Ø } [zdd_3]", [&](){
          zdd out = zdd_project(zdd(zdd_3), [](zdd::label_t x) { return x > 2; });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("computes zdd_3 with dom = { x | x % 2 == 0 }", [&](){
          /* Expected: { {0}, {2}, {0,2} }
          //
          //                           1    ---- x0
          //                          / \
          //                          | |   ---- x1
          //                          \ /
          //                           2    ---- x2
          //                          / \
          //                          T T
          */

          zdd out = zdd_project(zdd_3, [](zdd::label_t x) { return !(x % 2); });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                         terminal_T,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                         ptr_uint64(2, ptr_uint64::MAX_ID),
                                                         ptr_uint64(2, ptr_uint64::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(2,1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(2u));
        });

        it("bails out of inner sweep for zdd_4 with dom = { x | x % 2 == 0 } [const &]", [&](){
          const zdd in = zdd_1;

          /* Expected: { Ø, {0}, {2}, {4} }
          //
          //         1       ---- x0
          //        / \
          //        3 T      ---- x2
          //       / \
          //       5 T       ---- x4
          //      / \
          //      T T
          */
          zdd out = zdd_project(in, [](const zdd::label_t x) { return x % 2 == 0; });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::MAX_ID,
                                                         terminal_T,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                         ptr_uint64(4, ptr_uint64::MAX_ID),
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                         ptr_uint64(2, ptr_uint64::MAX_ID),
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(4,1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(2,1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(4u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(4u));
        });

        quantify_mode = quantify_mode_t::AUTO;
      });

      describe("quantify_mode == AUTO", [&]() {
        quantify_mode = quantify_mode_t::AUTO;

        it("computes with dom = Ø to be { Ø } for non-empty input [zdd_2]", [&](){
          adiar::shared_file<zdd::label_t> dom;

          zdd out = zdd_project(zdd(zdd_2), [](zdd::label_t) { return false; });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("computes with disjoint dom = { x | x > 2 } to be { Ø } [zdd_3]", [&](){
          zdd out = zdd_project(zdd(zdd_3), [](zdd::label_t x) { return x > 2; });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("computes zdd_1 with dom = { x | x % 2 == 1 }", [&](){
          /* Expected: { Ø, {1}, {1,3} }
          //
          //                    2         ---- x1
          //                   / \
          //                   | |
          //                   \ /
          //                    4         ---- x3
          //                   / \
          //                   T T
          */

          zdd out = zdd_project(zdd_1, [](zdd::label_t x) { return x % 2 == 1; });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                         terminal_T,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                         ptr_uint64(3, ptr_uint64::MAX_ID),
                                                         ptr_uint64(3, ptr_uint64::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(3,1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(1,1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(2u));
        });

        it("computes zdd_1 with dom = { x | x != 0,2 }", [&](){
          /* Expected: { Ø, {1}, {1,3}, {1,3,4} }
          //
          //                    2         ---- x1
          //                   / \
          //                   | |
          //                   \ /
          //                    4         ---- x3
          //                   / \
          //                   T 5        ---- x4
          //                    / \
          //                    T T
          */

          zdd out = zdd_project(zdd_1, [](zdd::label_t x) { return x != 0 && x != 2; });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::MAX_ID,
                                                         terminal_T,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                         terminal_T,
                                                         ptr_uint64(4, ptr_uint64::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                         ptr_uint64(3, ptr_uint64::MAX_ID),
                                                         ptr_uint64(3, ptr_uint64::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(4,1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(3,1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(1,1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(3u));
        });

        it("computes zdd_3 with dom = { x | x % 2 == 0 }", [&](){
          /* Expected: { {0}, {2}, {0,2} }
          //
          //                           1    ---- x0
          //                          / \
          //                          | |   ---- x1
          //                          \ /
          //                           2    ---- x2
          //                          / \
          //                          T T
          */

          zdd out = zdd_project(zdd_3, [](zdd::label_t x) { return x % 2 == 0; });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                         terminal_T,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                         ptr_uint64(2, ptr_uint64::MAX_ID),
                                                         ptr_uint64(2, ptr_uint64::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(2,1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(2u));
        });

        it("switches to Nested Sweeping for exploding ZDD 5", [&]() {
          std::vector<zdd::label_t> call_history;
          zdd out = zdd_project(zdd_5, [&call_history](const zdd::label_t x) -> bool {
            call_history.push_back(x);
            return 1 < x;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (o) / (n,o)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(13, node::MAX_ID,
                                                         node::ptr_t(true),
                                                         node::ptr_t(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (n)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(13, node::MAX_ID-1,
                                                         node::ptr_t(false),
                                                         node::ptr_t(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (17)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(12, node::MAX_ID,
                                                         node::ptr_t(true),
                                                         node::ptr_t(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (16)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(12, node::MAX_ID-1,
                                                         node::ptr_t(false),
                                                         node::ptr_t(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (16,o) / (16,o,n)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(12, node::MAX_ID-2,
                                                         node::ptr_t(13, node::MAX_ID),
                                                         node::ptr_t(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (16,n)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(12, node::MAX_ID-3,
                                                         node::ptr_t(13, node::MAX_ID-1),
                                                         node::ptr_t(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (l)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(11, node::MAX_ID,
                                                         node::ptr_t(13, node::MAX_ID-1),
                                                         node::ptr_t(13, node::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (m)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(11, node::MAX_ID-1,
                                                         node::ptr_t(13, node::MAX_ID),
                                                         node::ptr_t(13, node::MAX_ID-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (14)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(11, node::MAX_ID-2,
                                                         node::ptr_t(12, node::MAX_ID-1),
                                                         node::ptr_t(12, node::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (15)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(11, node::MAX_ID-3,
                                                         node::ptr_t(12, node::MAX_ID),
                                                         node::ptr_t(12, node::MAX_ID-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (TODO)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(11, node::MAX_ID-4,
                                                         node::ptr_t(12, node::MAX_ID-2),
                                                         node::ptr_t(12, node::MAX_ID-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (14,l)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(11, node::MAX_ID-5,
                                                         node::ptr_t(12, node::MAX_ID-3),
                                                         node::ptr_t(12, node::MAX_ID-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (15,m)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(11, node::MAX_ID-6,
                                                         node::ptr_t(12, node::MAX_ID-2),
                                                         node::ptr_t(12, node::MAX_ID-3))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (k)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(10, node::MAX_ID,
                                                         node::ptr_t(11, node::MAX_ID-1),
                                                         node::ptr_t(11, node::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (j)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(10, node::MAX_ID-1,
                                                         node::ptr_t(11, node::MAX_ID),
                                                         node::ptr_t(11, node::MAX_ID-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (13)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(10, node::MAX_ID-2,
                                                         node::ptr_t(11, node::MAX_ID-3),
                                                         node::ptr_t(11, node::MAX_ID-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (12)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(10, node::MAX_ID-3,
                                                         node::ptr_t(11, node::MAX_ID-2),
                                                         node::ptr_t(11, node::MAX_ID-3))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (TODO)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(10, node::MAX_ID-4,
                                                         node::ptr_t(11, node::MAX_ID-4),
                                                         node::ptr_t(11, node::MAX_ID-4))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (13,k)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(10, node::MAX_ID-5,
                                                         node::ptr_t(11, node::MAX_ID-6),
                                                         node::ptr_t(11, node::MAX_ID-5))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (12,j)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(10, node::MAX_ID-6,
                                                         node::ptr_t(11, node::MAX_ID-5),
                                                         node::ptr_t(11, node::MAX_ID-6))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (h)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(9, node::MAX_ID,
                                                         node::ptr_t(10, node::MAX_ID-1),
                                                         node::ptr_t(10, node::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (i)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(9, node::MAX_ID-1,
                                                         node::ptr_t(10, node::MAX_ID),
                                                         node::ptr_t(10, node::MAX_ID-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (10)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(9, node::MAX_ID-2,
                                                         node::ptr_t(10, node::MAX_ID-3),
                                                         node::ptr_t(10, node::MAX_ID-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (11)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(9, node::MAX_ID-3,
                                                         node::ptr_t(10, node::MAX_ID-2),
                                                         node::ptr_t(10, node::MAX_ID-3))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (TODO)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(9, node::MAX_ID-4,
                                                         node::ptr_t(10, node::MAX_ID-4),
                                                         node::ptr_t(10, node::MAX_ID-4))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (10,h)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(9, node::MAX_ID-5,
                                                         node::ptr_t(10, node::MAX_ID-6),
                                                         node::ptr_t(10, node::MAX_ID-5))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (11,i)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(9, node::MAX_ID-6,
                                                         node::ptr_t(10, node::MAX_ID-5),
                                                         node::ptr_t(10, node::MAX_ID-6))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (g)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(8, node::MAX_ID,
                                                         node::ptr_t(9, node::MAX_ID-1),
                                                         node::ptr_t(9, node::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (f)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(8, node::MAX_ID-1,
                                                         node::ptr_t(9, node::MAX_ID),
                                                         node::ptr_t(9, node::MAX_ID-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (9)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(8, node::MAX_ID-2,
                                                         node::ptr_t(9, node::MAX_ID-3),
                                                         node::ptr_t(9, node::MAX_ID-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (8)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(8, node::MAX_ID-3,
                                                         node::ptr_t(9, node::MAX_ID-2),
                                                         node::ptr_t(9, node::MAX_ID-3))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (8,g)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(8, node::MAX_ID-4,
                                                         node::ptr_t(9, node::MAX_ID-4),
                                                         node::ptr_t(9, node::MAX_ID-4))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (8,f)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(8, node::MAX_ID-5,
                                                         node::ptr_t(9, node::MAX_ID-5),
                                                         node::ptr_t(9, node::MAX_ID-6))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (7)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(7, node::MAX_ID,
                                                         node::ptr_t(8, node::MAX_ID-3),
                                                         node::ptr_t(8, node::MAX_ID-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (7,g)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(7, node::MAX_ID-1,
                                                         node::ptr_t(8, node::MAX_ID-4),
                                                         node::ptr_t(8, node::MAX_ID-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (d)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(6, node::MAX_ID,
                                                         node::ptr_t(8, node::MAX_ID-1),
                                                         node::ptr_t(8, node::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (6)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(6, node::MAX_ID-1,
                                                         node::ptr_t(7, node::MAX_ID),
                                                         node::ptr_t(8, node::MAX_ID-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(6, node::MAX_ID-2,
                                                         node::ptr_t(8, node::MAX_ID-3),
                                                         node::ptr_t(7, node::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5,d)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(6, node::MAX_ID-3,
                                                         node::ptr_t(8, node::MAX_ID-5),
                                                         node::ptr_t(7, node::MAX_ID-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (c)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::MAX_ID,
                                                         node::ptr_t(6, node::MAX_ID),
                                                         node::ptr_t(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (b,5,d)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::MAX_ID-1,
                                                         node::ptr_t(6, node::MAX_ID-3),
                                                         node::ptr_t(6, node::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (4,b,5,d)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::MAX_ID-2,
                                                         node::ptr_t(6, node::MAX_ID-2),
                                                         node::ptr_t(6, node::MAX_ID-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (3,b,5,d)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::MAX_ID,
                                                         node::ptr_t(5, node::MAX_ID-2),
                                                         node::ptr_t(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (2,b,5,d)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::MAX_ID-1,
                                                         node::ptr_t(5, node::MAX_ID-1),
                                                         node::ptr_t(5, node::MAX_ID-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1,b,5,d)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                         node::ptr_t(4, node::MAX_ID-1),
                                                         node::ptr_t(4, node::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1,a,5,d)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                         node::ptr_t(3, node::MAX_ID),
                                                         node::ptr_t(5, node::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(13u,2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(12u,4u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(11u,7u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(10u,7u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(9u,7u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(8u,6u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(7u,2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(6u,4u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5u,3u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4u,2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...

          // Check call history
          //
          // NOTE: Test failure does NOT indicate a bug, but only indicates a
          //       change. Please verify that this change makes sense and is as
          //       intended.
          AssertThat(call_history.size(), Is().EqualTo(53u));

          // - First check for at least one variable satisfying the predicate.
          AssertThat(call_history.at(0),  Is().EqualTo(13u));
          AssertThat(call_history.at(1),  Is().EqualTo(12u));
          AssertThat(call_history.at(2),  Is().EqualTo(11u));
          AssertThat(call_history.at(3),  Is().EqualTo(10u));
          AssertThat(call_history.at(4),  Is().EqualTo(9u));
          AssertThat(call_history.at(5),  Is().EqualTo(8u));
          AssertThat(call_history.at(6),  Is().EqualTo(7u));
          AssertThat(call_history.at(7),  Is().EqualTo(6u));
          AssertThat(call_history.at(8),  Is().EqualTo(5u));
          AssertThat(call_history.at(9),  Is().EqualTo(4u));
          AssertThat(call_history.at(10), Is().EqualTo(3u));
          AssertThat(call_history.at(11), Is().EqualTo(2u));
          AssertThat(call_history.at(12), Is().EqualTo(1u));

          // - Count number of variables above widest
          AssertThat(call_history.at(13), Is().EqualTo(11u)); // <-- widest level
          AssertThat(call_history.at(14), Is().EqualTo(10u));
          AssertThat(call_history.at(15), Is().EqualTo(9u));
          AssertThat(call_history.at(16), Is().EqualTo(8u));
          AssertThat(call_history.at(17), Is().EqualTo(7u));
          AssertThat(call_history.at(18), Is().EqualTo(6u));
          AssertThat(call_history.at(19), Is().EqualTo(5u));
          AssertThat(call_history.at(20), Is().EqualTo(4u));
          AssertThat(call_history.at(21), Is().EqualTo(3u));
          AssertThat(call_history.at(22), Is().EqualTo(2u));
          AssertThat(call_history.at(23), Is().EqualTo(1u));
          AssertThat(call_history.at(24), Is().EqualTo(0u));

          // - Top-down sweep (root call)
          AssertThat(call_history.at(25), Is().EqualTo(0u));

          // - Top-down sweep
          AssertThat(call_history.at(26), Is().EqualTo(0u));
          AssertThat(call_history.at(27), Is().EqualTo(1u));
          AssertThat(call_history.at(28), Is().EqualTo(2u));
          AssertThat(call_history.at(29), Is().EqualTo(3u));
          AssertThat(call_history.at(30), Is().EqualTo(4u));
          AssertThat(call_history.at(31), Is().EqualTo(5u));
          AssertThat(call_history.at(32), Is().EqualTo(6u));
          AssertThat(call_history.at(33), Is().EqualTo(7u));
          AssertThat(call_history.at(34), Is().EqualTo(8u));
          AssertThat(call_history.at(35), Is().EqualTo(9u));
          AssertThat(call_history.at(36), Is().EqualTo(10u));
          AssertThat(call_history.at(37), Is().EqualTo(11u));
          AssertThat(call_history.at(38), Is().EqualTo(12u));
          AssertThat(call_history.at(39), Is().EqualTo(13u));

          // - Nested Sweeping (x0 is gone)
          AssertThat(call_history.at(40), Is().EqualTo(13u));
          AssertThat(call_history.at(41), Is().EqualTo(12u));
          AssertThat(call_history.at(42), Is().EqualTo(11u));
          AssertThat(call_history.at(43), Is().EqualTo(10u));
          AssertThat(call_history.at(44), Is().EqualTo(9u));
          AssertThat(call_history.at(45), Is().EqualTo(8u));
          AssertThat(call_history.at(46), Is().EqualTo(7u));
          AssertThat(call_history.at(47), Is().EqualTo(6u));
          AssertThat(call_history.at(48), Is().EqualTo(5u));
          AssertThat(call_history.at(49), Is().EqualTo(4u));
          AssertThat(call_history.at(50), Is().EqualTo(3u));
          AssertThat(call_history.at(51), Is().EqualTo(2u));
          AssertThat(call_history.at(52), Is().EqualTo(1u));
        });

        quantify_mode = quantify_mode_t::AUTO;
      });
    });

    describe("zdd_project(const zdd&, const generator<zdd::label_t>&)", [&]() {
      it("returns same file for Ø with dom = {6,4,2,0} [const &]", [&](){
        zdd::label_t var = 6;

        const zdd in = zdd_empty;
        zdd out = zdd_project(in, [&var]() {
          const zdd::label_t ret = var;
          var -= 2;
          return ret;
        });

        AssertThat(out.file_ptr(), Is().EqualTo(in.file_ptr()));
      });

      it("returns same file for { Ø } with dom = {1,3,5} [&&]", [&](){
        zdd::label_t var = 6;

        zdd out = zdd_project(zdd(zdd_null), [&var]() {
          const zdd::label_t ret = var;
          var -= 2;
          return ret;
        });

        AssertThat(out.file_ptr(), Is().EqualTo(zdd_null));
      });

      it("returns same file for Ø with dom = Ø [&&]", [&](){
        zdd out = zdd_project(zdd(zdd_empty), []() { return -1; });
        AssertThat(out.file_ptr(), Is().EqualTo(zdd_empty));
      });

      it("returns same file for { Ø } with dom = Ø [const &]", [&](){
        const zdd in = zdd_null;
        zdd out = zdd_project(in, []() { return -1; });
        AssertThat(out.file_ptr(), Is().EqualTo(in.file_ptr()));
      });

      describe("quantify_mode == SINGLETON / PARTIAL", [&]() {
        quantify_mode = quantify_mode_t::SINGLETON;

        it("collapses zdd_1 with dom = Ø into { Ø } [const &]", [&](){
          const zdd in = zdd_1;
          zdd out = zdd_project(in, []() { return -1; });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("collapses zdd_2 with dom = Ø into { Ø } [&&]", [&](){
          zdd out = zdd_project(zdd(zdd_2), []() { return -1; });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("collapses zdd_3 with dom = Ø into { Ø } [const &]", [&](){
          const zdd in = zdd_3;
          zdd out = zdd_project(in, []() { return -1; });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("collapses zdd_2 with disjoint domain into { Ø } [const &]", [&](){
          zdd::label_t var = 1;

          const zdd in = zdd_2;
          zdd out = zdd_project(in, [&var]() {
            const zdd::label_t ret = var;
            var -= 2;
            return ret;
          });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("computes with disjoint dom to be { Ø } [zdd_3] [&&]", [&](){
          zdd::label_t var = 5;
          zdd out = zdd_project(zdd(zdd_3), [&var]() {
            return 3 <= var && var <= 5 ? var-- : -1;
          });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        //////////////////////
        // Single-node case
        shared_levelized_file<zdd::node_t> zdd_x1;
        {
          node_writer nw(zdd_x1);
          nw << node(1u, node::MAX_ID, ptr_uint64(false), ptr_uint64(true));
        }

        it("returns { Ø } for {1} with dom = {0} [const &]", [&](){
          adiar::shared_file<zdd::label_t> dom;
          { label_writer lw(dom);
            lw << 0;
          }

          const zdd in = zdd_x1;

          /* Expected: { Ø }
           *
           *        T
           */

          zdd::label_t var = 0;
          zdd out = zdd_project(in, [&var]() {
            return var == 0 ? var--: var;
          });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("returns { 1 } for {1} with dom = {1,0} [const &]", [&](){
          adiar::shared_file<zdd::label_t> dom;
          { label_writer lw(dom);
            lw << 0 << 1;
          }

          const zdd in = zdd_x1;

          /* Expected: { 1 }
           *
           *        1       ---- x1
           *       / \
           *       F T
           */
          zdd::label_t var = 1;
          zdd out = zdd_project(in, [&var]() {
            return var <= 1 ? var--: var;
          });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID, zdd::ptr_t(false), zdd::ptr_t(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(1,1u)));
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        //////////////////////
        // Non-terminal general case
        it("computes zdd_1 with dom = {4,3,2} [const &]", [&](){
          const zdd in = zdd_1;

          /* Expected: { Ø, {2}, {3}, {3,4} }
          //
          //         1    ---- x2
          //        / \
          //        2 T   ---- x3
          //       / \
          //       T 3    ---- x4
          //        / \
          //        T T
          */
          zdd::label_t var = 4;
          zdd out = zdd_project(in, [&var]() {
            return 2 <= var && var <= 4 ? var-- : -1;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::MAX_ID,
                                                         terminal_T,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                         terminal_T,
                                                         ptr_uint64(4, ptr_uint64::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                         ptr_uint64(3, ptr_uint64::MAX_ID),
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(4,1u)));


          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(3,1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(2,1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(4u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(4u));
        });

        it("computes zdd_2 with dom = {4,3,2} [&&]", [&](){
          /* Expected: { Ø, {2}, {3}, {2,4} }
          //
          //      1      ---- x2
          //     / \
          //     2  \    ---- x3
          //    / \ |
          //    T T 3    ---- x4
          //       / \
          //       T T
          */
          zdd::label_t var = 4;
          zdd out = zdd_project(zdd_2, [&var]() {
            return 2 <= var && var <= 4 ? var-- : -1;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::MAX_ID,
                                                         terminal_T,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                         terminal_T,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                         ptr_uint64(3, ptr_uint64::MAX_ID),
                                                         ptr_uint64(4, ptr_uint64::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(4,1u)));


          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(3,1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(2,1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(4u));
        });

        it("computes zdd_3 with dom = {4,2,0} [&&]", [&](){
          /* Expected: { {0}, {2}, {0,2} }
          //
          //         1    ---- x0
          //        / \
          //        | |   ---- x1
          //        \ /
          //         2    ---- x2
          //        / \
          //        T T
          */
          zdd::label_t var = 4;
          zdd out = zdd_project(zdd_3, [&var]() {
            const zdd::label_t ret = var;
            var -= 2;
            return ret;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                         terminal_T,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                         ptr_uint64(2, ptr_uint64::MAX_ID),
                                                         ptr_uint64(2, ptr_uint64::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(2,1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(2u));
        });

        it("computes zdd_4 with dom = {4,0} [&&]", [&](){
          /* Expected: { {0}, {4}, {0,4} }
          //
          //       1     ---- x0
          //      / \
          //     /   \   ---- x2
          //     |   |
          //     2   3   ---- x4
          //    / \ / \
          //    F T T T
          */
          zdd::label_t var = 4;
          zdd out = zdd_project(zdd_4, [&var]() {
            const zdd::label_t ret = var;
            var -= 4;
            return ret;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::MAX_ID,
                                                         terminal_T,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::MAX_ID-1,
                                                         terminal_F,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                         ptr_uint64(4, ptr_uint64::MAX_ID-1),
                                                         ptr_uint64(4, ptr_uint64::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(4,2u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(3u));
        });

        it("computes zdd_4 with dom = {4,2} [&&]", [&](){
          /* Expected: { {2}, {4}, {2,4} }
          //
          //       1     ---- x2
          //      / \
          //      2 3    ---- x4
          //     / \||
          //     F  T
          */
          zdd::label_t var = 4;
          zdd out = zdd_project(zdd_4, [&var]() {
            const zdd::label_t res = var;
            if (var == 4) { var -= 2; }
            else          { var = -1; }
            return res;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::MAX_ID,
                                                         terminal_T,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::MAX_ID-1,
                                                         terminal_F,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                         ptr_uint64(4, ptr_uint64::MAX_ID-1),
                                                         ptr_uint64(4, ptr_uint64::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(4,2u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(2,1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(3u));
        });

        quantify_mode = quantify_mode_t::AUTO;
      });

      describe("quantify_mode == NESTED / AUTO", [&]() {
        quantify_mode = quantify_mode_t::NESTED;

        it("collapses zdd_1 with dom = Ø into { Ø } [const &]", [&](){
          const zdd in = zdd_1;
          zdd out = zdd_project(in, []() { return -1; });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("collapses zdd_2 with dom = Ø into { Ø } [&&]", [&](){
          zdd out = zdd_project(zdd(zdd_2), []() { return -1; });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("collapses zdd_3 with dom = Ø into { Ø } [const &]", [&](){
          const zdd in = zdd_3;
          zdd out = zdd_project(in, []() { return -1; });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("collapses zdd_2 with disjoint domain into { Ø } [const &]", [&](){
          zdd::label_t var = 1;

          const zdd in = zdd_2;
          zdd out = zdd_project(in, [&var]() {
            const zdd::label_t ret = var;
            var -= 2;
            return ret;
          });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("computes with disjoint dom to be { Ø } [zdd_3] [&&]", [&](){
          zdd::label_t var = 5;
          zdd out = zdd_project(zdd(zdd_3), [&var]() {
            return 3 <= var && var <= 5 ? var-- : -1;
          });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        // TODO: Shortcut on nothing to do

        //////////////////////
        // Single-node case
        shared_levelized_file<zdd::node_t> zdd_x1;
        {
          node_writer nw(zdd_x1);
          nw << node(1u, node::MAX_ID, ptr_uint64(false), ptr_uint64(true));
        }

        it("returns { Ø } for {1} with dom = {0} [const &]", [&](){
          adiar::shared_file<zdd::label_t> dom;
          { label_writer lw(dom);
            lw << 0;
          }

          const zdd in = zdd_x1;

          /* Expected: { Ø }
           *
           *        T
           */

          zdd::label_t var = 0;
          zdd out = zdd_project(in, [&var]() {
            return var == 0 ? var--: var;
          });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        it("returns { 1 } for {1} with dom = {1,0} [const &]", [&](){
          adiar::shared_file<zdd::label_t> dom;
          { label_writer lw(dom);
            lw << 0 << 1;
          }

          const zdd in = zdd_x1;

          /* Expected: { 1 }
           *
           *        1       ---- x1
           *       / \
           *       F T
           */
          zdd::label_t var = 1;
          zdd out = zdd_project(in, [&var]() {
            return var <= 1 ? var--: var;
          });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID, zdd::ptr_t(false), zdd::ptr_t(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(1,1u)));
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(1u));
        });

        //////////////////////
        // Non-terminal general case
        it("computes zdd_1 with dom = {4,3,2} [const &]", [&](){
          const zdd in = zdd_1;

          /* Expected: { Ø, {2}, {3}, {3,4} }
          //
          //         1    ---- x2
          //        / \
          //        2 T   ---- x3
          //       / \
          //       T 3    ---- x4
          //        / \
          //        T T
          */
          zdd::label_t var = 4;
          zdd out = zdd_project(in, [&var]() {
            return 2 <= var && var <= 4 ? var-- : -1;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::MAX_ID,
                                                         terminal_T,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                         terminal_T,
                                                         ptr_uint64(4, ptr_uint64::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                         ptr_uint64(3, ptr_uint64::MAX_ID),
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(4,1u)));


          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(3,1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(2,1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(4u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(4u));
        });

        it("computes zdd_2 with dom = {4,3,2} [&&]", [&](){
          /* Expected: { Ø, {2}, {3}, {2,4} }
          //
          //      1      ---- x2
          //     / \
          //     2  \    ---- x3
          //    / \ |
          //    T T 3    ---- x4
          //       / \
          //       T T
          */
          zdd::label_t var = 4;
          zdd out = zdd_project(zdd_2, [&var]() {
            return 2 <= var && var <= 4 ? var-- : -1;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::MAX_ID,
                                                         terminal_T,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                         terminal_T,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                         ptr_uint64(3, ptr_uint64::MAX_ID),
                                                         ptr_uint64(4, ptr_uint64::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(4,1u)));


          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(3,1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(2,1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(4u));
        });

        it("computes zdd_3 with dom = {4,2,0} [&&]", [&](){
          /* Expected: { {0}, {2}, {0,2} }
          //
          //         1    ---- x0
          //        / \
          //        | |   ---- x1
          //        \ /
          //         2    ---- x2
          //        / \
          //        T T
          */
          zdd::label_t var = 4;
          zdd out = zdd_project(zdd_3, [&var]() {
            const zdd::label_t ret = var;
            var -= 2;
            return ret;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                         terminal_T,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                         ptr_uint64(2, ptr_uint64::MAX_ID),
                                                         ptr_uint64(2, ptr_uint64::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(2,1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(2u));
        });

        it("computes zdd_4 with dom = {4,0} [&&]", [&](){
          /* Expected: { {0}, {4}, {0,4} }
          //
          //       1     ---- x0
          //      / \
          //     /   \   ---- x2
          //     |   |
          //     2   3   ---- x4
          //    / \ / \
          //    F T T T
          */
          zdd::label_t var = 4;
          zdd out = zdd_project(zdd_4, [&var]() {
            const zdd::label_t ret = var;
            var -= 4;
            return ret;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::MAX_ID,
                                                         terminal_T,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::MAX_ID-1,
                                                         terminal_F,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                         ptr_uint64(4, ptr_uint64::MAX_ID-1),
                                                         ptr_uint64(4, ptr_uint64::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(4,2u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(3u));
        });

        it("computes zdd_4 with dom = {4,2} [&&]", [&](){
          /* Expected: { {2}, {4}, {2,4} }
          //
          //       1     ---- x2
          //      / \
          //      2 3    ---- x4
          //     / \||
          //     F  T
          */
          zdd::label_t var = 4;
          zdd out = zdd_project(zdd_4, [&var]() {
            const zdd::label_t res = var;
            if (var == 4) { var -= 2; }
            else          { var = -1; }
            return res;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::MAX_ID,
                                                         terminal_T,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::MAX_ID-1,
                                                         terminal_F,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                         ptr_uint64(4, ptr_uint64::MAX_ID-1),
                                                         ptr_uint64(4, ptr_uint64::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(4,2u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(2,1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(3u));
        });

        it("bails out of inner sweep for zdd_4 with dom = { x | x % 2 == 0 } [const &]", [&](){
          const zdd in = zdd_1;

          /* Expected: { Ø, {0}, {2}, {4} }
          //
          //         1       ---- x0
          //        / \
          //        3 T      ---- x2
          //       / \
          //       5 T       ---- x4
          //      / \
          //      T T
          */
          zdd::label_t var = 8;
          zdd out = zdd_project(in, [&var]() {
            const zdd::label_t res = var;
            var -= 2;
            return res;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::MAX_ID,
                                                         terminal_T,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                         ptr_uint64(4, ptr_uint64::MAX_ID),
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                         ptr_uint64(2, ptr_uint64::MAX_ID),
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(4,1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(2,1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(4u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(4u));
        });

        quantify_mode = quantify_mode_t::AUTO;
      });
    });

    describe("zdd_project(const zdd&, IT, IT)", [&]() {
      // Since this is merely a wrapper on the generator function, we will just
      // double-check with a few tests.

      describe("quantify_mode == SINGLETON / PARTIAL", [&]() {
        quantify_mode = quantify_mode_t::SINGLETON;

        it("computes zdd_2 with dom = {4,3,2} [&&]", [&](){
          const std::vector<int> dom = {4,3,2};

          /* Expected: { Ø, {2}, {3}, {2,4} }
          //
          //      1      ---- x2
          //     / \
          //     2  \    ---- x3
          //    / \ |
          //    T T 3    ---- x4
          //       / \
          //       T T
          */
          zdd out = zdd_project(zdd_2, dom.cbegin(), dom.cend());

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::MAX_ID,
                                                         terminal_T,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::MAX_ID,
                                                         terminal_T,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                         ptr_uint64(3, ptr_uint64::MAX_ID),
                                                         ptr_uint64(4, ptr_uint64::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(4,1u)));


          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(3,1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(2,1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(4u));
        });

        quantify_mode = quantify_mode_t::AUTO;
      });

      describe("quantify_mode == NESTED / AUTO", [&]() {
        quantify_mode = quantify_mode_t::NESTED;

        it("computes zdd_3 with dom = {4,2,0} [&&]", [&](){
          const std::vector<int> dom = {4,2,0};

          /* Expected: { {0}, {2}, {0,2} }
          //
          //         1    ---- x0
          //        / \
          //        | |   ---- x1
          //        \ /
          //         2    ---- x2
          //        / \
          //        T T
          */
          zdd out = zdd_project(zdd_3, dom.cbegin(), dom.cend());

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                         terminal_T,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                         ptr_uint64(2, ptr_uint64::MAX_ID),
                                                         ptr_uint64(2, ptr_uint64::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(2,1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(2u));
        });

        it("computes zdd_4 with dom = {4,0} [const &]", [&](){
          std::vector<int> dom = {4,0};
          zdd in = zdd_4;

          /* Expected: { {0}, {4}, {0,4} }
          //
          //       1     ---- x0
          //      / \
          //     /   \   ---- x2
          //     |   |
          //     2   3   ---- x4
          //    / \ / \
          //    F T T T
          */
          zdd out = zdd_project(in, dom.cbegin(), dom.cend());

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::MAX_ID,
                                                         terminal_T,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::MAX_ID-1,
                                                         terminal_F,
                                                         terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                         ptr_uint64(4, ptr_uint64::MAX_ID-1),
                                                         ptr_uint64(4, ptr_uint64::MAX_ID))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(4,2u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(0,1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut_type::INTERNAL], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_FALSE], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut_type::INTERNAL_TRUE], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_1level_cut[cut_type::ALL], Is().GreaterThanOrEqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true],  Is().EqualTo(3u));
        });

        quantify_mode = quantify_mode_t::AUTO;
      });
    });
  });
 });
