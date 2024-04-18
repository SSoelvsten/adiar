#include "../../test.h"

go_bandit([]() {
  describe("adiar/zdd/project.cpp", []() {
    //////////////////////
    // Sink cases

    // Ø
    shared_levelized_file<zdd::node_type> zdd_empty;
    {
      node_writer nw(zdd_empty);
      nw << node(false);
    }

    // { Ø }
    shared_levelized_file<zdd::node_type> zdd_null;
    {
      node_writer nw(zdd_null);
      nw << node(true);
    }

    //////////////////////
    // Non-terminal edge cases
    const ptr_uint64 terminal_F = ptr_uint64(false);
    const ptr_uint64 terminal_T = ptr_uint64(true);

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
    shared_levelized_file<zdd::node_type> zdd_1;
    {
      const node n5(4, node::max_id, terminal_T, terminal_T);
      const node n4(3, node::max_id, terminal_F, n5.uid());
      const node n3(2, node::max_id, n4.uid(), terminal_T);
      const node n2(1, node::max_id, n3.uid(), n3.uid());
      const node n1(0, node::max_id, n2.uid(), terminal_T);

      node_writer nw(zdd_1);
      nw << n5 << n4 << n3 << n2 << n1;
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
    shared_levelized_file<zdd::node_type> zdd_2;
    {
      const node n4(4, node::max_id, terminal_T, terminal_T);
      const node n3(3, node::max_id, terminal_T, terminal_T);
      const node n2(2, node::max_id, terminal_F, n4.uid());
      const node n1(0, node::max_id, n2.uid(), n3.uid());

      node_writer nw(zdd_2);
      nw << n4 << n3 << n2 << n1;
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
    shared_levelized_file<zdd::node_type> zdd_3;
    {
      const node n4(2, node::max_id, terminal_T, terminal_T);
      const node n3(2, node::max_id - 1, terminal_F, terminal_T);
      const node n2(1, node::max_id, n3.uid(), n4.uid());
      const node n1(0, node::max_id, n2.uid(), n4.uid());

      node_writer nw(zdd_3);
      nw << n4 << n3 << n2 << n1;
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
    shared_levelized_file<zdd::node_type> zdd_4;
    {
      const node n5(4, node::max_id, terminal_T, terminal_T);
      const node n4(4, node::max_id - 1, terminal_F, terminal_T);
      const node n3(2, node::max_id, n4.uid(), n5.uid());
      const node n2(2, node::max_id - 1, n4.uid(), n4.uid());
      const node n1(0, node::max_id, n2.uid(), n3.uid());

      node_writer nw(zdd_4);
      nw << n5 << n4 << n3 << n2 << n1;
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
    const node n5_o = node(13, node::max_id, ptr_uint64(true), ptr_uint64(true));
    const node n5_n = node(13, node::max_id - 1, ptr_uint64(false), ptr_uint64(true));

    const node n5_17 = node(12, node::max_id, ptr_uint64(true), ptr_uint64(true));
    const node n5_16 = node(12, node::max_id - 1, ptr_uint64(false), ptr_uint64(true));

    const node n5_m  = node(11, node::max_id, n5_o.uid(), n5_n.uid());
    const node n5_l  = node(11, node::max_id - 1, n5_n.uid(), n5_o.uid());
    const node n5_15 = node(11, node::max_id - 2, n5_17.uid(), n5_16.uid());
    const node n5_14 = node(11, node::max_id - 3, n5_16.uid(), n5_17.uid());

    const node n5_k  = node(10, node::max_id, n5_m.uid(), n5_l.uid());
    const node n5_j  = node(10, node::max_id - 1, n5_l.uid(), n5_m.uid());
    const node n5_13 = node(10, node::max_id - 2, n5_15.uid(), n5_14.uid());
    const node n5_12 = node(10, node::max_id - 3, n5_14.uid(), n5_15.uid());

    const node n5_i  = node(9, node::max_id, n5_k.uid(), n5_j.uid());
    const node n5_h  = node(9, node::max_id - 1, n5_j.uid(), n5_k.uid());
    const node n5_11 = node(9, node::max_id - 2, n5_13.uid(), n5_12.uid());
    const node n5_10 = node(9, node::max_id - 3, n5_12.uid(), n5_13.uid());

    const node n5_g = node(8, node::max_id, n5_i.uid(), n5_h.uid());
    const node n5_f = node(8, node::max_id - 1, n5_h.uid(), n5_i.uid());
    const node n5_9 = node(8, node::max_id - 2, n5_11.uid(), n5_10.uid());
    const node n5_8 = node(8, node::max_id - 3, n5_10.uid(), n5_11.uid());

    const node n5_7 = node(7, node::max_id, n5_8.uid(), n5_9.uid());

    const node n5_d = node(6, node::max_id, n5_f.uid(), n5_g.uid());
    const node n5_6 = node(6, node::max_id - 1, n5_7.uid(), n5_9.uid());
    const node n5_5 = node(6, node::max_id - 2, n5_8.uid(), n5_7.uid());

    const node n5_c = node(5, node::max_id, n5_d.uid(), ptr_uint64(true));
    const node n5_b = node(5, node::max_id - 1, ptr_uint64(false), n5_d.uid());
    const node n5_4 = node(5, node::max_id - 2, n5_5.uid(), n5_6.uid());

    const node n5_3 = node(4, node::max_id, n5_4.uid(), ptr_uint64(true));
    const node n5_2 = node(4, node::max_id - 1, ptr_uint64(false), n5_4.uid());

    const node n5_1 = node(3, node::max_id, n5_2.uid(), n5_3.uid());

    const node n5_a = node(2, node::max_id, n5_b.uid(), n5_c.uid());

    const node n5_r3 = node(1, node::max_id, n5_5.uid(), n5_d.uid());
    const node n5_r2 = node(1, node::max_id - 1, n5_1.uid(), n5_a.uid());

    const node n5_r1 = node(0, node::max_id, n5_r2.uid(), n5_r3.uid());

    shared_levelized_file<zdd::node_type> zdd_5;
    {
      node_writer nw(zdd_5);
      nw << n5_o << n5_n << n5_17 << n5_16 << n5_m << n5_l << n5_15 << n5_14 << n5_k << n5_j
         << n5_13 << n5_12 << n5_i << n5_h << n5_11 << n5_10 << n5_g << n5_f << n5_9 << n5_8 << n5_7
         << n5_d << n5_6 << n5_5 << n5_c << n5_b << n5_4 << n5_3 << n5_2 << n5_1 << n5_a << n5_r3
         << n5_r2 << n5_r1;
    }

    // { ... }
    //
    // See BDD 16 in test/adiar/bdd/test_quantify.cpp for more details.
    /*
    //               _1_              ---- x0
    //              /   \
    //            _2_    \            ---- x1
    //           /   \    \
    //          3     4    \          ---- x2
    //         / \   / \    \
    //         5 6   7 8     \        ---- x3
    //        /| |\ /| |\     \
    //        ---- 9 ----     |       ---- x4
    //        \.|./  \.|./    |
    //          a      b      |       ---- x5
    //         / \    / \     |
    //         F T    T T     c       ---- x6
    //                       / \
    //                       F T
    */
    shared_levelized_file<zdd::node_type> zdd_6;

    {
      const node nc = node(6, node::max_id, ptr_uint64(false), ptr_uint64(true));
      const node nb = node(5, node::max_id, ptr_uint64(true), ptr_uint64(true));
      const node na = node(5, node::max_id - 1, ptr_uint64(false), ptr_uint64(true));

      const node n9_3 = node(4, node::max_id, na.uid(), nb.uid());
      const node n9_2 = node(4, node::max_id - 1, na.uid(), nb.uid());
      const node n9_1 = node(4, node::max_id - 2, na.uid(), nb.uid());

      const node n8 = node(3, node::max_id, n9_2.uid(), n9_3.uid());
      const node n7 = node(3, node::max_id - 1, n9_1.uid(), n9_3.uid());
      const node n6 = node(3, node::max_id - 2, n9_2.uid(), n9_1.uid());
      const node n5 = node(3, node::max_id - 3, n9_1.uid(), n9_2.uid());
      const node n4 = node(2, node::max_id, n7.uid(), n8.uid());
      const node n3 = node(2, node::max_id - 1, n5.uid(), n6.uid());
      const node n2 = node(1, node::max_id, n3.uid(), n4.uid());
      const node n1 = node(0, node::max_id, n2.uid(), nc.uid());

      node_writer nw(zdd_6);
      nw << nc << nb << na << n9_3 << n9_2 << n9_1 << n8 << n7 << n6 << n5 << n4 << n3 << n2 << n1;
    }

    // TODO: Turn 'GreaterThanOrEqualTo' in max 1-level cuts below into an
    // 'EqualTo'.

    describe("zdd_project(const zdd&, const predicate<zdd::label_type>)", [&]() {
      it("returns same file for Ø with dom = {1,3,5,...} [const &]", [&]() {
        zdd out = zdd_project(zdd_empty, [](zdd::label_type x) { return x % 2; });
        AssertThat(out.file_ptr(), Is().EqualTo(zdd_empty));
      });

      it("returns same file for { Ø } with dom = {0,2,4,...} [&&]", [&]() {
        zdd out = zdd_project(zdd(zdd_null), [](zdd::label_type x) { return !(x % 2); });
        AssertThat(out.file_ptr(), Is().EqualTo(zdd_null));
      });

      describe("algorithm: Singleton", [&]() {
        const exec_policy& ep = exec_policy::quantify::Singleton;

        it("computes with dom = Ø to be { Ø } for non-empty input [zdd_1] [const &]", [&]() {
          shared_file<zdd::label_type> dom;

          const zdd in = zdd_1;
          zdd out      = zdd_project(ep, in, [](zdd::label_type) { return false; });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("computes with dom = Ø to be { Ø } for non-empty input [zdd_2] [&&]", [&]() {
          shared_file<zdd::label_type> dom;

          zdd out = zdd_project(ep, zdd(zdd_2), [](zdd::label_type) { return false; });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("computes with disjoint dom = { x | x > 2 } to be { Ø } [zdd_3] [&&]", [&]() {
          zdd out = zdd_project(ep, zdd(zdd_3), [](zdd::label_type x) { return x > 2; });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("computes zdd_3 with dom = { x | x % 2 == 0 }", [&]() {
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

          zdd out = zdd_project(ep, zdd_3, [](zdd::label_type x) { return !(x % 2); });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_T, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0,
                                       node::max_id,
                                       ptr_uint64(2, ptr_uint64::max_id),
                                       ptr_uint64(2, ptr_uint64::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
        });
      });

      describe("algorithm: Nested, max: 0", [&]() {
        const exec_policy ep =
          exec_policy::quantify::Nested & exec_policy::quantify::transposition_max(0);

        it("computes with dom = Ø to be { Ø } for non-empty input [zdd_1]", [&]() {
          shared_file<zdd::label_type> dom;

          const zdd in = zdd_1;
          zdd out      = zdd_project(ep, in, [](zdd::label_type) { return false; });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("computes with disjoint dom = { x | x > 2 } to be { Ø } [zdd_3]", [&]() {
          zdd out = zdd_project(ep, zdd(zdd_3), [](zdd::label_type x) { return x > 2; });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("computes zdd_3 with dom = { x | x % 2 == 0 }", [&]() {
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

          zdd out = zdd_project(ep, zdd_3, [](zdd::label_type x) { return !(x % 2); });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_T, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0,
                                       node::max_id,
                                       ptr_uint64(2, ptr_uint64::max_id),
                                       ptr_uint64(2, ptr_uint64::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
        });

        it("bails out of inner sweep for zdd_4 with dom = { x | x % 2 == 0 } [const &]", [&]() {
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
          zdd out = zdd_project(ep, in, [](const zdd::label_type x) { return x % 2 == 0; });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id, terminal_T, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(2, node::max_id, ptr_uint64(4, ptr_uint64::max_id), terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, node::max_id, ptr_uint64(2, ptr_uint64::max_id), terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(4u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(4u));
        });

        it("accounts for number of root arcs from Outer Sweep [const &]", [&]() {
          const zdd in = zdd_6;

          /* Expected: { Ø, {5} }
          //
          //         1       ---- x5
          //        / \
          //        T T
          */
          zdd out = zdd_project(ep, in, [](const zdd::label_type x) { return x == 5; });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id, terminal_T, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(5, 1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
        });
      });

      describe("algorithm: Nested, max: 1+", [&]() {
        const exec_policy ep = exec_policy::quantify::Nested
          & exec_policy::quantify::transposition_growth(1.5)
          & exec_policy::quantify::transposition_max(2);

        it("computes with dom = Ø to be { Ø } for non-empty input [zdd_2]", [&]() {
          shared_file<zdd::label_type> dom;

          zdd out = zdd_project(ep, zdd(zdd_2), [](zdd::label_type) { return false; });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("computes with disjoint dom = { x | x > 2 } to be { Ø } [zdd_3]", [&]() {
          zdd out = zdd_project(ep, zdd(zdd_3), [](zdd::label_type x) { return x > 2; });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("finishes during initial transposition in zdd_3 and dom = { x | x % 2 == 0 }", [&]() {
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

          std::vector<zdd::label_type> call_history;
          zdd out = zdd_project(ep, zdd_3, [&call_history](zdd::label_type x) {
            call_history.push_back(x);
            return (x % 2) == 0;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_T, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0,
                                       node::max_id,
                                       ptr_uint64(2, ptr_uint64::max_id),
                                       ptr_uint64(2, ptr_uint64::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));

          // Check call history
          //
          // NOTE: Test failure does NOT indicate a bug, but only indicates a
          //       change. Please verify that this change makes sense and is as
          //       intended.
          AssertThat(call_history.size(), Is().EqualTo(7u));

          // - First check for at least one variable NOT satisfying the predicate.
          AssertThat(call_history.at(0), Is().EqualTo(2u));
          AssertThat(call_history.at(1), Is().EqualTo(1u));

          // - Upper bound partial quantifications; capped at N/3(ish) nodes.
          AssertThat(call_history.at(2), Is().EqualTo(0u));
          AssertThat(call_history.at(3), Is().EqualTo(1u)); // 2 out of 4 nodes

          // - First top-down sweep
          AssertThat(call_history.at(4), Is().EqualTo(0u));
          AssertThat(call_history.at(5), Is().EqualTo(1u));
          AssertThat(call_history.at(6), Is().EqualTo(2u));
        });

        it("computes zdd_1 with dom = { x | x % 2 == 1 }", [&]() {
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

          zdd out = zdd_project(ep, zdd_1, [](zdd::label_type x) { return x % 2 == 1; });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id, terminal_T, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(1,
                                       node::max_id,
                                       ptr_uint64(3, ptr_uint64::max_id),
                                       ptr_uint64(3, ptr_uint64::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
        });

        it("computes zdd_1 with dom = { x | x != 0,2 }", [&]() {
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

          zdd out = zdd_project(ep, zdd_1, [](zdd::label_type x) { return x != 0 && x != 2; });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id, terminal_T, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(3, node::max_id, terminal_T, ptr_uint64(4, ptr_uint64::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(1,
                                       node::max_id,
                                       ptr_uint64(3, ptr_uint64::max_id),
                                       ptr_uint64(3, ptr_uint64::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(3u));
        });

        it("computes zdd_3 with dom = { x | x % 2 == 0 }", [&]() {
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

          zdd out = zdd_project(ep, zdd_3, [](zdd::label_type x) { return x % 2 == 0; });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_T, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0,
                                       node::max_id,
                                       ptr_uint64(2, ptr_uint64::max_id),
                                       ptr_uint64(2, ptr_uint64::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
        });

        it("computes zdd_4 with dom = { x | x <= 2 }", [&]() {
          std::vector<zdd::label_type> call_history;
          zdd out = zdd_project(ep, zdd_4, [&call_history](zdd::label_type x) {
            call_history.push_back(x);
            return x <= 2;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (2) / (3)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_T, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0,
                                       node::max_id,
                                       ptr_uint64(2, ptr_uint64::max_id),
                                       ptr_uint64(2, ptr_uint64::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));

          // Check call history
          //
          // NOTE: Test failure does NOT indicate a bug, but only indicates a
          //       change. Please verify that this change makes sense and is as
          //       intended.
          AssertThat(call_history.size(), Is().EqualTo(5u));

          // - First check for at least one variable satisfying the predicate.
          AssertThat(call_history.at(0), Is().EqualTo(4u));

          // - Upper bound partial quantifications; capped at N/3(ish) nodes.
          AssertThat(call_history.at(1), Is().EqualTo(0u));
          AssertThat(call_history.at(2), Is().EqualTo(2u)); // width and 3 out of 5 nodes

          // - Nested sweep checking if any other than the deepest variable needs quantifying
          AssertThat(call_history.at(3), Is().EqualTo(2u));
          AssertThat(call_history.at(4), Is().EqualTo(0u));
        });

        it("quantifies exploding zdd_5 with unbounded transpositions", [&]() {
          const exec_policy ep = exec_policy::quantify::Nested
            & exec_policy::quantify::transposition_growth::max()
            & exec_policy::quantify::transposition_max::max();

          std::vector<zdd::label_type> call_history;
          zdd out = zdd_project(ep, zdd_5, [&call_history](const zdd::label_type x) -> bool {
            call_history.push_back(x);
            return 1 < x;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (o) / (n,o)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(13, node::max_id, node::pointer_type(true), node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (n)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(
                       13, node::max_id - 1, node::pointer_type(false), node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (17)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(12, node::max_id, node::pointer_type(true), node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (16)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(
                       12, node::max_id - 1, node::pointer_type(false), node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (16,o) / (16,o,n)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(12,
                                       node::max_id - 2,
                                       node::pointer_type(13, node::max_id),
                                       node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (16,n)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(12,
                                       node::max_id - 3,
                                       node::pointer_type(13, node::max_id - 1),
                                       node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (l)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(11,
                                       node::max_id,
                                       node::pointer_type(13, node::max_id - 1),
                                       node::pointer_type(13, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (m)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(11,
                                       node::max_id - 1,
                                       node::pointer_type(13, node::max_id),
                                       node::pointer_type(13, node::max_id - 1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (14)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(11,
                                       node::max_id - 2,
                                       node::pointer_type(12, node::max_id - 1),
                                       node::pointer_type(12, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (15)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(11,
                                       node::max_id - 3,
                                       node::pointer_type(12, node::max_id),
                                       node::pointer_type(12, node::max_id - 1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (TODO)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(11,
                                       node::max_id - 4,
                                       node::pointer_type(12, node::max_id - 2),
                                       node::pointer_type(12, node::max_id - 2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (14,l)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(11,
                                       node::max_id - 5,
                                       node::pointer_type(12, node::max_id - 3),
                                       node::pointer_type(12, node::max_id - 2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (15,m)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(11,
                                       node::max_id - 6,
                                       node::pointer_type(12, node::max_id - 2),
                                       node::pointer_type(12, node::max_id - 3))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (k)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(10,
                                       node::max_id,
                                       node::pointer_type(11, node::max_id - 1),
                                       node::pointer_type(11, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (j)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(10,
                                       node::max_id - 1,
                                       node::pointer_type(11, node::max_id),
                                       node::pointer_type(11, node::max_id - 1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (13)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(10,
                                       node::max_id - 2,
                                       node::pointer_type(11, node::max_id - 3),
                                       node::pointer_type(11, node::max_id - 2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (12)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(10,
                                       node::max_id - 3,
                                       node::pointer_type(11, node::max_id - 2),
                                       node::pointer_type(11, node::max_id - 3))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (TODO)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(10,
                                       node::max_id - 4,
                                       node::pointer_type(11, node::max_id - 4),
                                       node::pointer_type(11, node::max_id - 4))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (13,k)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(10,
                                       node::max_id - 5,
                                       node::pointer_type(11, node::max_id - 6),
                                       node::pointer_type(11, node::max_id - 5))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (12,j)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(10,
                                       node::max_id - 6,
                                       node::pointer_type(11, node::max_id - 5),
                                       node::pointer_type(11, node::max_id - 6))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (h)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(9,
                                       node::max_id,
                                       node::pointer_type(10, node::max_id - 1),
                                       node::pointer_type(10, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (i)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(9,
                                       node::max_id - 1,
                                       node::pointer_type(10, node::max_id),
                                       node::pointer_type(10, node::max_id - 1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (10)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(9,
                                       node::max_id - 2,
                                       node::pointer_type(10, node::max_id - 3),
                                       node::pointer_type(10, node::max_id - 2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (11)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(9,
                                       node::max_id - 3,
                                       node::pointer_type(10, node::max_id - 2),
                                       node::pointer_type(10, node::max_id - 3))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (TODO)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(9,
                                       node::max_id - 4,
                                       node::pointer_type(10, node::max_id - 4),
                                       node::pointer_type(10, node::max_id - 4))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (10,h)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(9,
                                       node::max_id - 5,
                                       node::pointer_type(10, node::max_id - 6),
                                       node::pointer_type(10, node::max_id - 5))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (11,i)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(9,
                                       node::max_id - 6,
                                       node::pointer_type(10, node::max_id - 5),
                                       node::pointer_type(10, node::max_id - 6))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (g)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(8,
                                       node::max_id,
                                       node::pointer_type(9, node::max_id - 1),
                                       node::pointer_type(9, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (f)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(8,
                                       node::max_id - 1,
                                       node::pointer_type(9, node::max_id),
                                       node::pointer_type(9, node::max_id - 1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (9)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(8,
                                       node::max_id - 2,
                                       node::pointer_type(9, node::max_id - 3),
                                       node::pointer_type(9, node::max_id - 2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (8)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(8,
                                       node::max_id - 3,
                                       node::pointer_type(9, node::max_id - 2),
                                       node::pointer_type(9, node::max_id - 3))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (8,g)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(8,
                                       node::max_id - 4,
                                       node::pointer_type(9, node::max_id - 4),
                                       node::pointer_type(9, node::max_id - 4))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (8,f)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(8,
                                       node::max_id - 5,
                                       node::pointer_type(9, node::max_id - 5),
                                       node::pointer_type(9, node::max_id - 6))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (7)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(7,
                                       node::max_id,
                                       node::pointer_type(8, node::max_id - 3),
                                       node::pointer_type(8, node::max_id - 2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (7,g)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(7,
                                       node::max_id - 1,
                                       node::pointer_type(8, node::max_id - 4),
                                       node::pointer_type(8, node::max_id - 2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (d)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(6,
                                       node::max_id,
                                       node::pointer_type(8, node::max_id - 1),
                                       node::pointer_type(8, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (6)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(6,
                                       node::max_id - 1,
                                       node::pointer_type(7, node::max_id),
                                       node::pointer_type(8, node::max_id - 2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(6,
                                       node::max_id - 2,
                                       node::pointer_type(8, node::max_id - 3),
                                       node::pointer_type(7, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5,d)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(6,
                                       node::max_id - 3,
                                       node::pointer_type(8, node::max_id - 5),
                                       node::pointer_type(7, node::max_id - 1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (c)
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(
              5, node::max_id, node::pointer_type(6, node::max_id), node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (b,5,d)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(5,
                                       node::max_id - 1,
                                       node::pointer_type(6, node::max_id - 3),
                                       node::pointer_type(6, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (4,b,5,d)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(5,
                                       node::max_id - 2,
                                       node::pointer_type(6, node::max_id - 2),
                                       node::pointer_type(6, node::max_id - 1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (3,b,5,d)
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(
              4, node::max_id, node::pointer_type(5, node::max_id - 2), node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (2,b,5,d)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(4,
                                       node::max_id - 1,
                                       node::pointer_type(5, node::max_id - 1),
                                       node::pointer_type(5, node::max_id - 2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1,b,5,d)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(3,
                                       node::max_id,
                                       node::pointer_type(4, node::max_id - 1),
                                       node::pointer_type(4, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1,a,5,d)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(2,
                                       node::max_id,
                                       node::pointer_type(3, node::max_id),
                                       node::pointer_type(5, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(13u, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(12u, 4u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(11u, 7u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(10u, 7u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(9u, 7u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(8u, 6u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(7u, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(6u, 4u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5u, 3u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4u, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...

          // Check call history
          //
          // NOTE: Test failure does NOT indicate a bug, but only indicates a
          //       change. Please verify that this change makes sense and is as
          //       intended.
          AssertThat(call_history.size(), Is().EqualTo(47u));

          // - First check for at least one variable satisfying the predicate.
          AssertThat(call_history.at(0), Is().EqualTo(13u));
          AssertThat(call_history.at(1), Is().EqualTo(12u));
          AssertThat(call_history.at(2), Is().EqualTo(11u));
          AssertThat(call_history.at(3), Is().EqualTo(10u));
          AssertThat(call_history.at(4), Is().EqualTo(9u));
          AssertThat(call_history.at(5), Is().EqualTo(8u));
          AssertThat(call_history.at(6), Is().EqualTo(7u));
          AssertThat(call_history.at(7), Is().EqualTo(6u));
          AssertThat(call_history.at(8), Is().EqualTo(5u));
          AssertThat(call_history.at(9), Is().EqualTo(4u));
          AssertThat(call_history.at(10), Is().EqualTo(3u));
          AssertThat(call_history.at(11), Is().EqualTo(2u));
          AssertThat(call_history.at(12), Is().EqualTo(1u));

          // - Upper bound partial quantifications; capped at N/3(ish) nodes.
          AssertThat(call_history.at(13), Is().EqualTo(0u));
          AssertThat(call_history.at(14), Is().EqualTo(1u));
          AssertThat(call_history.at(15), Is().EqualTo(2u));
          AssertThat(call_history.at(16), Is().EqualTo(3u));
          AssertThat(call_history.at(17), Is().EqualTo(4u));
          AssertThat(call_history.at(18), Is().EqualTo(5u));
          AssertThat(call_history.at(19), Is().EqualTo(6u)); // 13 out of 34 nodes

          // - First top-down sweep
          AssertThat(call_history.at(20), Is().EqualTo(0u));
          AssertThat(call_history.at(21), Is().EqualTo(1u));
          AssertThat(call_history.at(22), Is().EqualTo(2u));
          AssertThat(call_history.at(23), Is().EqualTo(3u));
          AssertThat(call_history.at(24), Is().EqualTo(4u));
          AssertThat(call_history.at(25), Is().EqualTo(5u));
          AssertThat(call_history.at(26), Is().EqualTo(6u));
          AssertThat(call_history.at(27), Is().EqualTo(7u));
          AssertThat(call_history.at(28), Is().EqualTo(8u));
          AssertThat(call_history.at(29), Is().EqualTo(9u));
          AssertThat(call_history.at(30), Is().EqualTo(10u));
          AssertThat(call_history.at(31), Is().EqualTo(11u));
          AssertThat(call_history.at(32), Is().EqualTo(12u));
          AssertThat(call_history.at(33), Is().EqualTo(13u));

          // - Second top-down sweep
          AssertThat(call_history.at(34), Is().EqualTo(1u));
          AssertThat(call_history.at(35), Is().EqualTo(2u));
          AssertThat(call_history.at(36), Is().EqualTo(3u));
          AssertThat(call_history.at(37), Is().EqualTo(4u));
          AssertThat(call_history.at(38), Is().EqualTo(5u));
          AssertThat(call_history.at(39), Is().EqualTo(6u));
          AssertThat(call_history.at(40), Is().EqualTo(7u));
          AssertThat(call_history.at(41), Is().EqualTo(8u));
          AssertThat(call_history.at(42), Is().EqualTo(9u));
          AssertThat(call_history.at(43), Is().EqualTo(10u));
          AssertThat(call_history.at(44), Is().EqualTo(11u));
          AssertThat(call_history.at(45), Is().EqualTo(12u));
          AssertThat(call_history.at(46), Is().EqualTo(13u));
        });

        it("switches to Nested Sweeping for exploding ZDD 5", [&]() {
          const exec_policy ep = exec_policy::quantify::Nested
            & exec_policy::quantify::transposition_growth(1.5)
            & exec_policy::quantify::transposition_max::max();

          std::vector<zdd::label_type> call_history;
          zdd out = zdd_project(ep, zdd_5, [&call_history](const zdd::label_type x) -> bool {
            call_history.push_back(x);
            return 1 < x;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (o) / (n,o)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(13, node::max_id, node::pointer_type(true), node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (n)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(
                       13, node::max_id - 1, node::pointer_type(false), node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (17)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(12, node::max_id, node::pointer_type(true), node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (16)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(
                       12, node::max_id - 1, node::pointer_type(false), node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (16,o) / (16,o,n)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(12,
                                       node::max_id - 2,
                                       node::pointer_type(13, node::max_id),
                                       node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (16,n)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(12,
                                       node::max_id - 3,
                                       node::pointer_type(13, node::max_id - 1),
                                       node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (l)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(11,
                                       node::max_id,
                                       node::pointer_type(13, node::max_id - 1),
                                       node::pointer_type(13, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (m)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(11,
                                       node::max_id - 1,
                                       node::pointer_type(13, node::max_id),
                                       node::pointer_type(13, node::max_id - 1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (14)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(11,
                                       node::max_id - 2,
                                       node::pointer_type(12, node::max_id - 1),
                                       node::pointer_type(12, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (15)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(11,
                                       node::max_id - 3,
                                       node::pointer_type(12, node::max_id),
                                       node::pointer_type(12, node::max_id - 1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (TODO)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(11,
                                       node::max_id - 4,
                                       node::pointer_type(12, node::max_id - 2),
                                       node::pointer_type(12, node::max_id - 2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (14,l)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(11,
                                       node::max_id - 5,
                                       node::pointer_type(12, node::max_id - 3),
                                       node::pointer_type(12, node::max_id - 2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (15,m)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(11,
                                       node::max_id - 6,
                                       node::pointer_type(12, node::max_id - 2),
                                       node::pointer_type(12, node::max_id - 3))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (k)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(10,
                                       node::max_id,
                                       node::pointer_type(11, node::max_id - 1),
                                       node::pointer_type(11, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (j)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(10,
                                       node::max_id - 1,
                                       node::pointer_type(11, node::max_id),
                                       node::pointer_type(11, node::max_id - 1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (13)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(10,
                                       node::max_id - 2,
                                       node::pointer_type(11, node::max_id - 3),
                                       node::pointer_type(11, node::max_id - 2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (12)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(10,
                                       node::max_id - 3,
                                       node::pointer_type(11, node::max_id - 2),
                                       node::pointer_type(11, node::max_id - 3))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (TODO)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(10,
                                       node::max_id - 4,
                                       node::pointer_type(11, node::max_id - 4),
                                       node::pointer_type(11, node::max_id - 4))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (13,k)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(10,
                                       node::max_id - 5,
                                       node::pointer_type(11, node::max_id - 6),
                                       node::pointer_type(11, node::max_id - 5))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (12,j)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(10,
                                       node::max_id - 6,
                                       node::pointer_type(11, node::max_id - 5),
                                       node::pointer_type(11, node::max_id - 6))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (h)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(9,
                                       node::max_id,
                                       node::pointer_type(10, node::max_id - 1),
                                       node::pointer_type(10, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (i)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(9,
                                       node::max_id - 1,
                                       node::pointer_type(10, node::max_id),
                                       node::pointer_type(10, node::max_id - 1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (10)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(9,
                                       node::max_id - 2,
                                       node::pointer_type(10, node::max_id - 3),
                                       node::pointer_type(10, node::max_id - 2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (11)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(9,
                                       node::max_id - 3,
                                       node::pointer_type(10, node::max_id - 2),
                                       node::pointer_type(10, node::max_id - 3))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (TODO)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(9,
                                       node::max_id - 4,
                                       node::pointer_type(10, node::max_id - 4),
                                       node::pointer_type(10, node::max_id - 4))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (10,h)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(9,
                                       node::max_id - 5,
                                       node::pointer_type(10, node::max_id - 6),
                                       node::pointer_type(10, node::max_id - 5))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (11,i)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(9,
                                       node::max_id - 6,
                                       node::pointer_type(10, node::max_id - 5),
                                       node::pointer_type(10, node::max_id - 6))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (g)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(8,
                                       node::max_id,
                                       node::pointer_type(9, node::max_id - 1),
                                       node::pointer_type(9, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (f)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(8,
                                       node::max_id - 1,
                                       node::pointer_type(9, node::max_id),
                                       node::pointer_type(9, node::max_id - 1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (9)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(8,
                                       node::max_id - 2,
                                       node::pointer_type(9, node::max_id - 3),
                                       node::pointer_type(9, node::max_id - 2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (8)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(8,
                                       node::max_id - 3,
                                       node::pointer_type(9, node::max_id - 2),
                                       node::pointer_type(9, node::max_id - 3))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (8,g)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(8,
                                       node::max_id - 4,
                                       node::pointer_type(9, node::max_id - 4),
                                       node::pointer_type(9, node::max_id - 4))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (8,f)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(8,
                                       node::max_id - 5,
                                       node::pointer_type(9, node::max_id - 5),
                                       node::pointer_type(9, node::max_id - 6))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (7)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(7,
                                       node::max_id,
                                       node::pointer_type(8, node::max_id - 3),
                                       node::pointer_type(8, node::max_id - 2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (7,g)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(7,
                                       node::max_id - 1,
                                       node::pointer_type(8, node::max_id - 4),
                                       node::pointer_type(8, node::max_id - 2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (d)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(6,
                                       node::max_id,
                                       node::pointer_type(8, node::max_id - 1),
                                       node::pointer_type(8, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (6)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(6,
                                       node::max_id - 1,
                                       node::pointer_type(7, node::max_id),
                                       node::pointer_type(8, node::max_id - 2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(6,
                                       node::max_id - 2,
                                       node::pointer_type(8, node::max_id - 3),
                                       node::pointer_type(7, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5,d)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(6,
                                       node::max_id - 3,
                                       node::pointer_type(8, node::max_id - 5),
                                       node::pointer_type(7, node::max_id - 1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (c)
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(
              5, node::max_id, node::pointer_type(6, node::max_id), node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (b,5,d)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(5,
                                       node::max_id - 1,
                                       node::pointer_type(6, node::max_id - 3),
                                       node::pointer_type(6, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (4,b,5,d)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(5,
                                       node::max_id - 2,
                                       node::pointer_type(6, node::max_id - 2),
                                       node::pointer_type(6, node::max_id - 1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (3,b,5,d)
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(
              4, node::max_id, node::pointer_type(5, node::max_id - 2), node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (2,b,5,d)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(4,
                                       node::max_id - 1,
                                       node::pointer_type(5, node::max_id - 1),
                                       node::pointer_type(5, node::max_id - 2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1,b,5,d)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(3,
                                       node::max_id,
                                       node::pointer_type(4, node::max_id - 1),
                                       node::pointer_type(4, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1,a,5,d)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(2,
                                       node::max_id,
                                       node::pointer_type(3, node::max_id),
                                       node::pointer_type(5, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(13u, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(12u, 4u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(11u, 7u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(10u, 7u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(9u, 7u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(8u, 6u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(7u, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(6u, 4u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5u, 3u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4u, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...

          // Check call history
          //
          // NOTE: Test failure does NOT indicate a bug, but only indicates a
          //       change. Please verify that this change makes sense and is as
          //       intended.
          AssertThat(call_history.size(), Is().EqualTo(47u));

          // - First check for at least one variable satisfying the predicate.
          AssertThat(call_history.at(0), Is().EqualTo(13u));
          AssertThat(call_history.at(1), Is().EqualTo(12u));
          AssertThat(call_history.at(2), Is().EqualTo(11u));
          AssertThat(call_history.at(3), Is().EqualTo(10u));
          AssertThat(call_history.at(4), Is().EqualTo(9u));
          AssertThat(call_history.at(5), Is().EqualTo(8u));
          AssertThat(call_history.at(6), Is().EqualTo(7u));
          AssertThat(call_history.at(7), Is().EqualTo(6u));
          AssertThat(call_history.at(8), Is().EqualTo(5u));
          AssertThat(call_history.at(9), Is().EqualTo(4u));
          AssertThat(call_history.at(10), Is().EqualTo(3u));
          AssertThat(call_history.at(11), Is().EqualTo(2u));
          AssertThat(call_history.at(12), Is().EqualTo(1u));

          // - Upper bound partial quantifications; capped at N/3(ish) nodes.
          AssertThat(call_history.at(13), Is().EqualTo(0u));
          AssertThat(call_history.at(14), Is().EqualTo(1u));
          AssertThat(call_history.at(15), Is().EqualTo(2u));
          AssertThat(call_history.at(16), Is().EqualTo(3u));
          AssertThat(call_history.at(17), Is().EqualTo(4u));
          AssertThat(call_history.at(18), Is().EqualTo(5u));
          AssertThat(call_history.at(19), Is().EqualTo(6u)); // 13 out of 34 nodes

          // - Top-down sweep
          AssertThat(call_history.at(20), Is().EqualTo(0u));
          AssertThat(call_history.at(21), Is().EqualTo(1u));
          AssertThat(call_history.at(22), Is().EqualTo(2u));
          AssertThat(call_history.at(23), Is().EqualTo(3u));
          AssertThat(call_history.at(24), Is().EqualTo(4u));
          AssertThat(call_history.at(25), Is().EqualTo(5u));
          AssertThat(call_history.at(26), Is().EqualTo(6u));
          AssertThat(call_history.at(27), Is().EqualTo(7u));
          AssertThat(call_history.at(28), Is().EqualTo(8u));
          AssertThat(call_history.at(29), Is().EqualTo(9u));
          AssertThat(call_history.at(30), Is().EqualTo(10u));
          AssertThat(call_history.at(31), Is().EqualTo(11u));
          AssertThat(call_history.at(32), Is().EqualTo(12u));
          AssertThat(call_history.at(33), Is().EqualTo(13u));

          // - Nested Sweeping (x0 is gone)
          AssertThat(call_history.at(34), Is().EqualTo(13u));
          AssertThat(call_history.at(35), Is().EqualTo(12u));
          AssertThat(call_history.at(36), Is().EqualTo(11u));
          AssertThat(call_history.at(37), Is().EqualTo(10u));
          AssertThat(call_history.at(38), Is().EqualTo(9u));
          AssertThat(call_history.at(39), Is().EqualTo(8u));
          AssertThat(call_history.at(40), Is().EqualTo(7u));
          AssertThat(call_history.at(41), Is().EqualTo(6u));
          AssertThat(call_history.at(42), Is().EqualTo(5u));
          AssertThat(call_history.at(43), Is().EqualTo(4u));
          AssertThat(call_history.at(44), Is().EqualTo(3u));
          AssertThat(call_history.at(45), Is().EqualTo(2u));
          AssertThat(call_history.at(46), Is().EqualTo(1u));
        });
      });
    });

    describe("zdd_project(const zdd&, const generator<zdd::label_type>&)", [&]() {
      it("returns same file for Ø with dom = {6,4,2,0} [const &]", [&]() {
        const zdd in = zdd_empty;
        zdd out      = zdd_project(in, [var = 6]() mutable -> optional<zdd::label_type> {
          if (var == 42) { return {}; }

          const zdd::label_type ret = var;
          var                       = ret == 1 ? 42 : var - 2;
          return { ret };
        });

        AssertThat(out.file_ptr(), Is().EqualTo(in.file_ptr()));
      });

      it("returns same file for { Ø } with dom = {5,3,1} [&&]", [&]() {
        zdd out = zdd_project(zdd(zdd_null), [var = 5]() mutable -> optional<zdd::label_type> {
          if (var == 42) { return {}; }

          const zdd::label_type ret = var;
          var                       = ret == 1 ? 42 : var - 2;
          return { ret };
        });

        AssertThat(out.file_ptr(), Is().EqualTo(zdd_null));
      });

      it("returns same file for Ø with dom = Ø [&&]", [&]() {
        zdd out = zdd_project(zdd(zdd_empty), []() { return make_optional<zdd::label_type>(); });

        AssertThat(out.file_ptr(), Is().EqualTo(zdd_empty));
      });

      it("returns same file for { Ø } with dom = Ø [const &]", [&]() {
        const zdd in = zdd_null;
        zdd out      = zdd_project(in, []() -> optional<zdd::label_type> { return {}; });

        AssertThat(out.file_ptr(), Is().EqualTo(in.file_ptr()));
      });

      describe("algorithm: Singleton", [&]() {
        const exec_policy ep = exec_policy::quantify::Singleton;

        it("collapses zdd_1 with dom = Ø into { Ø } [const &]", [&]() {
          const zdd in = zdd_1;
          zdd out      = zdd_project(in, []() -> optional<zdd::label_type> { return {}; });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("collapses zdd_2 with dom = Ø into { Ø } [&&]", [&]() {
          zdd out = zdd_project(ep, zdd(zdd_2), []() -> optional<zdd::label_type> { return {}; });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("collapses zdd_3 with dom = Ø into { Ø } [const &]", [&]() {
          const zdd in = zdd_3;
          zdd out      = zdd_project(ep, in, []() -> optional<zdd::label_type> { return {}; });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("collapses zdd_2 with disjoint domain into { Ø } [const &]", [&]() {
          const zdd in = zdd_2;

          zdd out = zdd_project(ep, in, [var = 1]() mutable -> optional<zdd::label_type> {
            if (var == 0) { return {}; }

            const zdd::label_type ret = var;
            var--;
            return { ret };
          });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("computes with disjoint dom to be { Ø } [zdd_3] [&&]", [&]() {
          zdd::label_type var = 5;
          zdd out             = zdd_project(ep, zdd(zdd_3), [&var]() {
            return 3 <= var && var <= 5 ? make_optional<zdd::label_type>(var--)
                                                    : make_optional<zdd::label_type>();
          });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        //////////////////////
        // Single-node case
        shared_levelized_file<zdd::node_type> zdd_x1;
        {
          node_writer nw(zdd_x1);
          nw << node(1u, node::max_id, ptr_uint64(false), ptr_uint64(true));
        }

        it("returns { Ø } for {1} with dom = {0} [const &]", [&]() {
          shared_file<zdd::label_type> dom;
          {
            label_writer lw(dom);
            lw << 0;
          }

          const zdd in = zdd_x1;

          /* Expected: { Ø }
           *
           *        T
           */

          zdd out = zdd_project(ep, in, [var = 0]() mutable {
            return var == 0 ? make_optional<zdd::label_type>(var--)
                            : make_optional<zdd::label_type>();
          });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("returns { 1 } for {1} with dom = {1,0} [const &]", [&]() {
          const zdd in = zdd_x1;

          /* Expected: { 1 }
           *
           *        1       ---- x1
           *       / \
           *       F T
           */
          zdd out = zdd_project(ep, in, [var = 1]() mutable {
            return 0 <= var ? make_optional<zdd::label_type>(var--)
                            : make_optional<zdd::label_type>();
          });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, node::max_id, zdd::pointer_type(false), zdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        //////////////////////
        // Non-terminal general case
        it("computes zdd_1 with dom = {4,3,2} [const &]", [&]() {
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
          zdd out = zdd_project(ep, in, [var = 4]() mutable {
            return 2 <= var && var <= 4 ? make_optional<zdd::label_type>(var--)
                                        : make_optional<zdd::label_type>();
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id, terminal_T, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(3, node::max_id, terminal_T, ptr_uint64(4, ptr_uint64::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(2, node::max_id, ptr_uint64(3, ptr_uint64::max_id), terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(4u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(4u));
        });

        it("computes zdd_2 with dom = {4,3,2} [&&]", [&]() {
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
          zdd out = zdd_project(ep, zdd_2, [var = 4]() mutable {
            return 2 <= var && var <= 4 ? make_optional<zdd::label_type>(var--)
                                        : make_optional<zdd::label_type>();
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id, terminal_T, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id, terminal_T, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(2,
                                       node::max_id,
                                       ptr_uint64(3, ptr_uint64::max_id),
                                       ptr_uint64(4, ptr_uint64::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(4u));
        });

        it("computes zdd_3 with dom = {4,2,0} [&&]", [&]() {
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
          zdd out = zdd_project(ep, zdd_3, [var = 4]() mutable -> optional<zdd::label_type> {
            if (var == 42) { return {}; }

            const zdd::label_type ret = var;
            var                       = ret == 0 ? 42 : var - 2;
            return { ret };
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_T, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0,
                                       node::max_id,
                                       ptr_uint64(2, ptr_uint64::max_id),
                                       ptr_uint64(2, ptr_uint64::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
        });

        it("computes zdd_4 with dom = {4,0} [&&]", [&]() {
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
          zdd out = zdd_project(ep, zdd_4, [var = 4]() mutable -> optional<zdd::label_type> {
            if (var == 42) { return {}; }

            const zdd::label_type ret = var;
            var                       = ret == 0 ? 42 : var - 4;
            return { ret };
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id, terminal_T, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(4, node::max_id - 1, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0,
                                       node::max_id,
                                       ptr_uint64(4, ptr_uint64::max_id - 1),
                                       ptr_uint64(4, ptr_uint64::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(4, 2u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(3u));
        });

        it("computes zdd_4 with dom = {4,2} [&&]", [&]() {
          /* Expected: { {2}, {4}, {2,4} }
          //
          //       1     ---- x2
          //      / \
          //      2 3    ---- x4
          //     / \||
          //     F  T
          */
          zdd out = zdd_project(ep, zdd_4, [var = 4]() mutable -> optional<zdd::label_type> {
            if (var == 42) { return make_optional<zdd::label_type>(); }

            const zdd::label_type res = var;
            if (var == 4) {
              var -= 2;
            } else {
              var = 42;
            }
            return res;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id, terminal_T, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(4, node::max_id - 1, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(2,
                                       node::max_id,
                                       ptr_uint64(4, ptr_uint64::max_id - 1),
                                       ptr_uint64(4, ptr_uint64::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(4, 2u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(3u));
        });
      });

      describe("algorithm: Nested, max: _", [&]() {
        const exec_policy ep = exec_policy::quantify::Nested;

        it("collapses zdd_1 with dom = Ø into { Ø } [const &]", [&]() {
          const zdd in = zdd_1;
          zdd out      = zdd_project(ep, in, []() -> optional<zdd::label_type> { return {}; });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("collapses zdd_2 with dom = Ø into { Ø } [&&]", [&]() {
          zdd out = zdd_project(ep, zdd(zdd_2), []() -> optional<zdd::label_type> { return {}; });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("collapses zdd_3 with dom = Ø into { Ø } [const &]", [&]() {
          const zdd in = zdd_3;
          zdd out      = zdd_project(ep, in, []() -> optional<zdd::label_type> { return {}; });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("collapses zdd_2 with disjoint domain into { Ø } [const &]", [&]() {
          const zdd in = zdd_2;
          zdd out      = zdd_project(ep, in, [var = 1]() mutable -> optional<zdd::label_type> {
            if (var != 1) { return {}; }
            return { var-- };
          });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("computes with disjoint dom to be { Ø } [zdd_3] [&&]", [&]() {
          zdd out = zdd_project(ep, zdd(zdd_3), [var = 5]() mutable {
            return 3 <= var && var <= 5 ? make_optional<zdd::label_type>(var--)
                                        : make_optional<zdd::label_type>();
          });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        // TODO: Shortcut on nothing to do

        //////////////////////
        // Single-node case
        shared_levelized_file<zdd::node_type> zdd_x1;
        {
          node_writer nw(zdd_x1);
          nw << node(1u, node::max_id, ptr_uint64(false), ptr_uint64(true));
        }

        it("returns { Ø } for {1} with dom = {0} [const &]", [&]() {
          shared_file<zdd::label_type> dom;
          {
            label_writer lw(dom);
            lw << 0;
          }

          const zdd in = zdd_x1;

          /* Expected: { Ø }
           *
           *        T
           */
          zdd out = zdd_project(ep, in, [var = 0]() mutable {
            return var == 0 ? make_optional<zdd::label_type>(var--)
                            : make_optional<zdd::label_type>();
          });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("returns { 1 } for {1} with dom = {1,0} [const &]", [&]() {
          const zdd in = zdd_x1;

          /* Expected: { 1 }
           *
           *        1       ---- x1
           *       / \
           *       F T
           */
          zdd out = zdd_project(ep, in, [var = 1]() mutable {
            return 0 <= var ? make_optional<zdd::label_type>(var--)
                            : make_optional<zdd::label_type>();
          });

          node_test_stream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, node::max_id, zdd::pointer_type(false), zdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);
          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        //////////////////////
        // Non-terminal general case
        it("computes zdd_1 with dom = {4,3,2} [const &]", [&]() {
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
          zdd out = zdd_project(ep, in, [var = 4]() mutable {
            return 2 <= var && var <= 4 ? make_optional<zdd::label_type>(var--)
                                        : make_optional<zdd::label_type>();
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id, terminal_T, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(3, node::max_id, terminal_T, ptr_uint64(4, ptr_uint64::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(2, node::max_id, ptr_uint64(3, ptr_uint64::max_id), terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(4u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(4u));
        });

        it("computes zdd_2 with dom = {4,3,2} [&&]", [&]() {
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
          zdd out = zdd_project(ep, zdd_2, [var = 4]() mutable {
            return 2 <= var && var <= 4 ? make_optional<zdd::label_type>(var--)
                                        : make_optional<zdd::label_type>();
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id, terminal_T, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id, terminal_T, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(2,
                                       node::max_id,
                                       ptr_uint64(3, ptr_uint64::max_id),
                                       ptr_uint64(4, ptr_uint64::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(4u));
        });

        it("computes zdd_3 with dom = {4,2,0} [&&]", [&]() {
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
          zdd out = zdd_project(ep, zdd_3, [var = 4]() mutable -> optional<zdd::label_type> {
            if (var == 42) { return make_optional<zdd::label_type>(); }

            const zdd::label_type ret = var;
            var                       = ret == 0 ? 42 : var - 2;
            return ret;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_T, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0,
                                       node::max_id,
                                       ptr_uint64(2, ptr_uint64::max_id),
                                       ptr_uint64(2, ptr_uint64::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
        });

        it("computes zdd_4 with dom = {4,0} [&&]", [&]() {
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
          zdd out = zdd_project(ep, zdd_4, [var = 4]() mutable -> optional<zdd::label_type> {
            if (var == 42) { return {}; }

            const zdd::label_type ret = var;
            var                       = ret == 0 ? 42 : var - 4;
            return { ret };
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id, terminal_T, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(4, node::max_id - 1, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0,
                                       node::max_id,
                                       ptr_uint64(4, ptr_uint64::max_id - 1),
                                       ptr_uint64(4, ptr_uint64::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(4, 2u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(3u));
        });

        it("computes zdd_4 with dom = {4,2} [&&]", [&]() {
          /* Expected: { {2}, {4}, {2,4} }
          //
          //       1     ---- x2
          //      / \
          //      2 3    ---- x4
          //     / \||
          //     F  T
          */
          zdd out = zdd_project(ep, zdd_4, [var = 4]() mutable -> optional<zdd::label_type> {
            if (var == 42) { return make_optional<zdd::label_type>(); }

            const zdd::label_type res = var;
            if (var == 4) {
              var -= 2;
            } else {
              var = 42;
            }
            return res;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id, terminal_T, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(4, node::max_id - 1, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(2,
                                       node::max_id,
                                       ptr_uint64(4, ptr_uint64::max_id - 1),
                                       ptr_uint64(4, ptr_uint64::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(4, 2u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(3u));
        });

        it("bails out of inner sweep for zdd_4 with dom = { x | x % 2 == 0 } [const &]", [&]() {
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
          zdd out = zdd_project(ep, in, [var = 8]() mutable -> optional<zdd::label_type> {
            if (var == 42) { return {}; }

            const zdd::label_type res = var;
            var                       = var == 0 ? 42 : var - 2;
            return { res };
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id, terminal_T, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(2, node::max_id, ptr_uint64(4, ptr_uint64::max_id), terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, node::max_id, ptr_uint64(2, ptr_uint64::max_id), terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(4u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(4u));
        });

        it("accounts for number of root arcs from Outer Sweep [const &]", [&]() {
          const zdd in = zdd_6;

          /* Expected: { Ø, {5} }
          //
          //         1       ---- x5
          //        / \
          //        T T
          */
          zdd out = zdd_project(ep, in, [](const zdd::label_type x) { return x == 5; });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id, terminal_T, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(5, 1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
        });
      });
    });

    describe("zdd_project(const zdd&, ForwardIt, ForwardIt)", [&]() {
      // Since this is merely a wrapper on the generator function, we will just
      // double-check with a few tests.

      describe("algorithm: Singleton", [&]() {
        const exec_policy ep = exec_policy::quantify::Singleton;

        it("computes zdd_2 with dom = {4,3,2} [&&]", [&]() {
          const std::vector<int> dom = { 4, 3, 2 };

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
          zdd out = zdd_project(ep, zdd_2, dom.cbegin(), dom.cend());

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id, terminal_T, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id, terminal_T, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(2,
                                       node::max_id,
                                       ptr_uint64(3, ptr_uint64::max_id),
                                       ptr_uint64(4, ptr_uint64::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(0u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(4u));
        });
      });

      describe("algorithm: Nested, max: _", [&]() {
        const exec_policy ep = exec_policy::quantify::Nested;

        it("computes zdd_3 with dom = {4,2,0} [&&]", [&]() {
          const std::vector<int> dom = { 4, 2, 0 };

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
          zdd out = zdd_project(ep, zdd_3, dom.cbegin(), dom.cend());

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_T, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0,
                                       node::max_id,
                                       ptr_uint64(2, ptr_uint64::max_id),
                                       ptr_uint64(2, ptr_uint64::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
        });

        it("computes zdd_4 with dom = {4,0} [const &]", [&]() {
          std::vector<int> dom = { 4, 0 };
          zdd in               = zdd_4;

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
          zdd out = zdd_project(ep, in, dom.cbegin(), dom.cend());

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id, terminal_T, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(4, node::max_id - 1, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0,
                                       node::max_id,
                                       ptr_uint64(4, ptr_uint64::max_id - 1),
                                       ptr_uint64(4, ptr_uint64::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream ms(out);

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(4, 2u)));

          AssertThat(ms.can_pull(), Is().True());
          AssertThat(ms.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(ms.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(3u));
        });
      });
    });
  });
});
