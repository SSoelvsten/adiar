#include "../../test.h"

#include <vector>

go_bandit([]() {
  describe("adiar/bdd/quantify.cpp", []() {
    ////////////////////////////////////////////////////////////////////////
    // Sink only BDDs
    shared_levelized_file<bdd::node_type> terminal_F;

    { // Garbage collect writer to free write-lock}
      node_writer nw_F(terminal_F);
      nw_F << node(false);
    }

    shared_levelized_file<bdd::node_type> terminal_T;

    { // Garbage collect writer to free write-lock
      node_writer nw_T(terminal_T);
      nw_T << node(true);
    }

    ////////////////////////////////////////////////////////////////////////
    // BDD 1
    /*
    //   1     ---- x0
    //  / \
    //  T 2    ---- x1
    //   / \
    //   F T
    */
    shared_levelized_file<bdd::node_type> bdd_1;

    const node n1_2 = node(1, node::max_id, ptr_uint64(false), ptr_uint64(true));
    const node n1_1 = node(0, node::max_id, ptr_uint64(true), n1_2.uid());

    { // Garbage collect writer to free write-lock
      node_writer nw_1(bdd_1);
      nw_1 << n1_2 << n1_1;
    }

    ////////////////////////////////////////////////////////////////////////
    // BDD 2
    /*
    //       1       ---- x0
    //      / \
    //     2   3     ---- x1
    //    / \ / \
    //   4   5  F    ---- x2
    //  / \ / \
    //  T F F T
    */
    shared_levelized_file<bdd::node_type> bdd_2;

    const node n2_5 = node(2, node::max_id, ptr_uint64(false), ptr_uint64(true));
    const node n2_4 = node(2, node::max_id-1, ptr_uint64(true), ptr_uint64(false));
    const node n2_3 = node(1, node::max_id, n2_5.uid(), ptr_uint64(false));
    const node n2_2 = node(1, node::max_id-1, n2_4.uid(), n2_5.uid());
    const node n2_1 = node(0, node::max_id, n2_2.uid(), n2_3.uid());

    { // Garbage collect writer to free write-lock
      node_writer nw_2(bdd_2);
      nw_2 << n2_5 << n2_4 << n2_3 <<n2_2 << n2_1;
    }

    ////////////////////////////////////////////////////////////////////////
    // BDD 3
    /*
    //       1       ---- x0
    //      / \
    //      |  2     ---- x1
    //      \ / \
    //       3   4   ---- x2
    //      / \ / \
    //      T F F T
    */
    shared_levelized_file<bdd::node_type> bdd_3;

    const node n3_4 = node(2, node::max_id, ptr_uint64(false), ptr_uint64(true));
    const node n3_3 = node(2, node::max_id-1, ptr_uint64(true), ptr_uint64(false));
    const node n3_2 = node(1, node::max_id, n3_3.uid(), n3_4.uid());
    const node n3_1 = node(0, node::max_id, n3_3.uid(), n3_2.uid());

    { // Garbage collect writer to free write-lock
      node_writer nw_3(bdd_3);
      nw_3 << n3_4 << n3_3 << n3_2 << n3_1;
    }

    ////////////////////////////////////////////////////////////////////////
    // BDD 4
    /*
    //       1       ---- x0
    //      / \
    //      |  2     ---- x1
    //      \ / \
    //       3   4   ---- x2
    //      / \ / \
    //      F  5  T  ---- x3
    //        / \
    //        F T
    */
    shared_levelized_file<bdd::node_type> bdd_4;

    const node n4_5 = node(3, node::max_id, ptr_uint64(false), ptr_uint64(true));
    const node n4_4 = node(2, node::max_id, n4_5.uid(), ptr_uint64(true));
    const node n4_3 = node(2, node::max_id-1, ptr_uint64(false), n4_5.uid());
    const node n4_2 = node(1, node::max_id, n4_3.uid(), n4_4.uid());
    const node n4_1 = node(0, node::max_id, n4_3.uid(), n4_2.uid());

    { // Garbage collect writer to free write-lock
      node_writer nw_4(bdd_4);
      nw_4 << n4_5 << n4_4 << n4_3 << n4_2 << n4_1;
    }

    ////////////////////////////////////////////////////////////////////////
    // BDD 5
    /*
    //    1      ---- x0
    //   / \
    //   F 2     ---- x1
    //    / \
    //   3   4   ---- x2
    //  / \ / \
    //  F T T F
    */
    shared_levelized_file<bdd::node_type> bdd_5;

    const node n5_4 = node(2, node::max_id, ptr_uint64(true), ptr_uint64(false));
    const node n5_3 = node(2, node::max_id-1, ptr_uint64(false), ptr_uint64(true));
    const node n5_2 = node(1, node::max_id, n5_3.uid(), n5_4.uid());
    const node n5_1 = node(0, node::max_id, ptr_uint64(false), n5_2.uid());

    { // Garbage collect writer to free write-lock
      node_writer nw_5(bdd_5);
      nw_5 << n5_4 << n5_3 << n5_2 << n5_1;
    }

    ////////////////////////////////////////////////////////////////////////////
    // x2 variable BDD
    shared_levelized_file<bdd::node_type> bdd_x2;

    { // Garbage collect writer to free write-lock
      node_writer nw_x2(bdd_x2);
      nw_x2 << node(2, node::max_id, ptr_uint64(false), ptr_uint64(true));
    }

    ////////////////////////////////////////////////////////////////////////////
    // BDD 6
    /*
    //         ___1__        ---- x0
    //        /      \
    //      _3__   __2__     ---- x1
    //     /    \ /     \
    //     4     6      5    ---- x2
    //    / \   / \    / \
    //    F  \ /  T   /  F
    //        8       7      ---- x3
    //       / \     / \
    //       F T     T F
    */
    shared_levelized_file<bdd::node_type> bdd_6;

    { // Garbage collect writer to free write-lock
      node_writer nw_6(bdd_6);
      nw_6 << node(3, node::max_id,   ptr_uint64(false),                   ptr_uint64(true))                    // 8
           << node(3, node::max_id-1, ptr_uint64(true),                    ptr_uint64(false))                   // 7
           << node(2, node::max_id,   ptr_uint64(3, ptr_uint64::max_id),   ptr_uint64(true))                    // 6
           << node(2, node::max_id-1, ptr_uint64(3, ptr_uint64::max_id-1), ptr_uint64(false))                   // 5
           << node(2, node::max_id-2, ptr_uint64(false),                   ptr_uint64(3, ptr_uint64::max_id))   // 4
           << node(1, node::max_id,   ptr_uint64(2, ptr_uint64::max_id-2), ptr_uint64(2, ptr_uint64::max_id))   // 3
           << node(1, node::max_id-1, ptr_uint64(2, ptr_uint64::max_id),   ptr_uint64(2, ptr_uint64::max_id-1)) // 2
           << node(0, node::max_id,   ptr_uint64(1, ptr_uint64::max_id),   ptr_uint64(1, ptr_uint64::max_id-1)) // 1
        ;
    }

    // BDD 6 with an 'x4' instead of T that can collapse to T during the
    // transposition sweep
    shared_levelized_file<bdd::node_type> bdd_6_x4T;

    { // Garbage collect writer to free write-lock
      node_writer nw_6(bdd_6_x4T);
      nw_6 << node(4, node::max_id,   ptr_uint64(false),                   ptr_uint64(true))                    // T
           << node(3, node::max_id,   ptr_uint64(false),                   ptr_uint64(4, ptr_uint64::max_id))   // 8
           << node(3, node::max_id-1, ptr_uint64(4, ptr_uint64::max_id),   ptr_uint64(false))                   // 7
           << node(2, node::max_id,   ptr_uint64(3, ptr_uint64::max_id),   ptr_uint64(4, ptr_uint64::max_id))   // 6
           << node(2, node::max_id-1, ptr_uint64(3, ptr_uint64::max_id-1), ptr_uint64(false))                   // 5
           << node(2, node::max_id-2, ptr_uint64(false),                   ptr_uint64(3, ptr_uint64::max_id))   // 4
           << node(1, node::max_id,   ptr_uint64(2, ptr_uint64::max_id-2), ptr_uint64(2, ptr_uint64::max_id))   // 3
           << node(1, node::max_id-1, ptr_uint64(2, ptr_uint64::max_id),   ptr_uint64(2, ptr_uint64::max_id-1)) // 2
           << node(0, node::max_id,   ptr_uint64(1, ptr_uint64::max_id),   ptr_uint64(1, ptr_uint64::max_id-1)) // 1
        ;
    }

    ////////////////////////////////////////////////////////////////////////////
    // BDD 7
    /*
    //          _1_        ---- x0
    //         /   \
    //         3   2       ---- x1
    //        / \ / \
    //        \_ | __\
    //           |    \
    //           4    5    ---- x2
    //          / \  / \
    //          T F  F T
    */
    shared_levelized_file<bdd::node_type> bdd_7;

    { // Garbage collect writer to free write-lock
      node_writer nw_7(bdd_7);
      nw_7 << node(2, node::max_id,   ptr_uint64(false),      ptr_uint64(true))       // 5
           << node(2, node::max_id-1, ptr_uint64(true),       ptr_uint64(false))      // 4
           << node(1, node::max_id,   ptr_uint64(2, ptr_uint64::max_id),   ptr_uint64(2, ptr_uint64::max_id-1)) // 3
           << node(1, node::max_id-1, ptr_uint64(2, ptr_uint64::max_id-1), ptr_uint64(2, ptr_uint64::max_id))   // 2
           << node(0, node::max_id,   ptr_uint64(1, ptr_uint64::max_id),   ptr_uint64(1, ptr_uint64::max_id-1)) // 1
        ;
    }

    ////////////////////////////////////////////////////////////////////////////
    // BDD 8 (b is mirrored on the nodes for x2)
    /*
    //           __1__         ---- x0
    //          /     \
    //         _2_     \       ---- x1
    //        /   \     \
    //        3    4    |      ---- x2
    //       / \  / \   |
    //       T  \ F  \  |
    //           \____\ |
    //                 \|
    //                  5      ---- x3
    //                 / \
    //                 F T
    */
    shared_levelized_file<bdd::node_type> bdd_8a, bdd_8b;

    { // Garbage collect writer to free write-lock
      node_writer nw_8a(bdd_8a);
      nw_8a << node(3, node::max_id,   ptr_uint64(false),      ptr_uint64(true))       // 5
            << node(2, node::max_id,   ptr_uint64(false),      ptr_uint64(3, ptr_uint64::max_id))   // 4
            << node(2, node::max_id-1, ptr_uint64(true),       ptr_uint64(3, ptr_uint64::max_id))   // 3
            << node(1, node::max_id,   ptr_uint64(2, ptr_uint64::max_id-1), ptr_uint64(2, ptr_uint64::max_id))   // 2
            << node(0, node::max_id,   ptr_uint64(1, ptr_uint64::max_id),   ptr_uint64(3, ptr_uint64::max_id))   // 1
        ;

      node_writer nw_8b(bdd_8b);
      nw_8b << node(3, node::max_id,   ptr_uint64(false),      ptr_uint64(true))       // 5
            << node(2, node::max_id,   ptr_uint64(3, ptr_uint64::max_id),   ptr_uint64(false))      // 4
            << node(2, node::max_id-1, ptr_uint64(3, ptr_uint64::max_id),   ptr_uint64(true))       // 3
            << node(1, node::max_id,   ptr_uint64(2, ptr_uint64::max_id-1), ptr_uint64(2, ptr_uint64::max_id))   // 2
            << node(0, node::max_id,   ptr_uint64(1, ptr_uint64::max_id),   ptr_uint64(3, ptr_uint64::max_id))   // 1
        ;
    }

    ////////////////////////////////////////////////////////////////////////////
    // BDD 9 (T terminal)
    /*
    //           _1_       ---- x1
    //          /   \
    //          2   3      ---- x2 (inner sweep)
    //         / \ / \
    //         4  5  |     ---- x3
    //        / \/ \ |
    //        6 F  T |     ---- x4 (skippable sweep)
    //       / \__ __/
    //       |    7        ---- x5
    //       |   / \
    //       8   F T       ---- x6 (transposition sweep)
    //      / \
    //      F T
    */
    const node n9T_8 = node(6, node::max_id,   ptr_uint64(false), ptr_uint64(true));
    const node n9T_7 = node(5, node::max_id,   ptr_uint64(false), ptr_uint64(true));
    const node n9T_6 = node(4, node::max_id,   n9T_8.uid(),       n9T_7.uid());
    const node n9T_5 = node(3, node::max_id,   ptr_uint64(false), ptr_uint64(true));
    const node n9T_4 = node(3, node::max_id-1, n9T_6.uid(),       ptr_uint64(false));
    const node n9T_3 = node(2, node::max_id,   n9T_5.uid(),       n9T_7.uid());
    const node n9T_2 = node(2, node::max_id-1, n9T_4.uid(),       n9T_5.uid());
    const node n9T_1 = node(1, node::max_id,   n9T_2.uid(),       n9T_3.uid());

    shared_levelized_file<bdd::node_type> bdd_9T;
    { // Garbage collect writer to free write-lock
      node_writer nw(bdd_9T);
      nw << n9T_8 << n9T_7 << n9T_6 << n9T_5 << n9T_4 << n9T_3 << n9T_2 << n9T_1;
    }

    ////////////////////////////////////////////////////////////////////////////
    // BDD 9 (F terminal)
    /* input
    //           _1_       ---- x1
    //          /   \
    //          2   3      ---- x2 (final sweep)
    //         / \ / \
    //         4  5  |     ---- x3
    //        / \/ \ |
    //        6 F  T |     ---- x4 (skippable sweep)
    //       / \__ __/
    //       F    7        ---- x5
    //           / \
    //           F 8       ---- x6 (transposition sweep)
    //            / \
    //            F T
    */
    const node n9F_8 = node(6, node::max_id,   ptr_uint64(false), ptr_uint64(true));
    const node n9F_7 = node(5, node::max_id,   ptr_uint64(false), n9F_8.uid());
    const node n9F_6 = node(4, node::max_id,   ptr_uint64(false), n9F_7.uid());
    const node n9F_5 = node(3, node::max_id,   ptr_uint64(false), ptr_uint64(true));
    const node n9F_4 = node(3, node::max_id-1, n9F_6.uid(),       ptr_uint64(false));
    const node n9F_3 = node(2, node::max_id,   n9F_5.uid(),       n9F_7.uid());
    const node n9F_2 = node(2, node::max_id-1, n9F_4.uid(),       n9F_5.uid());
    const node n9F_1 = node(1, node::max_id,   n9F_2.uid(),       n9F_3.uid());

    shared_levelized_file<bdd::node_type> bdd_9F;

    { // Garbage collect writer to free write-lock
      node_writer nw(bdd_9F);
      nw << n9F_8 << n9F_7 << n9F_6 << n9F_5 << n9F_4 << n9F_3 << n9F_2 << n9F_1;
    }

    ////////////////////////////////////////////////////////////////////////////
    // BDD 10 : node (8) dies when quantifying x2 and x3
    /*
    //          _1_           ---- x0
    //         /   \
    //         F   _2_        ---- x1
    //            /   \
    //            3   4       ---- x2
    //           / \ / \
    //           5  6  7      ---- x3
    //          / \| |/ \
    //          T   8   T     ---- x4
    //             / \
    //             F T
    */
    const node n10_8 = node(4, node::max_id,   ptr_uint64(false), ptr_uint64(true));
    const node n10_7 = node(3, node::max_id,   n10_8.uid(),       ptr_uint64(true));
    const node n10_6 = node(3, node::max_id-1, n10_8.uid(),       n10_8.uid());
    const node n10_5 = node(3, node::max_id-2, ptr_uint64(true),  n10_8.uid());
    const node n10_4 = node(2, node::max_id,   n10_6.uid(),       n10_7.uid());
    const node n10_3 = node(2, node::max_id-1, n10_5.uid(),       n10_6.uid());
    const node n10_2 = node(1, node::max_id,   n10_3.uid(),       n10_4.uid());
    const node n10_1 = node(0, node::max_id,   ptr_uint64(false), n10_2.uid());

    shared_levelized_file<bdd::node_type> bdd_10;

    { // Garbage collect writer to free write-lock
      node_writer nw(bdd_10);
      nw << n10_8 << n10_7 << n10_6 << n10_5 << n10_4 << n10_3 << n10_2 << n10_1;
    }

    ////////////////////////////////////////////////////////////////////////////
    // BDD 11 : Same as BDD 3, but with gaps in-between the levelsn
    /*
    //       1       ---- x2
    //      / \
    //      |  2     ---- x4
    //      \ / \
    //       3   4   ---- x6
    //      / \ / \
    //      T F F T
    */
    shared_levelized_file<bdd::node_type> bdd_11;

    node n11_4 = node(6, node::max_id,   ptr_uint64(false), ptr_uint64(true));
    node n11_3 = node(6, node::max_id-1, ptr_uint64(true),  ptr_uint64(false));
    node n11_2 = node(4, node::max_id,   n11_3.uid(),       n11_4.uid());
    node n11_1 = node(2, node::max_id,   n11_3.uid(),       n11_2.uid());

    { // Garbage collect writer to free write-lock
      node_writer nw_11(bdd_11);
      nw_11 << n11_4 << n11_3 << n11_2 << n11_1;
    }

    ////////////////////////////////////////////////////////////////////////////
    // BDD 12a : primary partial quantification example from paper
    /*
    //          1          ---- x0
    //         / \
    //         | 2         ---- x1
    //         |/ \
    //         3   4       ---- x2
    //        / \ / \
    //        5  6   \     ---- x3
    //       / \/ \  |
    //       T  F T  7     ---- x4
    //              / \
    //              F T
    */
    const node n12a_7 = node(4, node::max_id,   ptr_uint64(false), ptr_uint64(true));
    const node n12a_6 = node(3, node::max_id,   ptr_uint64(false), ptr_uint64(true));
    const node n12a_5 = node(3, node::max_id-1, ptr_uint64(true),  ptr_uint64(false));
    const node n12a_4 = node(2, node::max_id,   n12a_6.uid(),      n12a_7.uid());
    const node n12a_3 = node(2, node::max_id-1, n12a_5.uid(),      n12a_6.uid());
    const node n12a_2 = node(1, node::max_id,   n12a_3.uid(),      n12a_4.uid());
    const node n12a_1 = node(0, node::max_id,   n12a_3.uid(),      n12a_2.uid());

    shared_levelized_file<bdd::node_type> bdd_12a;

    { // Garbage collect writer to free write-lock
      node_writer nw(bdd_12a);
      nw << n12a_7 << n12a_6 << n12a_5 << n12a_4 << n12a_3 << n12a_2 << n12a_1;
    }

    ////////////////////////////////////////////////////////////////////////////
    // BDD 12b : extends the above such that it does not collapse to the true
    //           terminal for 0 < x < 3.
    /*
    //          1          ---- x0
    //         / \
    //         | 2         ---- x1
    //         |/ \
    //         3   4       ---- x2
    //        / \ / \
    //        5  6  |      ---- x3
    //       / \/ \ /
    //       7 F   8       ---- x4
    //      / \   / \
    //      T F   F T
    */
    const node n12b_8 = node(4, node::max_id,   ptr_uint64(false), ptr_uint64(true));
    const node n12b_7 = node(4, node::max_id-1, ptr_uint64(true),  ptr_uint64(false));
    const node n12b_6 = node(3, node::max_id,   ptr_uint64(false), n12b_8.uid());
    const node n12b_5 = node(3, node::max_id-1, n12b_7.uid(),      ptr_uint64(false));
    const node n12b_4 = node(2, node::max_id,   n12b_6.uid(),      n12b_8.uid());
    const node n12b_3 = node(2, node::max_id-1, n12b_5.uid(),      n12b_6.uid());
    const node n12b_2 = node(1, node::max_id,   n12b_3.uid(),      n12b_4.uid());
    const node n12b_1 = node(0, node::max_id,   n12b_3.uid(),      n12b_2.uid());

    shared_levelized_file<bdd::node_type> bdd_12b;

    { // Garbage collect writer to free write-lock
      node_writer nw(bdd_12b);
      nw << n12b_8 << n12b_7 << n12b_6 << n12b_5 << n12b_4 << n12b_3 << n12b_2 << n12b_1;
    }

    ////////////////////////////////////////////////////////////////////////////
    // BDD 13 : quantifying variables x0 and x1 create a BDD larger than the
    //          original input.
    /*
    //           _____1_____             ---- x0
    //          /           \
    //       __2__        __3__          ---- x1
    //      /     \      /     \
    //      |     |      |     |
    //      4     5      |     |         ---- x2
    //     / \   / \     |     |
    //    /   \  F |     6    _7_        ---- x3
    //    |   |    |    / \  /   \
    //    8   9    |    F | 10   11      ---- x4
    //    X  /|    |      |  X  /  \
    //   / \/ |    |      | / \/   /
    //   | /\ |    |      | | /\  /
    //   \ | \|    |      / 12  13       ---- x5
    //    \| ||    |     / / |  | \
    //    14 15    \    /  T F  F T      ---- x6
    //   / | | \    \  /
    //   T F F T     16                  ---- x7
    //              /  \
    //              F  T
    //
    // If x1 is quantified, the pairs (4,5) or (6,7) are created. Here, the F
    // terminal of 5 and 6 together with the node ? duplicates both the entire
    // subtree of 4 and 5. To be sure the output is large enough, we need to
    // make these subtrees at least as large as the number of nodes removed by
    // quantification.
    //
    // This also applies to partial quantification when x0 and x1 are
    // quantified, where the node (x1, (4,5), (6,7)) has to be created for
    // a later sweep.
    */

    // This is definitely NOT canonical... it is not worth making.
    const node n13_16 = node(7, node::max_id,   ptr_uint64(false), ptr_uint64(true));
    const node n13_15 = node(6, node::max_id,   ptr_uint64(false), ptr_uint64(true));
    const node n13_14 = node(6, node::max_id-1, ptr_uint64(true),  ptr_uint64(false));
    const node n13_13 = node(5, node::max_id,   ptr_uint64(false), ptr_uint64(true));
    const node n13_12 = node(5, node::max_id-1, ptr_uint64(true),  ptr_uint64(false));
    const node n13_11 = node(4, node::max_id,   n13_12.uid(),      n13_13.uid());
    const node n13_10 = node(4, node::max_id-1, n13_13.uid(),      n13_12.uid());
    const node n13_9  = node(4, node::max_id-2, n13_14.uid(),      n13_15.uid());
    const node n13_8  = node(4, node::max_id-3, n13_15.uid(),      n13_14.uid());
    const node n13_7  = node(3, node::max_id,   n13_10.uid(),      n13_11.uid());
    const node n13_6  = node(3, node::max_id-1, ptr_uint64(false), n13_16.uid());
    const node n13_5  = node(2, node::max_id,   ptr_uint64(false), n13_16.uid());
    const node n13_4  = node(2, node::max_id-1, n13_8.uid(),       n13_9.uid());
    const node n13_3  = node(1, node::max_id,   n13_6.uid(),       n13_7.uid());
    const node n13_2  = node(1, node::max_id-1, n13_4.uid(),       n13_5.uid());
    const node n13_1  = node(0, node::max_id,   n13_2.uid(),       n13_3.uid());

    shared_levelized_file<bdd::node_type> bdd_13;

    { // Garbage collect writer to free write-lock
      node_writer nw(bdd_13);
      nw << n13_16 << n13_15 << n13_14 << n13_13 << n13_12 << n13_11 << n13_10
         << n13_9 << n13_8 << n13_7 << n13_6 << n13_5 << n13_4 << n13_3 << n13_2
         << n13_1;
    }

    ////////////////////////////////////////////////////////////////////////////
    // BDD 14a : Partial Quantification does not immediately resolve
    //           quantification of x3 and x4.
    /*
    //                __1__         ---- x0
    //               /     \
    //              _2_   _3_       ---- x1
    //             /   \ /   \
    //             4   _5_   6      ---- x2  <-- widest level
    //            / \ /   \ / \
    //            F  7_   _8  F     ---- x3
    //              /  \ /  \
    //             /    9    \      ---- x4
    //            /    / \    \
    //           10   /   \   11    ---- x5
    //          /  \  |   |  /  \
    //          T  F 12   13 F  T   ---- x6
    //              /  \ /  \
    //              T  F F  T
    */
    const node n14_13 = node(8, node::max_id,   ptr_uint64(false), ptr_uint64(true));
    const node n14_12 = node(8, node::max_id-1, ptr_uint64(true),  ptr_uint64(false));
    const node n14_11 = node(7, node::max_id,   ptr_uint64(false), ptr_uint64(true));
    const node n14_10 = node(7, node::max_id-1, ptr_uint64(true),  ptr_uint64(false));
    const node n14_9  = node(6, node::max_id,   n14_12.uid(),      n14_13.uid());
    const node n14_8  = node(5, node::max_id,   n14_9.uid(),       n14_11.uid());
    const node n14_7  = node(5, node::max_id-1, n14_10.uid(),      n14_9.uid());
    const node n14_6  = node(4, node::max_id,   n14_8.uid(),       ptr_uint64(false));
    const node n14_5  = node(4, node::max_id-1, n14_7.uid(),       n14_8.uid());
    const node n14_4  = node(4, node::max_id-2, ptr_uint64(false), n14_7.uid());
    const node n14_3  = node(3, node::max_id,   n14_5.uid(),       n14_6.uid());
    const node n14_2  = node(3, node::max_id-1, n14_4.uid(),       n14_5.uid());
    const node n14_1  = node(2, node::max_id,   n14_2.uid(),       n14_3.uid());

    shared_levelized_file<bdd::node_type> bdd_14a;

    { // Garbage collect writer to free write-lock
      node_writer nw(bdd_14a);
      nw << n14_13 << n14_12 << n14_11 << n14_10 << n14_9 << n14_8 << n14_7
         << n14_6 << n14_5 << n14_4 << n14_3 << n14_2 << n14_1;
    }

    ////////////////////////////////////////////////////////////////////////////
    // BDD 14b : Similar to BDD 14a but the following root has been added.
    /*
    //                   __1*__        ---- x0
    //                  /      \
    //                  |      2*      ---- x1
    //                  |     / \
    //               BDD 14   F T
    */
    const node n14b_2  = node(1, node::max_id,   ptr_uint64(false), ptr_uint64(true));
    const node n14b_1  = node(0, node::max_id,   n14_1.uid(),       n14b_2.uid());

    shared_levelized_file<bdd::node_type> bdd_14b;

    { // Garbage collect writer to free write-lock
      node_writer nw(bdd_14b);
      nw << n14_13 << n14_12 << n14_11 << n14_10 << n14_9 << n14_8 << n14_7
         << n14_6 << n14_5 << n14_4 << n14_3 << n14_2 << n14_1
         << n14b_2 << n14b_1;
    }

    ////////////////////////////////////////////////////////////////////////////
    // BDD 15 : The two subtrees are designed such that their products is more
    //          than twice their original size. If each subtree itself also is
    //          duplicated, then it gets close(ish) to 200% the input size.
    //
    //          The variables x0 and x1 are designed such that Partial
    //          Quantification has to leave another node for later.
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
    //                 F _4_ F        __b c__           ---- x5
    //                  /   \        /  \ /  \
    //                  5   6        F   d   F          ---- x6
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
    //                 T F F T          n o             ---- x13
    //                                 /| |\
    //                                T F F T
    //
    // Number of nodes:
    //   - roots: 3
    //   - left : 7 + 2i
    //   - right: 5 + 2i
    //
    // Total: 15 + 4i
    */
    //
    /*
    //                            _______(1,a)_______
    //                           /                   \
    //              __________(1,b)__________      (1,c)   <-- (1,c) is symmetric to (1,b) (these merge with (4,b) and (4,c))
    //             /                         \       .         Size: 3 additional nodes (incl. (1,c))
    //           (2,b)                     (3,c)     .
    //           /   \                      |  \     .
    //          (b) (4,b)                (4,c) (c)
    //              /   \                /   \
    //             (5) (6,d)           (5,d) (6)            <-- At this point we have copies of both inputs.
    //                 /   \           /   \                    Left subtree:  #nodes - 4
    //              (7,f) (9,g)    (8,d) (7,g)                  Right subtree: #nodes - 1
    //              /   \                /   \
    //          (8,f)   (9,f)          (8,g) (9,g)          <-- (8,g) and (9,g) also produce the same pairs
    //         /   |     |   \          / /   / /
    //        /    |     |    \        . .   . .
    //       /     |     |     \      . .   . .
    //   (10,h) (11,i) (11,h) (10,i)
    //    /  \   /  \   /  \   /  \
    //    .  .   .  .   .  .   .  .
    //   (12,j) (13,k) (12,j) (12,k)
    //    /  \   /  \   /  \   /  \
    //    .  .   .  .   .  .   .  .
    //   (14,l) (14,m) (15,l) (15,m)
    //    /  \   /  \   /  \   /  \
    //    .  .   .  .   .  .   .  .
    //   (16,n) (16,o) (17,n) (17,o)
    //    /  \   /  \   /  \   /  \
    //    T (n)  T (o)  T (n)  F (o)
    //
    // Number of nodes (after one partial quantification):
    //   - roots   : 1
    //   - left    : (7-4) + 2i
    //   - right   : (5-1) + 2i
    //   - product : 13 + 4i + 2 = 15 + 4i
    //   - (5,d)   : 1
    //
    // Total: 22 + 4i + 4(i-1)
    //
    // For i = 5, this should construct roughly 58 unreduced nodes in the first
    // sweep, which is 1.65 of the original 35 nodes.
    */
    // NOTE: This is not going to be canonical (i.e. ordered) because that would
    //       become too messy.

    const node n15_o  = node(13, node::max_id,   ptr_uint64(false), ptr_uint64(true));
    const node n15_n  = node(13, node::max_id-1, ptr_uint64(true),  ptr_uint64(false));

    const node n15_17 = node(12, node::max_id,   ptr_uint64(false), ptr_uint64(true));
    const node n15_16 = node(12, node::max_id-1, ptr_uint64(true),  ptr_uint64(false));

    const node n15_m  = node(11, node::max_id,   n15_o.uid(),       n15_n.uid());
    const node n15_l  = node(11, node::max_id-1, n15_n.uid(),       n15_o.uid());
    const node n15_15 = node(11, node::max_id-2, n15_17.uid(),      n15_16.uid());
    const node n15_14 = node(11, node::max_id-3, n15_16.uid(),      n15_17.uid());

    const node n15_k  = node(10, node::max_id,   n15_m.uid(),       n15_l.uid());
    const node n15_j  = node(10, node::max_id-1, n15_l.uid(),       n15_m.uid());
    const node n15_13 = node(10, node::max_id-2, n15_15.uid(),      n15_14.uid());
    const node n15_12 = node(10, node::max_id-3, n15_14.uid(),      n15_15.uid());

    const node n15_i  = node(9,  node::max_id,   n15_k.uid(),       n15_j.uid());
    const node n15_h  = node(9,  node::max_id-1, n15_j.uid(),       n15_k.uid());
    const node n15_11 = node(9,  node::max_id-2, n15_13.uid(),      n15_12.uid());
    const node n15_10 = node(9,  node::max_id-3, n15_12.uid(),      n15_13.uid());

    const node n15_g  = node(8,  node::max_id,   n15_i.uid(),       n15_h.uid());
    const node n15_f  = node(8,  node::max_id-1, n15_h.uid(),       n15_i.uid());
    const node n15_9  = node(8,  node::max_id-2, n15_11.uid(),      n15_10.uid());
    const node n15_8  = node(8,  node::max_id-3, n15_10.uid(),      n15_11.uid());

    const node n15_7  = node(7,  node::max_id,   n15_8.uid(),       n15_9.uid());

    const node n15_d  = node(6,  node::max_id,   n15_f.uid(),       n15_g.uid());
    const node n15_6  = node(6,  node::max_id-1, n15_7.uid(),       n15_9.uid());
    const node n15_5  = node(6,  node::max_id-2, n15_8.uid(),       n15_7.uid());

    const node n15_c  = node(5,  node::max_id,   n15_d.uid(),       ptr_uint64(false));
    const node n15_b  = node(5,  node::max_id-1, ptr_uint64(false), n15_d.uid());
    const node n15_4  = node(5,  node::max_id-2, n15_5.uid(),       n15_6.uid());

    const node n15_3  = node(4,  node::max_id,   n15_4.uid(),       ptr_uint64(false));
    const node n15_2  = node(4,  node::max_id-1, ptr_uint64(false), n15_4.uid());

    const node n15_1  = node(3,  node::max_id,   n15_2.uid(),       n15_3.uid());

    const node n15_a  = node(2,  node::max_id,   n15_b.uid(),       n15_c.uid());

    const node n15_r3 = node(1,  node::max_id,   n15_5.uid(),       n15_d.uid());
    const node n15_r2 = node(1,  node::max_id-1, n15_1.uid(),       n15_a.uid());

    const node n15_r1 = node(0,  node::max_id,   n15_r2.uid(),      n15_r3.uid());

    shared_levelized_file<bdd::node_type> bdd_15;

    { // Garbage collect writer to free write-lock
      node_writer nw(bdd_15);
      nw << n15_o  << n15_n
         << n15_17 << n15_16
         << n15_m  << n15_l  << n15_15 << n15_14
         << n15_k  << n15_j  << n15_13 << n15_12
         << n15_i  << n15_h  << n15_11 << n15_10
         << n15_g  << n15_f  << n15_9  << n15_8
         << n15_7
         << n15_d  << n15_6  << n15_5
         << n15_c  << n15_b  << n15_4
         << n15_3  << n15_2
         << n15_1
         << n15_a
         << n15_r3 << n15_r2
         << n15_r1
        ;
    }

    ////////////////////////////////////////////////////////////////////////////
    describe("bdd_exists(const bdd&, bdd::label_type)", [&]() {
      it("should quantify T terminal-only BDD as itself", [&]() {
        __bdd out = bdd_exists(terminal_T, 42);

        AssertThat(out.get<shared_levelized_file<bdd::node_type>>(), Is().EqualTo(terminal_T));
        AssertThat(out.negate, Is().False());
      });

      it("should quantify F terminal-only BDD as itself", [&]() {
        __bdd out = bdd_exists(terminal_F, 21);

        AssertThat(out.get<shared_levelized_file<bdd::node_type>>(), Is().EqualTo(terminal_F));
        AssertThat(out.negate, Is().False());
      });

      it("should shortcut quantification of root into T terminal [BDD 1]", [&]() {
        __bdd out = bdd_exists(bdd_1, 0);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->levels(), Is().EqualTo(0u));

        AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->max_1level_cut[cut_type::Internal], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->max_1level_cut[cut_type::Internal_False], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->max_1level_cut[cut_type::Internal_True], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->max_1level_cut[cut_type::All], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("should shortcut quantification of root into T terminal [x2]", [&]() {
        __bdd out = bdd_exists(bdd_x2, 2);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
        AssertThat(out_nodes.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->levels(), Is().EqualTo(0u));

        AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->max_1level_cut[cut_type::Internal], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->max_1level_cut[cut_type::Internal_False], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->max_1level_cut[cut_type::Internal_True], Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->max_1level_cut[cut_type::All], Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<bdd::node_type>>()->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("should shortcut quantification on non-existent label in input [BDD 1]", [&]() {
        __bdd out = bdd_exists(bdd_1, 42);

        AssertThat(out.get<shared_levelized_file<bdd::node_type>>(), Is().EqualTo(bdd_1));
        AssertThat(out.negate, Is().False());
      });

      it("should quantify bottom-most nodes [BDD 1]", [&]() {
        __bdd out = bdd_exists(bdd_1, 1);

        arc_test_stream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0u,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],  Is().EqualTo(2u));
      });

      it("should quantify root without terminal arcs [BDD 2]", [&]() {
        __bdd out = bdd_exists(bdd_2, 0);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(1,0), true,  ptr_uint64(2,1) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // (4,5)
                   Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(2,0), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // (5,_)
                   Is().EqualTo(arc { ptr_uint64(2,1), false, ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(2,1), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2u,2u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],  Is().EqualTo(3u));
      });

      it("should quantify root with F terminal [BDD 5]", [&]() {
        __bdd out = bdd_exists(bdd_5, 0);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(1,0), true,  ptr_uint64(2,1) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // true due to 4.low()
                   Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // true due to 5.high()
                   Is().EqualTo(arc { ptr_uint64(2,0), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // true due to 5.high()
                   Is().EqualTo(arc { ptr_uint64(2,1), false, ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // false due to its own leaf
                   Is().EqualTo(arc { ptr_uint64(2,1), true,  ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2u,2u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],  Is().EqualTo(2u));

        // TODO: meta variables...
      });

      it("should quantify nodes with terminal or nodes as children [BDD 2]", [&]() {
        __bdd out = bdd_exists(bdd_2, 1);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), true,   ptr_uint64(2,1) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // (4,5)
                   Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(2,0), true,   ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // (5,_)
                   Is().EqualTo(arc { ptr_uint64(2,1), false, ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(2,1), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2u,2u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],  Is().EqualTo(3u));
      });

      it("should output terminal arcs in order, despite the order of resolvement [BDD 2]", [&]() {
        __bdd out = bdd_exists(bdd_2, 2);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), true,   ptr_uint64(1,1) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // true due to 4.low()
                   Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // true due to 5.high()
                   Is().EqualTo(arc { ptr_uint64(1,0), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // true due to 5.high()
                   Is().EqualTo(arc { ptr_uint64(1,1), false, ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // false due to its own leaf
                   Is().EqualTo(arc { ptr_uint64(1,1), true,  ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1u,2u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],  Is().EqualTo(3u));
      });

      it("should keep nodes as is when skipping quantified level [BDD 3]", [&]() {
        __bdd out = bdd_exists(bdd_3, 1);

        arc_test_stream arcs(out);

        // Note, that node (2,0) reflects (3,nil) since while n4 < nil we process this
        // request without forwarding n3 through the secondary priority queue
        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(2,1) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // n3
                   Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // n3
                   Is().EqualTo(arc { ptr_uint64(2,0), true,  ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // true due to 3.low()
                   Is().EqualTo(arc { ptr_uint64(2,1), false, ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // true due to 4.high()
                   Is().EqualTo(arc { ptr_uint64(2,1), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2u,2u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],  Is().EqualTo(3u));
      });

      it("should output terminal arcs in order, despite the order of resolvement [BDD 3]", [&]() {
        __bdd out = bdd_exists(bdd_3, 2);

        arc_test_stream arcs(out);

        // Note, that node (2,0) reflects (3,nil) while n4 < nil since we process this
        // request without forwarding n3 through the secondary priority queue
        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // true due to 3.low()
                   Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // true due to 3.low()
                   Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // true due to 4.high()
                   Is().EqualTo(arc { ptr_uint64(1,0), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1u,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],  Is().EqualTo(3u));
      });

      it("should resolve terminal-terminal requests in [BDD 5]", [&]() {
        __bdd out = bdd_exists(bdd_5, 1);

        arc_test_stream arcs(out);

        // Note, that node (2,0) reflects (3,nil) while n4 < nil since we process this
        // request without forwarding n3 through the secondary priority queue
        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // true due to 4.low()
                   Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // true due to 3.high()
                   Is().EqualTo(arc { ptr_uint64(2,0), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2u,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],  Is().EqualTo(2u));
      });

      it("can shortcut/prune irrelevant subtrees [OR-chain]", [&]() {
        shared_levelized_file<bdd::node_type> bdd_chain;

        node n4 = node(3, node::max_id, ptr_uint64(false), ptr_uint64(true));
        node n3 = node(2, node::max_id, n4.uid(), ptr_uint64(true));
        node n2 = node(1, node::max_id, n3.uid(), ptr_uint64(true));
        node n1 = node(0, node::max_id, n2.uid(), ptr_uint64(true));

        { // Garbage collect writer to free write-lock
          node_writer bdd_chain_w(bdd_chain);
          bdd_chain_w << n4 << n3 << n2 << n1;
        }

        __bdd out = bdd_exists(bdd_chain, 2);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // true due to quantification of x2
                   Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(1,0), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1u,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],  Is().EqualTo(3u));
      });

      it("can forward information across a level [BDD 6]", [&]() {
        __bdd out = bdd_exists(bdd_6, 1);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True()); // (4,6)
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (5,6)
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(2,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (7,8)
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(2,1), false, ptr_uint64(3,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (8,F)
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(3,1) }));

        AssertThat(arcs.can_pull_internal(), Is().False());
        AssertThat(arcs.can_pull_terminal(), Is().True()); // (4,6)
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(2,0), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (5,6)
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(2,1), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (7,8)
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(3,0), false, ptr_uint64(true) }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(3,0), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (8,F)
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(3,1), false, ptr_uint64(false) }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(3,1), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2u,2u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3u,2u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],  Is().EqualTo(5u));
      });

      it("can forward multiple arcs to the same node across a level [BDD 7]", [&]() {
        __bdd out = bdd_exists(bdd_7, 1);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True()); // (4,5)
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());
        AssertThat(arcs.can_pull_terminal(), Is().True()); // (4,5)
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(true) }));
        AssertThat(arcs.can_pull_terminal(), Is().True()); // (4,5)
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(2,0), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2u,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],  Is().EqualTo(2u));
      });

      it("should collapse tuple requests of the same node back into request on a single node [BDD 8a]", [&]() {
        __bdd out = bdd_exists(bdd_8a, 1);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True()); // (3,4)
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (5,_)
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(3,0) }));
        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(2,0), true,  ptr_uint64(3,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (3,4)
        AssertThat(arcs.pull_terminal(), // true due to 3.low()
                   Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (5,_)
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(3,0), false, ptr_uint64(false) }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(3,0), true,  ptr_uint64(true) }));


        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3u,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],  Is().EqualTo(2u));
      });

      it("should collapse tuple requests of the same node back into request on a single node [BDD 8b]", [&]() {
        __bdd out = bdd_exists(bdd_8b, 1);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True()); // (3,4)
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (5,_)
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(3,0) }));
        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(3,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (3,4)
        AssertThat(arcs.pull_terminal(), // true due to 3.low()
                   Is().EqualTo(arc { ptr_uint64(2,0), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (5,_)
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(3,0), false, ptr_uint64(false) }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(3,0), true,  ptr_uint64(true) }));


        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3u,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],  Is().EqualTo(2u));
      });
    });

    describe("bdd_exists(const bdd&, const predicate<bdd::label_type>&)", [&]() {
      it("returns input on always-false predicate BDD 1 [&&]", [&]() {
        __bdd out = bdd_exists(bdd_1, [](const bdd::label_type) -> bool {
          return false;
        });
        AssertThat(out.get<shared_levelized_file<bdd::node_type>>(), Is().EqualTo(bdd_1));
      });

      describe("quantify_mode == Singleton", [&]() {
        quantify_mode = quantify_mode_t::Singleton;

        it("quantifies odd variables in BDD 4 [&&]", [&]() {
          bdd out = bdd_exists(bdd_4, [](const bdd::label_type x) -> bool {
            return x % 2;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (3)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::max_id,
                                                         ptr_uint64(2, ptr_uint64::max_id),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies 1, 2 in BDD 4 [&&]", [&]() {
          bdd out = bdd_exists(bdd_4, [](const bdd::label_type x) -> bool {
            return x == 1 || x == 2;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::max_id,
                                                         ptr_uint64(3, ptr_uint64::max_id),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies even variables in BDD 4 [const &]", [&]() {
          const bdd in = bdd_4;
          const bdd out = bdd_exists(in, [](const bdd::label_type x) -> bool {
            return !(x % 2);
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (5)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (2')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::max_id,
                                                         ptr_uint64(3, ptr_uint64::max_id),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies odd variables in BDD 1 [&&]", [&]() {
          bdd out = bdd_exists(bdd_1, [](const bdd::label_type x) -> bool {
            return x % 2;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);
          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("terminates early when quantifying to a terminal in BDD 1 [&&]", [&]() {
          // TODO: top-down dependant?
          int calls = 0;

          const bdd out = bdd_exists(bdd_1, [&calls](const bdd::label_type) -> bool {
            calls++;
            return true;
          });

          AssertThat(calls, Is().EqualTo(1));

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);
          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies with always-true predicate in BDD 4 [&&]", [&]() {
          bdd out = bdd_exists(bdd_4, [](const bdd::label_type) -> bool {
            return true;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);
          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        quantify_mode = quantify_mode_t::Auto;
      });

      describe("quantify_mode == Partial", [&]() {
        quantify_mode = quantify_mode_t::Partial;

        it("collapses during initial transposition of all variables in BDD 4 [&&]", [&]() {
          std::vector<bdd::label_type> call_history;
          bdd out = bdd_exists(bdd_4, [&call_history](const bdd::label_type x) -> bool {
            call_history.push_back(x);
            return true;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);
          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...

          // Check call history
          //
          // NOTE: Test failure does NOT indicate a bug, but only indicates a
          //       change. Please verify that this change makes sense and is as
          //       intended.
          AssertThat(call_history.size(), Is().EqualTo(5u));

          // - First check for at least one variable satisfying the predicate.
          AssertThat(call_history.at(0), Is().EqualTo(3u));


          // - First top-down sweep (root call)
          AssertThat(call_history.at(1), Is().EqualTo(0u));

          // - First top-down sweep
          AssertThat(call_history.at(2), Is().EqualTo(0u));
          AssertThat(call_history.at(3), Is().EqualTo(1u));
          AssertThat(call_history.at(4), Is().EqualTo(2u));
        });

        it("finishes during initial transposition of even variables in BDD 4 [const &]", [&]() {
          std::vector<bdd::label_type> call_history;

          const bdd in = bdd_4;
          const bdd out = bdd_exists(in, [&call_history](const bdd::label_type x) -> bool {
            call_history.push_back(x);
            return !(x % 2);
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (5)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (2')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::max_id,
                                                         ptr_uint64(3, ptr_uint64::max_id),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables ...

          // Check call history
          //
          // NOTE: Test failure does NOT indicate a bug, but only indicates a
          //       change. Please verify that this change makes sense and is as
          //       intended.
          AssertThat(call_history.size(), Is().EqualTo(7u));

          // - First check for at least one variable satisfying the predicate.
          AssertThat(call_history.at(0), Is().EqualTo(3u));
          AssertThat(call_history.at(1), Is().EqualTo(2u));

          // - First top-down sweep (root call)
          AssertThat(call_history.at(2), Is().EqualTo(0u));

          // - First top-down sweep
          AssertThat(call_history.at(3), Is().EqualTo(0u));
          AssertThat(call_history.at(4), Is().EqualTo(1u));
          AssertThat(call_history.at(5), Is().EqualTo(2u));
          AssertThat(call_history.at(6), Is().EqualTo(3u));
        });

        it("collapses during repeated transposition with variables 1 2 variables in BDD 11a [&&]", [&]() {
          std::vector<bdd::label_type> call_history;
          bdd out = bdd_exists(bdd_12a, [&call_history](const bdd::label_type x) -> bool {
            call_history.push_back(x);
            return 0 < x && x < 3;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);
          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...

          // Check call history
          //
          // NOTE: Test failure does NOT indicate a bug, but only indicates a
          //       change. Please verify that this change makes sense and is as
          //       intended.
          AssertThat(call_history.size(), Is().EqualTo(13u));

          // - First check for at least one variable satisfying the predicate.
          AssertThat(call_history.at(0), Is().EqualTo(4u));
          AssertThat(call_history.at(1), Is().EqualTo(3u));
          AssertThat(call_history.at(2), Is().EqualTo(2u));

          // - First top-down sweep (root call)
          AssertThat(call_history.at(3), Is().EqualTo(0u));

          // - First top-down sweep
          AssertThat(call_history.at(4),  Is().EqualTo(0u));
          AssertThat(call_history.at(5),  Is().EqualTo(1u));
          AssertThat(call_history.at(6), Is().EqualTo(2u));
          AssertThat(call_history.at(7), Is().EqualTo(3u));
          AssertThat(call_history.at(8), Is().EqualTo(4u));

          // - Second top-down sweep (root call)
          AssertThat(call_history.at(9),  Is().EqualTo(0u));

          // - Second top-down sweep
          AssertThat(call_history.at(10), Is().EqualTo(0u));
          AssertThat(call_history.at(11), Is().EqualTo(2u));
          AssertThat(call_history.at(12), Is().EqualTo(3u));
        });

        it("finishes during repeated transposition with variables 1 and 2 in BDD 11b [&&]", [&]() {
          std::vector<bdd::label_type> call_history;
          bdd out = bdd_exists(bdd_12b, [&call_history](const bdd::label_type x) -> bool {
            call_history.push_back(x);
            return 0 < x && x < 3;
          });

          /* expected
          //             1        ---- x0
          //            / \
          //           /   \      ---- x1
          //          /     \
          //          |     |     ---- x2
          //          |     |
          //          ?     ?     ---- x3
          //         / \   / \
          //         |  \  T |
          //         |   \__ /
          //         7      8     ---- x4
          //        / \    / \
          //        T F    F T
          //
          // The 'T' terminal at 'x3' is due to the pair (7,8) collapsing to the
          // 'T' terminal. This tuple is created from (7,F,8) which in turn is
          // created from (5,6,8) from the quantification (3,4) from (2).
          */
          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (8)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id,
                                                         node::pointer_type(false),
                                                         node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (7)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id-1,
                                                         node::pointer_type(true),
                                                         node::pointer_type(false))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5,6,8)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id,
                                                         node::pointer_type(true),
                                                         node::pointer_type(4, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5,6)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id-1,
                                                         node::pointer_type(4, node::max_id-1),
                                                         node::pointer_type(4, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::max_id,
                                                         node::pointer_type(3, node::max_id-1),
                                                         node::pointer_type(3, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4u,2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...

          // Check call history
          //
          // NOTE: Test failure does NOT indicate a bug, but only indicates a
          //       change. Please verify that this change makes sense and is as
          //       intended.
          AssertThat(call_history.size(), Is().EqualTo(14u));

          // - First check for at least one variable satisfying the predicate.
          AssertThat(call_history.at(0), Is().EqualTo(4u));
          AssertThat(call_history.at(1), Is().EqualTo(3u));
          AssertThat(call_history.at(2), Is().EqualTo(2u));

          // - First top-down sweep (root call)
          AssertThat(call_history.at(3), Is().EqualTo(0u));

          // - First top-down sweep
          AssertThat(call_history.at(4), Is().EqualTo(0u));
          AssertThat(call_history.at(5), Is().EqualTo(1u));
          AssertThat(call_history.at(6), Is().EqualTo(2u));
          AssertThat(call_history.at(7), Is().EqualTo(3u));
          AssertThat(call_history.at(8), Is().EqualTo(4u));

          // - Second top-down sweep (root call)
          AssertThat(call_history.at(9),  Is().EqualTo(0u));

          // - Second top-down sweep
          AssertThat(call_history.at(10), Is().EqualTo(0u));
          AssertThat(call_history.at(11), Is().EqualTo(2u));
          AssertThat(call_history.at(12), Is().EqualTo(3u));
          AssertThat(call_history.at(13), Is().EqualTo(4u));
        });

        it("finishes during repeated transposition with variables 1 and 2 in BDD 12 [&&]", [&]() {
          std::vector<bdd::label_type> call_history;
          bdd out = bdd_exists(bdd_13, [&call_history](const bdd::label_type x) -> bool {
            call_history.push_back(x);
            return x < 2;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (16)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(7, node::max_id,
                                                         node::pointer_type(false),
                                                         node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (15)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(6, node::max_id,
                                                         node::pointer_type(false),
                                                         node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (15,16)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(6, node::max_id-1,
                                                         node::pointer_type(7, node::max_id),
                                                         node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (14)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(6, node::max_id-2,
                                                         node::pointer_type(true),
                                                         node::pointer_type(false))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (14,16)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(6, node::max_id-3,
                                                         node::pointer_type(true),
                                                         node::pointer_type(7, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (13,15)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id,
                                                         node::pointer_type(6, node::max_id),
                                                         node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (13,15,16)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id-1,
                                                         node::pointer_type(6, node::max_id-1),
                                                         node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (13,14,16)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id-2,
                                                         node::pointer_type(6, node::max_id-3),
                                                         node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (12,15,16)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id-3,
                                                         node::pointer_type(true),
                                                         node::pointer_type(6, node::max_id-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (12,14)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id-4,
                                                         node::pointer_type(true),
                                                         node::pointer_type(6, node::max_id-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (12,14,16)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id-5,
                                                         node::pointer_type(true),
                                                         node::pointer_type(6, node::max_id-3))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (9,11,16)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id,
                                                         node::pointer_type(5, node::max_id-5),
                                                         node::pointer_type(5, node::max_id-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (8,11,16)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id-1,
                                                         node::pointer_type(5, node::max_id-3),
                                                         node::pointer_type(5, node::max_id-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (9,10,16)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id-2,
                                                         node::pointer_type(5, node::max_id-2),
                                                         node::pointer_type(5, node::max_id-3))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (8,10)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id-3,
                                                         node::pointer_type(5, node::max_id),
                                                         node::pointer_type(5, node::max_id-4))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (6,7,9,16)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id,
                                                         node::pointer_type(4, node::max_id-2),
                                                         node::pointer_type(4, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (6,7,8)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id-1,
                                                         node::pointer_type(4, node::max_id-3),
                                                         node::pointer_type(4, node::max_id-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (4,5,6,7)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id,
                                                         node::pointer_type(3, node::max_id-1),
                                                         node::pointer_type(3, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(7u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(6u,4u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5u,6u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4u,4u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...

          // Check call history
          //
          // NOTE: Test failure does NOT indicate a bug, but only indicates a
          //       change. Please verify that this change makes sense and is as
          //       intended.
          AssertThat(call_history.size(), Is().EqualTo(24u));

          // - First check for at least one variable satisfying the predicate.
          AssertThat(call_history.at(0), Is().EqualTo(7u));
          AssertThat(call_history.at(1), Is().EqualTo(6u));
          AssertThat(call_history.at(2), Is().EqualTo(5u));
          AssertThat(call_history.at(3), Is().EqualTo(4u));
          AssertThat(call_history.at(4), Is().EqualTo(3u));
          AssertThat(call_history.at(5), Is().EqualTo(2u));
          AssertThat(call_history.at(6), Is().EqualTo(1u));

          // - First top-down sweep (root call)
          AssertThat(call_history.at(7), Is().EqualTo(0u));

          // - First top-down sweep
          AssertThat(call_history.at(8),  Is().EqualTo(0u));
          AssertThat(call_history.at(9),  Is().EqualTo(1u));
          AssertThat(call_history.at(10), Is().EqualTo(2u));
          AssertThat(call_history.at(11), Is().EqualTo(3u));
          AssertThat(call_history.at(12), Is().EqualTo(4u));
          AssertThat(call_history.at(13), Is().EqualTo(5u));
          AssertThat(call_history.at(14), Is().EqualTo(6u));
          AssertThat(call_history.at(15), Is().EqualTo(7u));

          // - Second top-down sweep (root call)
          AssertThat(call_history.at(16), Is().EqualTo(1u));

          // - Second top-down sweep
          AssertThat(call_history.at(17), Is().EqualTo(1u));
          AssertThat(call_history.at(18), Is().EqualTo(2u));
          AssertThat(call_history.at(19), Is().EqualTo(3u));
          AssertThat(call_history.at(20), Is().EqualTo(4u));
          AssertThat(call_history.at(21), Is().EqualTo(5u));
          AssertThat(call_history.at(22), Is().EqualTo(6u));
          AssertThat(call_history.at(23), Is().EqualTo(7u));
        });

        it("quantifies exploding BDD 15", [&]() {
          std::vector<bdd::label_type> call_history;
          bdd out = bdd_exists(bdd_15, [&call_history](const bdd::label_type x) -> bool {
            call_history.push_back(x);
            return x < 2;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (o)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(13, node::max_id,
                                                         node::pointer_type(false),
                                                         node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (n)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(13, node::max_id-1,
                                                         node::pointer_type(true),
                                                         node::pointer_type(false))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (17,o)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(12, node::max_id,
                                                         node::pointer_type(13, node::max_id),
                                                         node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (17,n)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(12, node::max_id-1,
                                                         node::pointer_type(13, node::max_id-1),
                                                         node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (16,o)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(12, node::max_id-2,
                                                         node::pointer_type(true),
                                                         node::pointer_type(13, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (16,n)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(12, node::max_id-3,
                                                         node::pointer_type(true),
                                                         node::pointer_type(13, node::max_id-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (14,l)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(11, node::max_id,
                                                         node::pointer_type(12, node::max_id-3),
                                                         node::pointer_type(12, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (14,m)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(11, node::max_id-1,
                                                         node::pointer_type(12, node::max_id-2),
                                                         node::pointer_type(12, node::max_id-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (15,l)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(11, node::max_id-2,
                                                         node::pointer_type(12, node::max_id-1),
                                                         node::pointer_type(12, node::max_id-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (15,m)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(11, node::max_id-3,
                                                         node::pointer_type(12, node::max_id),
                                                         node::pointer_type(12, node::max_id-3))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (13,k)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(10, node::max_id,
                                                         node::pointer_type(11, node::max_id-3),
                                                         node::pointer_type(11, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (13,j)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(10, node::max_id-1,
                                                         node::pointer_type(11, node::max_id-2),
                                                         node::pointer_type(11, node::max_id-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (12,k)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(10, node::max_id-2,
                                                         node::pointer_type(11, node::max_id-1),
                                                         node::pointer_type(11, node::max_id-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (12,j)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(10, node::max_id-3,
                                                         node::pointer_type(11, node::max_id),
                                                         node::pointer_type(11, node::max_id-3))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (10,h)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(9, node::max_id,
                                                         node::pointer_type(10, node::max_id-3),
                                                         node::pointer_type(10, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (10,i)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(9, node::max_id-1,
                                                         node::pointer_type(10, node::max_id-2),
                                                         node::pointer_type(10, node::max_id-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (11,h)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(9, node::max_id-2,
                                                         node::pointer_type(10, node::max_id-1),
                                                         node::pointer_type(10, node::max_id-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (11,i)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(9, node::max_id-3,
                                                         node::pointer_type(10, node::max_id),
                                                         node::pointer_type(10, node::max_id-3))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (9,g)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(8, node::max_id,
                                                         node::pointer_type(9, node::max_id-3),
                                                         node::pointer_type(9, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (8,g)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(8, node::max_id-1,
                                                         node::pointer_type(9, node::max_id-1),
                                                         node::pointer_type(9, node::max_id-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (8,f)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(8, node::max_id-2,
                                                         node::pointer_type(9, node::max_id),
                                                         node::pointer_type(9, node::max_id-3))));

          // NOTE: (9,f) because the pair (7,f) is is merged with (8) which
          //       prunes that entire subtree away.

          AssertThat(out_nodes.can_pull(), Is().True()); // (7,8,f)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(7, node::max_id,
                                                         node::pointer_type(8, node::max_id-2),
                                                         node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (7,g,9)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(7, node::max_id-1,
                                                         node::pointer_type(true),
                                                         node::pointer_type(8, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (7,g)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(7, node::max_id-2,
                                                         node::pointer_type(8, node::max_id-1),
                                                         node::pointer_type(8, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (6,5,d)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(6, node::max_id,
                                                         node::pointer_type(7, node::max_id),
                                                         node::pointer_type(7, node::max_id-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5,d)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(6, node::max_id-1,
                                                         node::pointer_type(8, node::max_id-2),
                                                         node::pointer_type(7, node::max_id-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (4,5,d)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id,
                                                         node::pointer_type(6, node::max_id-1),
                                                         node::pointer_type(6, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (3,5,d)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id,
                                                         node::pointer_type(5, node::max_id),
                                                         node::pointer_type(6, node::max_id-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (2,5,d)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id-1,
                                                         node::pointer_type(6, node::max_id-1),
                                                         node::pointer_type(5, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1,5,d)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id,
                                                         node::pointer_type(4, node::max_id-1),
                                                         node::pointer_type(4, node::max_id))));

          // NOTE: The root (1,a,5,d) has x2 suppressed as the choice at (a)
          //       only is relevant for (b) and (c), not for (d).

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(13u,2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(12u,4u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(11u,4u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(10u,4u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(9u,4u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(8u,3u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(7u,3u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(6u,2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4u,2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,1u)));

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

        quantify_mode = quantify_mode_t::Auto;
      });

      describe("quantify_mode == Nested", [&]() {
        quantify_mode = quantify_mode_t::Nested;

        it("quantifies odd variables in BDD 4", [&]() {
          std::vector<bdd::label_type> call_history;

          bdd out = bdd_exists(bdd_4, [&call_history](const bdd::label_type x) -> bool {
            call_history.push_back(x);
            return x % 2;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (3)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::max_id,
                                                         ptr_uint64(2, ptr_uint64::max_id),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...

          // Check call history
          //
          // NOTE: Test failure does NOT indicate a bug, but only indicates a
          //       change. Please verify that this change makes sense and is as
          //       intended.
          AssertThat(call_history.size(), Is().EqualTo(4u));

          // - First check for at least one variable satisfying the predicate.
          //   This is then used for the inital transposition
          AssertThat(call_history.at(0), Is().EqualTo(3u));

          // - Nested sweep looking for the 'next_inner' bottom-up
          AssertThat(call_history.at(1), Is().EqualTo(2u));
          AssertThat(call_history.at(2), Is().EqualTo(1u));
          AssertThat(call_history.at(3), Is().EqualTo(0u));
        });

        it("quantifies odd variables in BDD 1", [&]() {
          bdd out = bdd_exists(bdd_1, [](const bdd::label_type x) -> bool {
            return x % 2;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies with always-true predicate in BDD 4 [&&]", [&]() {
          bdd out = bdd_exists(bdd_4, [](const bdd::label_type) -> bool {
            return true;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);
          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("bails out on a level that only shortcuts", [&bdd_9T]() {
          bdd out = bdd_exists(bdd_9T, [](const bdd::label_type x) -> bool {
            return !(x % 2);
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (7')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id,
                                                         ptr_uint64(5, node::max_id),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::max_id,
                                                         ptr_uint64(true),
                                                         ptr_uint64(3, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("bails out on a level that only is irrelevant", [&bdd_9F]() {
          bdd out = bdd_exists(bdd_9F, [](const bdd::label_type x) -> bool {
            return !(x % 2);
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (7')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id,
                                                         ptr_uint64(5, node::max_id),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("bails out on a level that both shortcuts and is irrelevant", [&bdd_6_x4T]() {
          bdd out = bdd_exists(bdd_6_x4T, [](const bdd::label_type x) -> bool {
            return x == 4 || x == 2 || x == 1;
          });

          // TODO predict output!
          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (7')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("kills intermediate dead partial solution", [&bdd_10]() {
          std::vector<bdd::label_type> call_history;
          bdd out = bdd_exists(bdd_10, [&call_history](const bdd::label_type x) -> bool {
            call_history.push_back(x);
            return x == 3 || x == 2;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::max_id,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables

          // Check call history
          //
          // NOTE: Test failure does NOT indicate a bug, but only indicates a
          //       change. Please verify that this change makes sense and is as
          //       intended.
          AssertThat(call_history.size(), Is().EqualTo(6u));

          // - First check for at least one variable satisfying the predicate.
          //   This is then used for the inital transposition
          AssertThat(call_history.at(0), Is().EqualTo(4u));
          AssertThat(call_history.at(1), Is().EqualTo(3u));

          // - Nested sweep looking for the 'next_inner' bottom-up
          AssertThat(call_history.at(2), Is().EqualTo(4u));
          AssertThat(call_history.at(3), Is().EqualTo(2u));
          AssertThat(call_history.at(4), Is().EqualTo(1u));
          AssertThat(call_history.at(5), Is().EqualTo(0u));
        });

        it("kills intermediate dead partial solutions multiple times", [&]() {
          bdd::label_type var = 7;

          /* expected
          //
          //         _1_
          //        /   \
          //        *   *
          //         \ /
          //          |
          //          /
          //         /
          //        *
          //        |
          //        T
          */
          bdd out = bdd_exists(bdd_6, [&var]() -> bdd::label_type {
            const bdd::label_type ret = var;
            var -= 2;
            return ret;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("quantifies exploding BDD 15", [&]() {
          bdd out = bdd_exists(bdd_15, [](const bdd::label_type x) -> bool { return x < 2; });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (o)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(13, node::max_id,
                                                         node::pointer_type(false),
                                                         node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (n)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(13, node::max_id-1,
                                                         node::pointer_type(true),
                                                         node::pointer_type(false))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (17,o)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(12, node::max_id,
                                                         node::pointer_type(13, node::max_id),
                                                         node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (17,n)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(12, node::max_id-1,
                                                         node::pointer_type(13, node::max_id-1),
                                                         node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (16,o)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(12, node::max_id-2,
                                                         node::pointer_type(true),
                                                         node::pointer_type(13, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (16,n)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(12, node::max_id-3,
                                                         node::pointer_type(true),
                                                         node::pointer_type(13, node::max_id-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (14,l)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(11, node::max_id,
                                                         node::pointer_type(12, node::max_id-3),
                                                         node::pointer_type(12, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (14,m)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(11, node::max_id-1,
                                                         node::pointer_type(12, node::max_id-2),
                                                         node::pointer_type(12, node::max_id-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (15,l)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(11, node::max_id-2,
                                                         node::pointer_type(12, node::max_id-1),
                                                         node::pointer_type(12, node::max_id-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (15,m)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(11, node::max_id-3,
                                                         node::pointer_type(12, node::max_id),
                                                         node::pointer_type(12, node::max_id-3))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (13,k)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(10, node::max_id,
                                                         node::pointer_type(11, node::max_id-3),
                                                         node::pointer_type(11, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (13,j)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(10, node::max_id-1,
                                                         node::pointer_type(11, node::max_id-2),
                                                         node::pointer_type(11, node::max_id-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (12,k)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(10, node::max_id-2,
                                                         node::pointer_type(11, node::max_id-1),
                                                         node::pointer_type(11, node::max_id-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (12,j)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(10, node::max_id-3,
                                                         node::pointer_type(11, node::max_id),
                                                         node::pointer_type(11, node::max_id-3))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (10,h)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(9, node::max_id,
                                                         node::pointer_type(10, node::max_id-3),
                                                         node::pointer_type(10, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (10,i)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(9, node::max_id-1,
                                                         node::pointer_type(10, node::max_id-2),
                                                         node::pointer_type(10, node::max_id-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (11,h)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(9, node::max_id-2,
                                                         node::pointer_type(10, node::max_id-1),
                                                         node::pointer_type(10, node::max_id-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (11,i)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(9, node::max_id-3,
                                                         node::pointer_type(10, node::max_id),
                                                         node::pointer_type(10, node::max_id-3))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (9,g)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(8, node::max_id,
                                                         node::pointer_type(9, node::max_id-3),
                                                         node::pointer_type(9, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (8,g)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(8, node::max_id-1,
                                                         node::pointer_type(9, node::max_id-1),
                                                         node::pointer_type(9, node::max_id-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (8,f)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(8, node::max_id-2,
                                                         node::pointer_type(9, node::max_id),
                                                         node::pointer_type(9, node::max_id-3))));

          // NOTE: (9,f) because the pair (7,f) is is merged with (8) which
          //       prunes that entire subtree away.

          AssertThat(out_nodes.can_pull(), Is().True()); // (7,8,f)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(7, node::max_id,
                                                         node::pointer_type(8, node::max_id-2),
                                                         node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (7,g,9)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(7, node::max_id-1,
                                                         node::pointer_type(true),
                                                         node::pointer_type(8, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (7,g)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(7, node::max_id-2,
                                                         node::pointer_type(8, node::max_id-1),
                                                         node::pointer_type(8, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (6,5,d)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(6, node::max_id,
                                                         node::pointer_type(7, node::max_id),
                                                         node::pointer_type(7, node::max_id-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5,d)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(6, node::max_id-1,
                                                         node::pointer_type(8, node::max_id-2),
                                                         node::pointer_type(7, node::max_id-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (4,5,d)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id,
                                                         node::pointer_type(6, node::max_id-1),
                                                         node::pointer_type(6, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (3,5,d)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id,
                                                         node::pointer_type(5, node::max_id),
                                                         node::pointer_type(6, node::max_id-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (2,5,d)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id-1,
                                                         node::pointer_type(6, node::max_id-1),
                                                         node::pointer_type(5, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1,5,d)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id,
                                                         node::pointer_type(4, node::max_id-1),
                                                         node::pointer_type(4, node::max_id))));

          // NOTE: The root (1,a,5,d) has x2 suppressed as the choice at (a)
          //       only is relevant for (b) and (c), not for (d).

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          // TODO

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(13u,2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(12u,4u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(11u,4u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(10u,4u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(9u,4u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(8u,3u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(7u,3u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(6u,2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4u,2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        quantify_mode = quantify_mode_t::Auto;
      });

      describe("quantify_mode == Auto", [&]() {
        quantify_mode = quantify_mode_t::Auto;

        it("collapses during initial transposition of all variables in BDD 4 [&&]", [&]() {
          std::vector<bdd::label_type> call_history;
          bdd out = bdd_exists(bdd_4, [&call_history](const bdd::label_type x) -> bool {
            call_history.push_back(x);
            return true;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);
          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...

          // Check call history
          //
          // NOTE: Test failure does NOT indicate a bug, but only indicates a
          //       change. Please verify that this change makes sense and is as
          //       intended.
          AssertThat(call_history.size(), Is().EqualTo(8u));

          // - First check for at least one variable satisfying the predicate.
          AssertThat(call_history.at(0), Is().EqualTo(3u));

          // - Count number of variables above widest
          AssertThat(call_history.at(1), Is().EqualTo(2u)); // <-- widest level
          AssertThat(call_history.at(2), Is().EqualTo(1u));
          AssertThat(call_history.at(3), Is().EqualTo(0u));

          // - First top-down sweep (root call)
          AssertThat(call_history.at(4), Is().EqualTo(0u));

          // - First top-down sweep
          AssertThat(call_history.at(5), Is().EqualTo(0u));
          AssertThat(call_history.at(6), Is().EqualTo(1u));
          AssertThat(call_history.at(7), Is().EqualTo(2u));
        });

        it("finishes during initial transposition of even variables in BDD 4 [const &]", [&]() {
          std::vector<bdd::label_type> call_history;

          const bdd in = bdd_4;
          const bdd out = bdd_exists(in, [&call_history](const bdd::label_type x) -> bool {
            call_history.push_back(x);
            return !(x % 2);
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (5)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (2')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::max_id,
                                                         ptr_uint64(3, ptr_uint64::max_id),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables ...

          // Check call history
          //
          // NOTE: Test failure does NOT indicate a bug, but only indicates a
          //       change. Please verify that this change makes sense and is as
          //       intended.
          AssertThat(call_history.size(), Is().EqualTo(10u));

          // - First check for at least one variable satisfying the predicate.
          AssertThat(call_history.at(0), Is().EqualTo(3u));
          AssertThat(call_history.at(1), Is().EqualTo(2u));

          // - Count number of variables above widest
          AssertThat(call_history.at(2), Is().EqualTo(2u)); // <-- widest level
          AssertThat(call_history.at(3), Is().EqualTo(1u));
          AssertThat(call_history.at(4), Is().EqualTo(0u));

          // - First top-down sweep (root call)
          AssertThat(call_history.at(5), Is().EqualTo(0u));

          // - First top-down sweep
          AssertThat(call_history.at(6), Is().EqualTo(0u));
          AssertThat(call_history.at(7), Is().EqualTo(1u));
          AssertThat(call_history.at(8), Is().EqualTo(2u));
          AssertThat(call_history.at(9), Is().EqualTo(3u));
        });

        it("collapses during repeated transposition with variables 1 2 variables in BDD 11a [&&]", [&]() {
          std::vector<bdd::label_type> call_history;
          bdd out = bdd_exists(bdd_12a, [&call_history](const bdd::label_type x) -> bool {
            call_history.push_back(x);
            return 0 < x && x < 3;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);
          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...

          // Check call history
          //
          // NOTE: Test failure does NOT indicate a bug, but only indicates a
          //       change. Please verify that this change makes sense and is as
          //       intended.
          AssertThat(call_history.size(), Is().EqualTo(17u));

          // - First check for at least one variable satisfying the predicate.
          AssertThat(call_history.at(0), Is().EqualTo(4u));
          AssertThat(call_history.at(1), Is().EqualTo(3u));
          AssertThat(call_history.at(2), Is().EqualTo(2u));

          // - Count number of variables above widest
          AssertThat(call_history.at(3), Is().EqualTo(3u)); // <-- widest level
          AssertThat(call_history.at(4), Is().EqualTo(2u));
          AssertThat(call_history.at(5), Is().EqualTo(1u));
          AssertThat(call_history.at(6), Is().EqualTo(0u));

          // - First top-down sweep (root call)
          AssertThat(call_history.at(7), Is().EqualTo(0u));

          // - First top-down sweep
          AssertThat(call_history.at(8),  Is().EqualTo(0u));
          AssertThat(call_history.at(9),  Is().EqualTo(1u));
          AssertThat(call_history.at(10), Is().EqualTo(2u));
          AssertThat(call_history.at(11), Is().EqualTo(3u));
          AssertThat(call_history.at(12), Is().EqualTo(4u));

          // - Second top-down sweep (root call)
          AssertThat(call_history.at(13),  Is().EqualTo(0u));

          // - Second top-down sweep
          AssertThat(call_history.at(14), Is().EqualTo(0u));
          AssertThat(call_history.at(15), Is().EqualTo(2u));
          AssertThat(call_history.at(16), Is().EqualTo(3u));
        });

        it("finishes during repeated transposition with variables 1 and 2 in BDD 11b [&&]", [&]() {
          std::vector<bdd::label_type> call_history;
          bdd out = bdd_exists(bdd_12b, [&call_history](const bdd::label_type x) -> bool {
            call_history.push_back(x);
            return 0 < x && x < 3;
          });

          /* expected
          //             1        ---- x0
          //            / \
          //           /   \      ---- x1
          //          /     \
          //          |     |     ---- x2
          //          |     |
          //          ?     ?     ---- x3
          //         / \   / \
          //         |  \  T |
          //         |   \__ /
          //         7      8     ---- x4
          //        / \    / \
          //        T F    F T
          //
          // The 'T' terminal at 'x3' is due to the pair (7,8) collapsing to the
          // 'T' terminal. This tuple is created from (7,F,8) which in turn is
          // created from (5,6,8) from the quantification (3,4) from (2).
          */
          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (8)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id,
                                                         node::pointer_type(false),
                                                         node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (7)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id-1,
                                                         node::pointer_type(true),
                                                         node::pointer_type(false))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5,6,8)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id,
                                                         node::pointer_type(true),
                                                         node::pointer_type(4, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5,6)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id-1,
                                                         node::pointer_type(4, node::max_id-1),
                                                         node::pointer_type(4, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::max_id,
                                                         node::pointer_type(3, node::max_id-1),
                                                         node::pointer_type(3, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4u,2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...

          // Check call history
          //
          // NOTE: Test failure does NOT indicate a bug, but only indicates a
          //       change. Please verify that this change makes sense and is as
          //       intended.
          AssertThat(call_history.size(), Is().EqualTo(19u));

          // - First check for at least one variable satisfying the predicate.
          AssertThat(call_history.at(0), Is().EqualTo(4u));
          AssertThat(call_history.at(1), Is().EqualTo(3u));
          AssertThat(call_history.at(2), Is().EqualTo(2u));

          // - Count number of variables above widest
          AssertThat(call_history.at(3), Is().EqualTo(4u)); // <-- widest level
          AssertThat(call_history.at(4), Is().EqualTo(3u)); // <-- widest level
          AssertThat(call_history.at(5), Is().EqualTo(2u));
          AssertThat(call_history.at(6), Is().EqualTo(1u));
          AssertThat(call_history.at(7), Is().EqualTo(0u));

          // - First top-down sweep (root call)
          AssertThat(call_history.at(8), Is().EqualTo(0u));

          // - First top-down sweep
          AssertThat(call_history.at(9),  Is().EqualTo(0u));
          AssertThat(call_history.at(10), Is().EqualTo(1u));
          AssertThat(call_history.at(11), Is().EqualTo(2u));
          AssertThat(call_history.at(12), Is().EqualTo(3u));
          AssertThat(call_history.at(13), Is().EqualTo(4u));

          // - Second top-down sweep (root call)
          AssertThat(call_history.at(14),  Is().EqualTo(0u));

          // - Second top-down sweep
          AssertThat(call_history.at(15), Is().EqualTo(0u));
          AssertThat(call_history.at(16), Is().EqualTo(2u));
          AssertThat(call_history.at(17), Is().EqualTo(3u));
          AssertThat(call_history.at(18), Is().EqualTo(4u));
        });

        it("finishes during repeated transposition with variables 1 and 2 in BDD 12 [&&]", [&]() {
          std::vector<bdd::label_type> call_history;
          bdd out = bdd_exists(bdd_13, [&call_history](const bdd::label_type x) -> bool {
            call_history.push_back(x);
            return x < 2;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (16)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(7, node::max_id,
                                                         node::pointer_type(false),
                                                         node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (15)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(6, node::max_id,
                                                         node::pointer_type(false),
                                                         node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (15,16)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(6, node::max_id-1,
                                                         node::pointer_type(7, node::max_id),
                                                         node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (14)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(6, node::max_id-2,
                                                         node::pointer_type(true),
                                                         node::pointer_type(false))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (14,16)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(6, node::max_id-3,
                                                         node::pointer_type(true),
                                                         node::pointer_type(7, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (13,15)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id,
                                                         node::pointer_type(6, node::max_id),
                                                         node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (13,15,16)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id-1,
                                                         node::pointer_type(6, node::max_id-1),
                                                         node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (13,14,16)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id-2,
                                                         node::pointer_type(6, node::max_id-3),
                                                         node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (12,15,16)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id-3,
                                                         node::pointer_type(true),
                                                         node::pointer_type(6, node::max_id-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (12,14)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id-4,
                                                         node::pointer_type(true),
                                                         node::pointer_type(6, node::max_id-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (12,14,16)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id-5,
                                                         node::pointer_type(true),
                                                         node::pointer_type(6, node::max_id-3))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (9,11,16)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id,
                                                         node::pointer_type(5, node::max_id-5),
                                                         node::pointer_type(5, node::max_id-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (8,11,16)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id-1,
                                                         node::pointer_type(5, node::max_id-3),
                                                         node::pointer_type(5, node::max_id-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (9,10,16)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id-2,
                                                         node::pointer_type(5, node::max_id-2),
                                                         node::pointer_type(5, node::max_id-3))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (8,10)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id-3,
                                                         node::pointer_type(5, node::max_id),
                                                         node::pointer_type(5, node::max_id-4))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (6,7,9,16)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id,
                                                         node::pointer_type(4, node::max_id-2),
                                                         node::pointer_type(4, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (6,7,8)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id-1,
                                                         node::pointer_type(4, node::max_id-3),
                                                         node::pointer_type(4, node::max_id-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (4,5,6,7)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id,
                                                         node::pointer_type(3, node::max_id-1),
                                                         node::pointer_type(3, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(7u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(6u,4u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5u,6u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4u,4u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...

          // Check call history
          //
          // NOTE: Test failure does NOT indicate a bug, but only indicates a
          //       change. Please verify that this change makes sense and is as
          //       intended.
          AssertThat(call_history.size(), Is().EqualTo(29u));

          // - First check for at least one variable satisfying the predicate.
          AssertThat(call_history.at(0), Is().EqualTo(7u));
          AssertThat(call_history.at(1), Is().EqualTo(6u));
          AssertThat(call_history.at(2), Is().EqualTo(5u));
          AssertThat(call_history.at(3), Is().EqualTo(4u));
          AssertThat(call_history.at(4), Is().EqualTo(3u));
          AssertThat(call_history.at(5), Is().EqualTo(2u));
          AssertThat(call_history.at(6), Is().EqualTo(1u));

          // - Count number of variables above widest
          AssertThat(call_history.at(7),  Is().EqualTo(4u)); // <-- widest level
          AssertThat(call_history.at(8),  Is().EqualTo(3u));
          AssertThat(call_history.at(9),  Is().EqualTo(2u));
          AssertThat(call_history.at(10), Is().EqualTo(1u));
          AssertThat(call_history.at(11), Is().EqualTo(0u));

          // - First top-down sweep (root call)
          AssertThat(call_history.at(12), Is().EqualTo(0u));

          // - First top-down sweep
          AssertThat(call_history.at(13), Is().EqualTo(0u));
          AssertThat(call_history.at(14), Is().EqualTo(1u));
          AssertThat(call_history.at(15), Is().EqualTo(2u));
          AssertThat(call_history.at(16), Is().EqualTo(3u));
          AssertThat(call_history.at(17), Is().EqualTo(4u));
          AssertThat(call_history.at(18), Is().EqualTo(5u));
          AssertThat(call_history.at(19), Is().EqualTo(6u));
          AssertThat(call_history.at(20), Is().EqualTo(7u));

          // - Second top-down sweep (root call)
          AssertThat(call_history.at(21), Is().EqualTo(1u));

          // - Second top-down sweep
          AssertThat(call_history.at(22), Is().EqualTo(1u));
          AssertThat(call_history.at(23), Is().EqualTo(2u));
          AssertThat(call_history.at(24), Is().EqualTo(3u));
          AssertThat(call_history.at(25), Is().EqualTo(4u));
          AssertThat(call_history.at(26), Is().EqualTo(5u));
          AssertThat(call_history.at(27), Is().EqualTo(6u));
          AssertThat(call_history.at(28), Is().EqualTo(7u));
        });

        it("finishes early during repeated transposition [&&]", [&]() {
          std::vector<bdd::label_type> call_history;
          bdd out = bdd_exists(bdd_10, [&call_history](const bdd::label_type x) -> bool {
            call_history.push_back(x);
            return 1 < x;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::max_id,
                                                         node::pointer_type(false),
                                                         node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...

          // Check call history
          //
          // NOTE: Test failure does NOT indicate a bug, but only indicates a
          //       change. Please verify that this change makes sense and is as
          //       intended.
          AssertThat(call_history.size(), Is().EqualTo(10u));

          // - First check for at least one variable satisfying the predicate.
          AssertThat(call_history.at(0), Is().EqualTo(4u));

          // - Count number of variables above widest
          AssertThat(call_history.at(1), Is().EqualTo(3u)); // <-- widest level
          AssertThat(call_history.at(2), Is().EqualTo(2u));
          AssertThat(call_history.at(3), Is().EqualTo(1u));
          AssertThat(call_history.at(4), Is().EqualTo(0u));

          // - First top-down sweep (root call)
          AssertThat(call_history.at(5), Is().EqualTo(0u));

          // - First top-down sweep
          AssertThat(call_history.at(6),  Is().EqualTo(0u));
          AssertThat(call_history.at(7),  Is().EqualTo(1u));
          AssertThat(call_history.at(8),  Is().EqualTo(2u));
          AssertThat(call_history.at(9),  Is().EqualTo(3u));

          // NOTE: Even though there are three levels that should be quantified,
          //       we only do one partial quantification.
        });

        it("switches to nested sweeping when the transposition explodes with BDD 15 [&&]", [&]() {
          std::vector<bdd::label_type> call_history;
          bdd out = bdd_exists(bdd_15, [&call_history](const bdd::label_type x) -> bool {
            call_history.push_back(x);
            return x < 2;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (o)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(13, node::max_id,
                                                         node::pointer_type(false),
                                                         node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (n)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(13, node::max_id-1,
                                                         node::pointer_type(true),
                                                         node::pointer_type(false))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (17,o)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(12, node::max_id,
                                                         node::pointer_type(13, node::max_id),
                                                         node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (17,n)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(12, node::max_id-1,
                                                         node::pointer_type(13, node::max_id-1),
                                                         node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (16,o)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(12, node::max_id-2,
                                                         node::pointer_type(true),
                                                         node::pointer_type(13, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (16,n)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(12, node::max_id-3,
                                                         node::pointer_type(true),
                                                         node::pointer_type(13, node::max_id-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (14,l)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(11, node::max_id,
                                                         node::pointer_type(12, node::max_id-3),
                                                         node::pointer_type(12, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (14,m)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(11, node::max_id-1,
                                                         node::pointer_type(12, node::max_id-2),
                                                         node::pointer_type(12, node::max_id-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (15,l)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(11, node::max_id-2,
                                                         node::pointer_type(12, node::max_id-1),
                                                         node::pointer_type(12, node::max_id-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (15,m)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(11, node::max_id-3,
                                                         node::pointer_type(12, node::max_id),
                                                         node::pointer_type(12, node::max_id-3))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (13,k)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(10, node::max_id,
                                                         node::pointer_type(11, node::max_id-3),
                                                         node::pointer_type(11, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (13,j)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(10, node::max_id-1,
                                                         node::pointer_type(11, node::max_id-2),
                                                         node::pointer_type(11, node::max_id-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (12,k)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(10, node::max_id-2,
                                                         node::pointer_type(11, node::max_id-1),
                                                         node::pointer_type(11, node::max_id-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (12,j)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(10, node::max_id-3,
                                                         node::pointer_type(11, node::max_id),
                                                         node::pointer_type(11, node::max_id-3))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (10,h)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(9, node::max_id,
                                                         node::pointer_type(10, node::max_id-3),
                                                         node::pointer_type(10, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (10,i)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(9, node::max_id-1,
                                                         node::pointer_type(10, node::max_id-2),
                                                         node::pointer_type(10, node::max_id-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (11,h)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(9, node::max_id-2,
                                                         node::pointer_type(10, node::max_id-1),
                                                         node::pointer_type(10, node::max_id-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (11,i)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(9, node::max_id-3,
                                                         node::pointer_type(10, node::max_id),
                                                         node::pointer_type(10, node::max_id-3))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (9,g)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(8, node::max_id,
                                                         node::pointer_type(9, node::max_id-3),
                                                         node::pointer_type(9, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (8,g)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(8, node::max_id-1,
                                                         node::pointer_type(9, node::max_id-1),
                                                         node::pointer_type(9, node::max_id-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (8,f)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(8, node::max_id-2,
                                                         node::pointer_type(9, node::max_id),
                                                         node::pointer_type(9, node::max_id-3))));

          // NOTE: (9,f) because the pair (7,f) is is merged with (8) which
          //       prunes that entire subtree away.

          AssertThat(out_nodes.can_pull(), Is().True()); // (7,8,f)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(7, node::max_id,
                                                         node::pointer_type(8, node::max_id-2),
                                                         node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (7,g,9)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(7, node::max_id-1,
                                                         node::pointer_type(true),
                                                         node::pointer_type(8, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (7,g)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(7, node::max_id-2,
                                                         node::pointer_type(8, node::max_id-1),
                                                         node::pointer_type(8, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (6,5,d)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(6, node::max_id,
                                                         node::pointer_type(7, node::max_id),
                                                         node::pointer_type(7, node::max_id-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5,d)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(6, node::max_id-1,
                                                         node::pointer_type(8, node::max_id-2),
                                                         node::pointer_type(7, node::max_id-2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (4,5,d)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id,
                                                         node::pointer_type(6, node::max_id-1),
                                                         node::pointer_type(6, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (3,5,d)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id,
                                                         node::pointer_type(5, node::max_id),
                                                         node::pointer_type(6, node::max_id-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (2,5,d)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id-1,
                                                         node::pointer_type(6, node::max_id-1),
                                                         node::pointer_type(5, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1,5,d)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id,
                                                         node::pointer_type(4, node::max_id-1),
                                                         node::pointer_type(4, node::max_id))));

          // NOTE: The root (1,a,5,d) has x2 suppressed as the choice at (a)
          //       only is relevant for (b) and (c), not for (d).

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          // TODO

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(13u,2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(12u,4u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(11u,4u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(10u,4u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(9u,4u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(8u,3u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(7u,3u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(6u,2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4u,2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,1u)));

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

        it("transposes with a single partial quantification if all variables are deep [&&]", [&]() {
          std::vector<bdd::label_type> call_history;
          bdd out = bdd_exists(bdd_14a, [&call_history](const bdd::label_type x) -> bool {
            call_history.push_back(x);
            return x == 5 || x == 6;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (4)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id,
                                                         node::pointer_type(false),
                                                         node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (6)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id-1,
                                                         node::pointer_type(true),
                                                         node::pointer_type(false))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (2)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id,
                                                         node::pointer_type(4, node::max_id),
                                                         node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (3)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id-1,
                                                         node::pointer_type(true),
                                                         node::pointer_type(4, node::max_id-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id,
                                                         node::pointer_type(3, node::max_id),
                                                         node::pointer_type(3, node::max_id-1))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4u,2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...

          // Check call history
          //
          // NOTE: Test failure does NOT indicate a bug, but only indicates a
          //       change. Please verify that this change makes sense and is as
          //       intended.
          AssertThat(call_history.size(), Is().EqualTo(20u));

          // - First check for at least one variable satisfying the predicate.
          AssertThat(call_history.at(0), Is().EqualTo(8u));
          AssertThat(call_history.at(1), Is().EqualTo(7u));
          AssertThat(call_history.at(2), Is().EqualTo(6u));

          // - Count number of variables above widest
          AssertThat(call_history.at(3), Is().EqualTo(4u)); // <-- widest level
          AssertThat(call_history.at(4), Is().EqualTo(3u));
          AssertThat(call_history.at(5), Is().EqualTo(2u));

          // - Top-down sweep (root call)
          AssertThat(call_history.at(6), Is().EqualTo(2u));

          // - Top-down sweep
          AssertThat(call_history.at(7),  Is().EqualTo(2u));
          AssertThat(call_history.at(8),  Is().EqualTo(3u));
          AssertThat(call_history.at(9),  Is().EqualTo(4u));
          AssertThat(call_history.at(10), Is().EqualTo(5u));
          AssertThat(call_history.at(11), Is().EqualTo(6u));
          AssertThat(call_history.at(12), Is().EqualTo(7u));
          AssertThat(call_history.at(13), Is().EqualTo(8u));

          // - Nested Sweeping (x5 is gone)
          AssertThat(call_history.at(14), Is().EqualTo(8u));
          AssertThat(call_history.at(15), Is().EqualTo(7u));
          AssertThat(call_history.at(16), Is().EqualTo(6u));
          AssertThat(call_history.at(17), Is().EqualTo(4u));
          AssertThat(call_history.at(18), Is().EqualTo(3u));
          AssertThat(call_history.at(19), Is().EqualTo(2u));
        });

        it("switches to nested sweeping if the only variables left are deep [&&]", [&]() {
          std::vector<bdd::label_type> call_history;
          bdd out = bdd_exists(bdd_14b, [&call_history](const bdd::label_type x) -> bool {
            call_history.push_back(x);
            return x == 1 || x == 5 || x == 6;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (4)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id,
                                                         node::pointer_type(false),
                                                         node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (6)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id-1,
                                                         node::pointer_type(true),
                                                         node::pointer_type(false))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (2)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id,
                                                         node::pointer_type(4, node::max_id),
                                                         node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (3)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id-1,
                                                         node::pointer_type(true),
                                                         node::pointer_type(4, node::max_id-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id,
                                                         node::pointer_type(3, node::max_id),
                                                         node::pointer_type(3, node::max_id-1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1*)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::max_id,
                                                         node::pointer_type(2, node::max_id),
                                                         node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4u,2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...

          // Check call history
          //
          // NOTE: Test failure does NOT indicate a bug, but only indicates a
          //       change. Please verify that this change makes sense and is as
          //       intended.
          AssertThat(call_history.size(), Is().EqualTo(25u));

          // - First check for at least one variable satisfying the predicate.
          AssertThat(call_history.at(0), Is().EqualTo(8u));
          AssertThat(call_history.at(1), Is().EqualTo(7u));
          AssertThat(call_history.at(2), Is().EqualTo(6u));

          // - Count number of variables above widest
          AssertThat(call_history.at(3), Is().EqualTo(4u)); // <-- widest level
          AssertThat(call_history.at(4), Is().EqualTo(3u));
          AssertThat(call_history.at(5), Is().EqualTo(2u));
          AssertThat(call_history.at(6), Is().EqualTo(1u));
          AssertThat(call_history.at(7), Is().EqualTo(0u));

          // - Top-down sweep (root call)
          AssertThat(call_history.at(8), Is().EqualTo(0u));

          // - Top-down sweep
          AssertThat(call_history.at(9),  Is().EqualTo(0u));
          AssertThat(call_history.at(10), Is().EqualTo(1u));
          AssertThat(call_history.at(11), Is().EqualTo(2u));
          AssertThat(call_history.at(12), Is().EqualTo(3u));
          AssertThat(call_history.at(13), Is().EqualTo(4u));
          AssertThat(call_history.at(14), Is().EqualTo(5u));
          AssertThat(call_history.at(15), Is().EqualTo(6u));
          AssertThat(call_history.at(16), Is().EqualTo(7u));
          AssertThat(call_history.at(17), Is().EqualTo(8u));

          // - Nested Sweeping (x5 is gone)
          AssertThat(call_history.at(18), Is().EqualTo(8u));
          AssertThat(call_history.at(19), Is().EqualTo(7u));
          AssertThat(call_history.at(20), Is().EqualTo(6u));
          AssertThat(call_history.at(21), Is().EqualTo(4u));
          AssertThat(call_history.at(22), Is().EqualTo(3u));
          AssertThat(call_history.at(23), Is().EqualTo(2u));
          AssertThat(call_history.at(24), Is().EqualTo(0u));
        });

        quantify_mode = quantify_mode_t::Auto;
      });
    });

    describe("bdd_exists(const bdd&, const generator<bdd::label_type>&)", [&]() {
      it("returns input on -1 generator BDD 1 [&&]", [&]() {
        __bdd out = bdd_exists(bdd_1, []() -> bdd::label_type { return -1; });
        AssertThat(out.get<shared_levelized_file<bdd::node_type>>(), Is().EqualTo(bdd_1));
      });

      describe("quantify_mode == Singleton", [&]() {
        quantify_mode = quantify_mode_t::Singleton;

        it("quantifies 3, 1, -1 in BDD 4 [&&]", [&]() {
          bdd::label_type var = 3;

          bdd out = bdd_exists(bdd_4, [&var]() -> bdd::label_type {
            const bdd::label_type ret = var;
            var -= 2;
            return ret;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (3)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::max_id,
                                                         ptr_uint64(2, ptr_uint64::max_id),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("quantifies 2, 0, -2 in BDD 4 [const &]", [&]() {
          const bdd in = bdd_4;

          bdd::label_type var = 2;

          bdd out = bdd_exists(in, [&var]() -> bdd::label_type {
            const bdd::label_type ret = var;
            var -= 2;
            return ret;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (5)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (2')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::max_id,
                                                         ptr_uint64(3, ptr_uint64::max_id),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("quantifies 1, -1 variables in BDD 1 [&&]", [&]() {
          bdd::label_type var = 1;

          bdd out = bdd_exists(bdd_1, [&var]() -> bdd::label_type {
            const bdd::label_type ret = var;
            var -= 2;
            return ret;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("terminates early when quantifying to a terminal in BDD 3 [&&]", [&]() {
          int calls = 0;

          const bdd out = bdd_exists(bdd_3, [&calls]() -> bdd::label_type {
            return 2 - 2*(calls++);
          });

          // What could be expected is 3 calls: 2, 0, -2 . But, here it terminates early.
          AssertThat(calls, Is().EqualTo(2));

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());
        });

        quantify_mode = quantify_mode_t::Auto;
      });

      describe("quantify_mode == Nested", [&]() {
        quantify_mode = quantify_mode_t::Nested;

        it("quantifies 3, 1, -1 in BDD 4 [&&]", [&]() {
          bdd::label_type var = 3;

          bdd out = bdd_exists(bdd_4, [&var]() -> bdd::label_type {
            const bdd::label_type ret = var;
            var -= 2;
            return ret;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (3)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::max_id,
                                                         ptr_uint64(2, ptr_uint64::max_id),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("quantifies 1, -1 variables in BDD 1 [&&]", [&]() {
          bdd::label_type var = 1;

          bdd out = bdd_exists(bdd_1, [&var]() -> bdd::label_type {
            const bdd::label_type ret = var;
            var -= 2;
            return ret;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("bails out on a level that only shortcuts", [&bdd_9T]() {
          bdd::label_type var = 6;
          bdd out = bdd_exists(bdd_9T, [&var]() -> bdd::label_type {
            const bdd::label_type res = var;
            var -= 2;
            return res;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (7')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id,
                                                         ptr_uint64(5, node::max_id),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::max_id,
                                                         ptr_uint64(true),
                                                         ptr_uint64(3, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("bails out on a level that only is irrelevant", [&bdd_9F]() {
          bdd::label_type var = 6;
          bdd out = bdd_exists(bdd_9F, [&var]() -> bdd::label_type {
            const bdd::label_type res = var;
            var -= 2;
            return res;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (7')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id,
                                                         ptr_uint64(5, node::max_id),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("bails out on a level that both shortcuts and is irrelevant", [&bdd_6_x4T]() {
          bdd::label_type var = 4;
          bdd out = bdd_exists(bdd_6_x4T, [&var]() -> bdd::label_type {
            const bdd::label_type res = var;
            switch (var) {
            case 4:  { var = 2;  break; } // <-- 4: transposing
            case 2:  { var = 1;  break; } // <-- 2: shortuctting / irrelevant
            default: { var = -1; break; } // <-- 1: final sweep
            }
            return res;
          });

          // TODO predict output!
          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (7')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("kills intermediate dead partial solution", [&bdd_10]() {
          bdd::label_type var = 3;
          bdd out = bdd_exists(bdd_10, [&var]() -> bdd::label_type {
            const bdd::label_type res = var;
            if (2 < var) { var -= 1; }
            else         { var = -1; }
            return res;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1)
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::max_id,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("kills intermediate dead partial solutions multiple times", [&]() {
          bdd::label_type var = 7;

          /* expected
          //
          //         _1_
          //        /   \
          //        *   *
          //         \ /
          //          |
          //          /
          //         /
          //        *
          //        |
          //        T
          */
          bdd out = bdd_exists(bdd_6, [&var]() -> bdd::label_type {
            const bdd::label_type ret = var;
            var -= 2;
            return ret;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        quantify_mode = quantify_mode_t::Auto;
      });
    });

    describe("bdd_exists(const bdd&, IT begin, IT end)", [&]() {
      it("returns original file for [].begin() in BDD 1 [&]", [&]() {
        const std::vector<bdd::label_type> vars = { };
        const bdd out = bdd_exists(bdd_1, vars.begin(), vars.end());

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_1));
      });

      it("returns original file for [3, 5].rbegin() in BDD 11 [&]", [&]() {
        const std::vector<bdd::label_type> vars = { 3,5 };
        const bdd out = bdd_exists(bdd_11, vars.rbegin(), vars.rend());

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_11));
      });

      it("returns original file for [0, 3].rbegin() in BDD 11 [&]", [&]() {
        const std::vector<bdd::label_type> vars = { 0,3 };
        const bdd out = bdd_exists(bdd_11, vars.rbegin(), vars.rend());

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_11));
      });

      it("quantifies [1, 3].rbegin() in BDD 4 [&&]", [&]() {
        std::vector<bdd::label_type> vars = { 1 , 3 };

        bdd out = bdd_exists(bdd_4, vars.rbegin(), vars.rend());

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True()); // (3)
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id,
                                                       ptr_uint64(false),
                                                       ptr_uint64(true))));

        AssertThat(out_nodes.can_pull(), Is().True()); // (1)
        AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::max_id,
                                                       ptr_uint64(2, ptr_uint64::max_id),
                                                       ptr_uint64(true))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2u,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u,1u)));

        AssertThat(out_meta.can_pull(), Is().False());
      });

      it("quantifies [2, 0].begin() in BDD 4 [const &]", [&]() {
        const bdd in = bdd_4;
        const std::vector<bdd::label_type> vars = { 2, 0 };

        bdd out = bdd_exists(in, vars.begin(), vars.end());

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True()); // (5)
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id,
                                                       ptr_uint64(false),
                                                       ptr_uint64(true))));

        AssertThat(out_nodes.can_pull(), Is().True()); // (2')
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::max_id,
                                                       ptr_uint64(3, ptr_uint64::max_id),
                                                       ptr_uint64(true))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u,1u)));

        AssertThat(out_meta.can_pull(), Is().False());
      });

      it("quantifies [4, 2, 0].begin() in BDD 4 [const &]", [&]() {
        const bdd in = bdd_4;
        const std::vector<bdd::label_type> vars = { 4, 2, 0 };

        bdd out = bdd_exists(in, vars.begin(), vars.end());

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True()); // (5)
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id,
                                                       ptr_uint64(false),
                                                       ptr_uint64(true))));

        AssertThat(out_nodes.can_pull(), Is().True()); // (2')
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::max_id,
                                                       ptr_uint64(3, ptr_uint64::max_id),
                                                       ptr_uint64(true))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u,1u)));

        AssertThat(out_meta.can_pull(), Is().False());
      });
    });

    ////////////////////////////////////////////////////////////////////////////
    // We will not test the Forall operator as much, since it is the same
    // underlying algorithm, but just with the AND operator.
    describe("bdd_forall(const bdd&, bdd::label_type)", [&]() {
      it("quantifies T terminal-only BDD as itself", [&]() {
        __bdd out = bdd_forall(terminal_T, 42);

        AssertThat(out.get<shared_levelized_file<bdd::node_type>>(), Is().EqualTo(terminal_T));
        AssertThat(out.negate, Is().False());
      });

      it("quantifies F terminal-only BDD as itself", [&]() {
        __bdd out = bdd_forall(terminal_F, 21);

        AssertThat(out.get<shared_levelized_file<bdd::node_type>>(), Is().EqualTo(terminal_F));
        AssertThat(out.negate, Is().False());
      });

      it("quantifies root with non-shortcutting terminal [BDD 1]", [&]() {
        __bdd out = bdd_forall(bdd_1, 0);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(1,0), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1u,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().GreaterThanOrEqualTo(0u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("quantifies root of [BDD 3]", [&]() {
        __bdd out = bdd_forall(bdd_3, 0);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(1,0), true,  ptr_uint64(2,1) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(2,0), true,  ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(2,1), false, ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(2,1), true,  ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2u,2u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false], Is().EqualTo(3u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("should prune shortcuttable requests [BDD 4]", [&]() {
        __bdd out = bdd_forall(bdd_4, 2);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(1,0), true,  ptr_uint64(3,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // false due to 3.low()
                   Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // false due to 3.low()
                   Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // false due to 5.low()
                   Is().EqualTo(arc { ptr_uint64(3,0), false, ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), // true due to 5.high() and 4.high()
                   Is().EqualTo(arc { ptr_uint64(3,0), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3u,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false], Is().EqualTo(3u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("can forward information across a level [BDD 6]", [&]() {
        __bdd out = bdd_forall(bdd_6, 1);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True()); // (4,6)
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (5,6)
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(2,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (7,8)
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(2,1), false, ptr_uint64(3,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (8,T)
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(2,0), true,  ptr_uint64(3,1) }));

        AssertThat(arcs.can_pull_internal(), Is().False());
        AssertThat(arcs.can_pull_terminal(), Is().True()); // (4,6)
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (5,6)
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(2,1), true,  ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (7,8)
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(3,0), false, ptr_uint64(false) }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(3,0), true,  ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (8,T)
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(3,1), false, ptr_uint64(false) }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(3,1), true,  ptr_uint64(true) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2u,2u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3u,2u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false], Is().EqualTo(5u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],  Is().EqualTo(1u));
      });

      it("should collapse tuple requests of the same node back into request on a single node [BDD 8a]", [&]() {
        __bdd out = bdd_forall(bdd_8a, 1);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True()); // (3,4)
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (5,_)
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(3,0) }));
        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc { ptr_uint64(2,0), true,  ptr_uint64(3,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (3,4)
        AssertThat(arcs.pull_terminal(), // false due to 4.low()
                   Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(false) }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // (5,_)
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(3,0), false, ptr_uint64(false) }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc { ptr_uint64(3,0), true,  ptr_uint64(true) }));


        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2u,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3u,1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],  Is().EqualTo(1u));
      });
    });

    describe("bdd_forall(const bdd&, const predicate<bdd::label_type>&)", [&]() {
      it("returns input on always-false predicate BDD 1 [&&]", [&]() {
        __bdd out = bdd_forall(bdd_1, [](const bdd::label_type) -> bool {
          return false;
        });

        AssertThat(out.get<shared_levelized_file<bdd::node_type>>(), Is().EqualTo(bdd_1));
      });

      describe("quantify_mode == Singleton", [&]() {
        quantify_mode = quantify_mode_t::Singleton;

        it("quantifies even variables in BDD 1 [const &]", [&]() {
          const bdd in = bdd_1;
          const bdd out = bdd_forall(in, [](const bdd::label_type x) -> bool {
            return !(x % 2);
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::max_id,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies odd variables in BDD 1 [&&]", [&]() {
          const bdd out = bdd_forall(bdd_1, [](const bdd::label_type x) -> bool {
            return x % 2;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::max_id,
                                                         ptr_uint64(true),
                                                         ptr_uint64(false))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies <= 2 variables in BDD 4 [&&]", [&]() {
          const bdd out = bdd_forall(bdd_4, [](const bdd::label_type x) -> bool {
            return x <= 2;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);
          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("terminates early when quantifying to a terminal in BDD 1 [&&]", [&]() {
          // TODO: top-down dependant?
          int calls = 0;

          const bdd out = bdd_forall(bdd_5, [&calls](const bdd::label_type) -> bool {
            calls++;
            return true;
          });

          AssertThat(calls, Is().EqualTo(1));

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);
          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        quantify_mode = quantify_mode_t::Auto;
      });

      describe("quantify_mode == Partial", [&]() {
        quantify_mode = quantify_mode_t::Partial;

        // TODO

        quantify_mode = quantify_mode_t::Auto;
      });

      describe("quantify_mode == Nested", [&]() {
        quantify_mode = quantify_mode_t::Nested;

        it("quantifies even variables in BDD 1", [&]() {
          const bdd out = bdd_forall(bdd_1, [](const bdd::label_type x) -> bool {
            return !(x % 2);
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::max_id,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("bails out on a level that only shortcuts", [&bdd_9T]() {
          bdd out = bdd_forall(bdd_not(bdd_9T), [](const bdd::label_type x) -> bool {
            return !(x % 2);
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (7')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id,
                                                         ptr_uint64(true),
                                                         ptr_uint64(false))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id,
                                                         ptr_uint64(5, node::max_id),
                                                         ptr_uint64(false))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::max_id,
                                                         ptr_uint64(false),
                                                         ptr_uint64(3, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("bails out on a level that only is irrelevant", [&bdd_9F]() {
          bdd out = bdd_forall(bdd_not(bdd_9F), [](const bdd::label_type x) -> bool {
            return !(x % 2);
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (7')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id,
                                                         ptr_uint64(true),
                                                         ptr_uint64(false))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id,
                                                         ptr_uint64(5, node::max_id),
                                                         ptr_uint64(false))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        quantify_mode = quantify_mode_t::Auto;
      });

      describe("quantify_mode == Auto", [&]() {
        quantify_mode = quantify_mode_t::Auto;

        it("quantifies odd variables in BDD 1", [&]() {
          const bdd out = bdd_forall(bdd_1, [](const bdd::label_type x) -> bool {
            return x % 2;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::max_id,
                                                         ptr_uint64(true),
                                                         ptr_uint64(false))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        quantify_mode = quantify_mode_t::Auto;
      });
    });

    describe("bdd_forall(const bdd&, const generator<bdd::label_type>&)", [&]() {
      it("returns input on -1 geneator in BDD 1 [&&]", [&]() {
        __bdd out = bdd_forall(bdd_1, []() -> bdd::label_type { return -1; });
        AssertThat(out.get<shared_levelized_file<bdd::node_type>>(), Is().EqualTo(bdd_1));
      });

      describe("quantify_mode == Singleton", [&]() {
        quantify_mode = quantify_mode_t::Singleton;

        it("quantifies 0, -2 in BDD 1 [const &]", [&]() {
          const bdd in = bdd_1;

          bdd::label_type var = 0;

          const bdd out = bdd_forall(in, [&var]() -> bdd::label_type {
            const bdd::label_type ret = var;
            var -= 2;
            return ret;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::max_id,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("quantifies 1, -1 in BDD 1 [&&]", [&]() {
          bdd::label_type var = 1;

          const bdd out = bdd_forall(bdd_1, [&var]() -> bdd::label_type {
            const bdd::label_type ret = var;
            var -= 2;
            return ret;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::max_id,
                                                         ptr_uint64(true),
                                                         ptr_uint64(false))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("terminates early when quantifying to a terminal in BDD 3 [&&]", [&]() {
          int calls = 0;

          const bdd out = bdd_forall(bdd_3, [&calls]() -> bdd::label_type {
            return 2 - 2*(calls++);
          });

          // What could be expected is 3 calls: 2, 0, -2 . But, here it terminates early.
          AssertThat(calls, Is().EqualTo(2));

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());
        });

        quantify_mode = quantify_mode_t::Auto;
      });

      describe("quantify_mode == Nested", [&]() {
        quantify_mode = quantify_mode_t::Nested;

        it("quantifies 0, -2 in BDD 1", [&]() {
          bdd::label_type var = 0;

          const bdd out = bdd_forall(bdd_1, [&var]() -> bdd::label_type {
            const bdd::label_type ret = var;
            var -= 2;
            return ret;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::max_id,
                                                         ptr_uint64(false),
                                                         ptr_uint64(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("quantifies 1, -1 in BDD 1", [&]() {
          bdd::label_type var = 1;

          const bdd out = bdd_forall(bdd_1, [&var]() -> bdd::label_type {
            const bdd::label_type ret = var;
            var -= 2;
            return ret;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::max_id,
                                                         ptr_uint64(true),
                                                         ptr_uint64(false))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("bails out on a level that only shortcuts", [&bdd_9T]() {
          bdd::label_type var = 6;
          bdd out = bdd_forall(bdd_not(bdd_9T), [&var]() -> bdd::label_type {
            const bdd::label_type res = var;
            var -= 2;
            return res;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (7')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id,
                                                         ptr_uint64(true),
                                                         ptr_uint64(false))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id,
                                                         ptr_uint64(5, node::max_id),
                                                         ptr_uint64(false))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::max_id,
                                                         ptr_uint64(false),
                                                         ptr_uint64(3, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("bails out on a level that only is irrelevant", [&bdd_9F]() {
          bdd::label_type var = 6;
          bdd out = bdd_forall(bdd_not(bdd_9F), [&var]() -> bdd::label_type {
            const bdd::label_type res = var;
            var -= 2;
            return res;
          });

          node_test_stream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (7')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id,
                                                         ptr_uint64(true),
                                                         ptr_uint64(false))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id,
                                                         ptr_uint64(5, node::max_id),
                                                         ptr_uint64(false))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_stream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5u,1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u,1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        quantify_mode = quantify_mode_t::Auto;
      });
    });

    describe("bdd_forall(const bdd&, IT begin, IT end)", [&]() {
      it("quantifies [0].rbegin() in BDD 1 [const &]", [&]() {
        const bdd in = bdd_1;
        const std::vector<bdd::label_type> vars = { 0 };

        const bdd out = bdd_forall(in, vars.rbegin(), vars.rend());

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True()); // (1')
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::max_id,
                                                       ptr_uint64(false),
                                                       ptr_uint64(true))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u,1u)));

        AssertThat(out_meta.can_pull(), Is().False());
      });

      it("quantifies [1].begin() in BDD 1 [&&]", [&]() {
        const std::vector<bdd::label_type> vars = { 1 };
        const bdd out = bdd_forall(bdd_1, vars.begin(), vars.end());

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True()); // (1')
        AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::max_id,
                                                       ptr_uint64(true),
                                                       ptr_uint64(false))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u,1u)));

        AssertThat(out_meta.can_pull(), Is().False());
      });
    });
  });
 });
