go_bandit([]() {
  describe("ZDD: Projection", [&]() {
    //////////////////////
    // Sink cases

    // Ø
    node_file zdd_empty;
    {
      node_writer nw(zdd_empty);
      nw << create_sink(false);
    }

    // { Ø }
    node_file zdd_null;
    {
      node_writer nw(zdd_null);
      nw << create_sink(true);
    }

    it("computes Ø with dom = {2,4,6} [const &]", [&](){
      label_file dom;
      {
        label_writer lw(dom);
        lw << 2 << 4 << 6;
      }

      zdd out = zdd_project(zdd_empty, dom);

      node_test_stream out_nodes(out);
      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(false)));

      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream<node_t, NODE_FILE_COUNT> ms(out);
      AssertThat(ms.can_pull(), Is().False());
    });

    it("computes { Ø } with dom = {1,3,5} [&&]", [&](){
      label_file dom;
      {
        label_writer lw(dom);
        lw << 1 << 3 << 5;
      }

      zdd out = zdd_project(zdd(zdd_null), dom);

      node_test_stream out_nodes(out);
      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));

      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream<node_t, NODE_FILE_COUNT> ms(out);
      AssertThat(ms.can_pull(), Is().False());
    });

    it("computes with dom = Ø to be Ø for Ø as input [&&]", [&](){
      label_file dom;

      zdd out = zdd_project(zdd(zdd_empty), dom);

      node_test_stream out_nodes(out);
      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(false)));

      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream<node_t, NODE_FILE_COUNT> ms(out);
      AssertThat(ms.can_pull(), Is().False());
    });

    it("computes with dom = Ø to be { Ø } for { Ø } as input [const &]", [&](){
      label_file dom;

      zdd out = zdd_project(zdd_null, dom);

      node_test_stream out_nodes(out);
      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));

      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream<node_t, NODE_FILE_COUNT> ms(out);
      AssertThat(ms.can_pull(), Is().False());
    });

    //////////////////////
    // Non-sink edge cases
    ptr_t sink_F = create_sink_ptr(false);
    ptr_t sink_T = create_sink_ptr(true);

    // { Ø, {0}, {1}, {1,2}, {1,3}, {1,3,4} }
    /*
                     1         ---- x0
                    / \
                    2 T        ---- x1
                   ||
                    3          ---- x2
                   / \
                   4 T         ---- x3
                  / \
                  F 5          ---- x4
                   / \
                   T T
    */
    node_file zdd_1;
    {
      node_writer nw(zdd_1);
      nw << create_node(4, MAX_ID, sink_T, sink_T)
         << create_node(3, MAX_ID, sink_F, create_node_ptr(4, MAX_ID))
         << create_node(2, MAX_ID, create_node_ptr(3, MAX_ID), sink_T)
         << create_node(1, MAX_ID, create_node_ptr(2, MAX_ID), create_node_ptr(2, MAX_ID))
         << create_node(0, MAX_ID, create_node_ptr(1, MAX_ID), sink_T)
        ;
    }

    // { {0}, {2}, {0,3}, {2,4} }
    /*
                    1         ---- x0
                   / \
                   |  \       ---- x1
                   |   \
                   2   |      ---- x2
                  / \  |
                  F |  3      ---- x3
                    | / \
                    4 T T     ---- x4
                   / \
                   T T
    */
    node_file zdd_2;
    {
      node_writer nw(zdd_2);
      nw << create_node(4, MAX_ID, sink_T, sink_T)
         << create_node(3, MAX_ID, sink_T, sink_T)
         << create_node(2, MAX_ID, sink_F, create_node_ptr(4, MAX_ID))
         << create_node(0, MAX_ID, create_node_ptr(2, MAX_ID), create_node_ptr(3, MAX_ID))
        ;
    }

    // { {0}, {2}, {1,2}, {0,2} }
    /*
                    1      ---- x0
                   / \
                   2 |     ---- x1
                  / \|
                  3  4     ---- x2
                 / \/ \
                 F T  T
    */
    node_file zdd_3;
    {
      node_writer nw(zdd_3);
      nw << create_node(2, MAX_ID,   sink_T, sink_T)
         << create_node(2, MAX_ID-1, sink_F, sink_T)
         << create_node(1, MAX_ID,   create_node_ptr(2, MAX_ID-1), create_node_ptr(2, MAX_ID))
         << create_node(0, MAX_ID,   create_node_ptr(1, MAX_ID), create_node_ptr(2, MAX_ID))
        ;
    }

    // { {4}, {0,2}, {0,4}, {2,4}, {0,2,4} }
    /*
                    _1_      ---- x0
                   /   \
                   2   3     ---- x2
                   \\ / \
                     4  5    ---- x4
                    / \/ \
                    F  T T
    */
    node_file zdd_4;
    {
      node_writer nw(zdd_4);
      nw << create_node(4, MAX_ID,   sink_T, sink_T)
         << create_node(4, MAX_ID-1, sink_F, sink_T)
         << create_node(2, MAX_ID,   create_node_ptr(4, MAX_ID-1), create_node_ptr(4, MAX_ID))
         << create_node(2, MAX_ID-1, create_node_ptr(4, MAX_ID-1), create_node_ptr(4, MAX_ID-1))
         << create_node(0, MAX_ID,   create_node_ptr(2, MAX_ID-1), create_node_ptr(2, MAX_ID))
        ;
    }

    it("computes with dom = Ø to be { Ø } for non-empty input [zdd_1] [const &]", [&](){
      label_file dom;

      zdd out = zdd_project(zdd_1, dom);

      node_test_stream out_nodes(out);
      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));

      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream<node_t, NODE_FILE_COUNT> ms(out);
      AssertThat(ms.can_pull(), Is().False());
    });

    it("computes with dom = Ø to be { Ø } for non-empty input [zdd_2] [&&]", [&](){
      label_file dom;

      zdd out = zdd_project(zdd(zdd_2), dom);

      node_test_stream out_nodes(out);
      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));

      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream<node_t, NODE_FILE_COUNT> ms(out);
      AssertThat(ms.can_pull(), Is().False());
    });

    it("computes with dom = Ø to be { Ø } for non-empty input [zdd_3] [const &]", [&](){
      label_file dom;

      zdd out = zdd_project(zdd_3, dom);

      node_test_stream out_nodes(out);
      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));

      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream<node_t, NODE_FILE_COUNT> ms(out);
      AssertThat(ms.can_pull(), Is().False());
    });

    it("computes with disjoint dom to be { Ø } [zdd_2] [const &]", [&](){
      label_file dom;
      { label_writer lw(dom);
        lw << 1;
      }

      zdd out = zdd_project(zdd_2, dom);

      node_test_stream out_nodes(out);
      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));

      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream<node_t, NODE_FILE_COUNT> ms(out);
      AssertThat(ms.can_pull(), Is().False());
    });

    it("computes with disjoint dom to be { Ø } [zdd_3] [&&]", [&](){
      label_file dom;
      { label_writer lw(dom);
        lw << 3 << 4 << 5;
      }

      zdd out = zdd_project(zdd(zdd_3), dom);

      node_test_stream out_nodes(out);
      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(create_sink(true)));

      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream<node_t, NODE_FILE_COUNT> ms(out);
      AssertThat(ms.can_pull(), Is().False());
    });

    // TODO: Shortcut on nothing to doa

    //////////////////////
    // Non-sink general case
    it("computes zdd_1 with dom = {2,3,4} [const &]", [&](){
      label_file dom;
      { label_writer lw(dom);
        lw << 2 << 3 << 4;
      }

      /* Expected: { Ø, {2}, {3}, {3,4} }

                           1    ---- x2
                          / \
                          2 T   ---- x3
                         / \
                         T 3    ---- x4
                          / \
                          T T
      */

      zdd out = zdd_project(zdd_1, dom);

      node_test_stream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(create_node(4, MAX_ID,
                                                            sink_T,
                                                            sink_T)));

      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(create_node(3, MAX_ID,
                                                            sink_T,
                                                            create_node_ptr(4, MAX_ID))));

      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(create_node(2, MAX_ID,
                                                            create_node_ptr(3, MAX_ID),
                                                            sink_T)));

      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream<node_t, NODE_FILE_COUNT> ms(out);

      AssertThat(ms.can_pull(), Is().True());
      AssertThat(ms.pull(), Is().EqualTo(create_level_info(4,1u)));


      AssertThat(ms.can_pull(), Is().True());
      AssertThat(ms.pull(), Is().EqualTo(create_level_info(3,1u)));

      AssertThat(ms.can_pull(), Is().True());
      AssertThat(ms.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(ms.can_pull(), Is().False());
    });

    it("computes zdd_2 with dom = {2,3,4} [&&]", [&](){
      label_file dom;
      { label_writer lw(dom);
        lw << 2 << 3 << 4;
      }

      /* Expected: { Ø, {2}, {3}, {2,4} }

                        1      ---- x2
                       / \
                       2  \    ---- x3
                      / \ |
                      T T 3    ---- x4
                         / \
                         T T

      */

      zdd out = zdd_project(zdd_2, dom);

      node_test_stream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(create_node(4, MAX_ID,
                                                            sink_T,
                                                            sink_T)));

      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(create_node(3, MAX_ID,
                                                            sink_T,
                                                            sink_T)));

      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(create_node(2, MAX_ID,
                                                            create_node_ptr(3, MAX_ID),
                                                            create_node_ptr(4, MAX_ID))));

      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream<node_t, NODE_FILE_COUNT> ms(out);

      AssertThat(ms.can_pull(), Is().True());
      AssertThat(ms.pull(), Is().EqualTo(create_level_info(4,1u)));


      AssertThat(ms.can_pull(), Is().True());
      AssertThat(ms.pull(), Is().EqualTo(create_level_info(3,1u)));

      AssertThat(ms.can_pull(), Is().True());
      AssertThat(ms.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(ms.can_pull(), Is().False());
    });

    it("computes zdd_3 with dom = {0,2,4}", [&](){
      label_file dom;
      { label_writer lw(dom);
        lw << 0 << 2 << 4;
      }

      /* Expected: { {0}, {2}, {0,2} }

                           1    ---- x0
                          / \
                          | |   ---- x1
                          \ /
                           2    ---- x2
                          / \
                          T T
      */

      zdd out = zdd_project(zdd_3, dom);

      node_test_stream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(create_node(2, MAX_ID,
                                                            sink_T,
                                                            sink_T)));

      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                            create_node_ptr(2, MAX_ID),
                                                            create_node_ptr(2, MAX_ID))));

      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream<node_t, NODE_FILE_COUNT> ms(out);

      AssertThat(ms.can_pull(), Is().True());
      AssertThat(ms.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(ms.can_pull(), Is().True());
      AssertThat(ms.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(ms.can_pull(), Is().False());
    });

    it("computes zdd_4 with dom = {0,4}", [&](){
      label_file dom;
      { label_writer lw(dom);
        lw << 0 << 4;
      }

      /* Expected: { {0}, {4}, {0,4} }

                         1     ---- x0
                        / \
                       /   \   ---- x2
                       |   |
                       2   3   ---- x4
                      / \ / \
                      F T T T
      */

      zdd out = zdd_project(zdd_4, dom);

      node_test_stream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(create_node(4, MAX_ID,
                                                            sink_T,
                                                            sink_T)));

      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(create_node(4, MAX_ID-1,
                                                            sink_F,
                                                            sink_T)));

      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(create_node(0, MAX_ID,
                                                            create_node_ptr(4, MAX_ID-1),
                                                            create_node_ptr(4, MAX_ID))));

      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream<node_t, NODE_FILE_COUNT> ms(out);

      AssertThat(ms.can_pull(), Is().True());
      AssertThat(ms.pull(), Is().EqualTo(create_level_info(4,2u)));

      AssertThat(ms.can_pull(), Is().True());
      AssertThat(ms.pull(), Is().EqualTo(create_level_info(0,1u)));

      AssertThat(ms.can_pull(), Is().False());
    });

    it("computes zdd_4 with dom = {2,4}", [&](){
      label_file dom;
      { label_writer lw(dom);
        lw << 2 << 4;
      }

      /* Expected: { {2}, {4}, {2,4} }

                         1     ---- x2
                         ||
                         2     ---- x4
                        / \
                        F T
      */

      zdd out = zdd_project(zdd_4, dom);

      node_test_stream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(create_node(4, MAX_ID,
                                                            sink_F,
                                                            sink_T)));

      AssertThat(out_nodes.can_pull(), Is().True());
      AssertThat(out_nodes.pull(), Is().EqualTo(create_node(2, MAX_ID,
                                                            create_node_ptr(4, MAX_ID),
                                                            create_node_ptr(4, MAX_ID))));

      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_stream<node_t, NODE_FILE_COUNT> ms(out);

      AssertThat(ms.can_pull(), Is().True());
      AssertThat(ms.pull(), Is().EqualTo(create_level_info(4,1u)));

      AssertThat(ms.can_pull(), Is().True());
      AssertThat(ms.pull(), Is().EqualTo(create_level_info(2,1u)));

      AssertThat(ms.can_pull(), Is().False());
    });
  });
 });
