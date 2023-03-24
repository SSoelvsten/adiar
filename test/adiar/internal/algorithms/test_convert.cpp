#include "../../../test.h"

go_bandit([]() {
  describe("adiar/internal/algorithms/convert.h", []() {
    adiar::shared_file<node::label_t> dom_012;
    {
      label_writer w(dom_012);
      w << 0 << 1 << 2;
    }

    adiar::shared_file<node::label_t> dom_0123;
    {
      label_writer w(dom_0123);
      w << 0 << 1 << 2 << 3;
    }

    adiar::shared_file<node::label_t> dom_024;
    {
      label_writer w(dom_024);
      w << 0 << 2 << 4;
    }

    shared_levelized_file<dd::node_t> nf_F;
    {
      node_writer w(nf_F);
      w << node(false);
    }

    shared_levelized_file<dd::node_t> nf_T;
    {
      node_writer w(nf_T);
      w << node(true);
    }

    const ptr_uint64 terminal_F = ptr_uint64(false);
    const ptr_uint64 terminal_T = ptr_uint64(true);

    shared_levelized_file<dd::node_t> nf_x0;
    {
      node_writer nw(nf_x0);
      nw << node(0, node::MAX_ID, terminal_F, terminal_T);
    }

    shared_levelized_file<dd::node_t> nf_x1;
    {
      node_writer nw(nf_x1);
      nw << node(1, node::MAX_ID, terminal_F, terminal_T);
    }

    shared_levelized_file<dd::node_t> nf_x2;
    {
      node_writer nw(nf_x2);
      nw << node(2, node::MAX_ID, terminal_F, terminal_T);
    }

    zdd zdd_F(nf_F);
    zdd zdd_T(nf_T);

    zdd zdd_x0(nf_x0);
    zdd zdd_x1(nf_x1);
    zdd zdd_x2(nf_x2);

    shared_levelized_file<dd::node_t> nf_x0_null;
    {
      node_writer nw(nf_x0_null);
      nw << node(0, node::MAX_ID, terminal_T, terminal_T);
    }
    zdd zdd_x0_null(nf_x0_null);

    shared_levelized_file<dd::node_t> nf_x1_null;
    {
      node_writer nw(nf_x1_null);
      nw << node(1, node::MAX_ID, terminal_T, terminal_T);
    }
    zdd zdd_x1_null(nf_x1_null);

    shared_levelized_file<dd::node_t> nf_x2_null;
    {
      node_writer nw(nf_x2_null);
      nw << node(2, node::MAX_ID, terminal_T, terminal_T);
    }
    zdd zdd_x2_null(nf_x2_null);

    // Fig. 5 from Minato: "Zero-suppressed BDDs and their applications". This
    // is the ZDD version of Fig. 3 in the same paper.
    shared_levelized_file<dd::node_t> nf_minato_fig5;
    {
      const node n2 = node(1, node::MAX_ID, terminal_F, terminal_T);
      const node n1 = node(0, node::MAX_ID, n2.uid(), terminal_T);

      node_writer nw(nf_minato_fig5);
      nw << n2 << n1;
    }
    zdd zdd_minato_fig5(nf_minato_fig5);

    bdd bdd_F(nf_F);
    bdd bdd_T(nf_T);

    bdd bdd_x0(nf_x0);
    bdd bdd_x2(nf_x2);

    describe("bdd_from(const zdd&, const label_file&)", [&]() {
      it("returns F terminal on Ø with dom = Ø", [&]() {
        adiar::shared_file<node::label_t> dom_empty;
        __bdd out = bdd_from(zdd_F, dom_empty);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream ms(out);
        AssertThat(ms.can_pull(), Is().False());
      });

      it("returns T terminal on { Ø } with dom = Ø", [&]() {
        adiar::shared_file<node::label_t> dom_empty;
        __bdd out = bdd_from(zdd_T, dom_empty);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream ms(out);
        AssertThat(ms.can_pull(), Is().False());
      });

      it("returns F terminal on Ø with dom = { 0,1,2 }", [&]() {
        __bdd out = bdd_from(zdd_F, dom_012);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream ms(out);
        AssertThat(ms.can_pull(), Is().False());
      });

      it("returns check-false chain to T terminal on { Ø } with dom = { 0,1,2 }", [&]() {
        __bdd out = bdd_from(zdd_T, dom_012);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                       terminal_T,
                                                       terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                       ptr_uint64(2, ptr_uint64::MAX_ID),
                                                       terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                       ptr_uint64(1, ptr_uint64::MAX_ID),
                                                       terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream ms(out);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(ms.can_pull(), Is().False());
      });

      it("adds pre-root false-chain on { { 2 } } with dom = { 0,1,2 }", [&]() {
        __bdd out = bdd_from(zdd_x2, dom_012);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), true,  terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(levels.can_pull(), Is().False());
      });

      it("adds post-nodes false-chain on { { 0 } } with dom = { 0,1,2 }", [&]() {
        __bdd out = bdd_from(zdd_x0, dom_012);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), true,  terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(levels.can_pull(), Is().False());
      });

      it("kills root and returns T terminal on { Ø, { 0 } } with dom = { 0 }", [&]() {
        adiar::shared_file<node::label_t> dom_0;
        {
          label_writer lw(dom_0);
          lw << 0;
        }

        __bdd out = bdd_from(zdd_x0_null, dom_0);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream ms(out);
        AssertThat(ms.can_pull(), Is().False());
      });

      it("kills root on { Ø, { 0 } } with dom = { 0,1,2 }", [&]() {
        __bdd out = bdd_from(zdd_x0_null, dom_012);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), true,  terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(levels.can_pull(), Is().False());
      });

      it("kills root and bridges over it on { Ø, { 1 } } with dom = { 0,1,2 }", [&]() {
        __bdd out = bdd_from(zdd_x1_null, dom_012);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), true,  terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(levels.can_pull(), Is().False());
      });

      it("kills root and bridges over it on { Ø, { 2 } } with dom = { 0,2,4 }", [&]() {
        __bdd out = bdd_from(zdd_x2_null, dom_024);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(4,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(4,0), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(4,0), true,  terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(4,1u)));

        AssertThat(levels.can_pull(), Is().False());
      });

      it("collapses to T terminal on pow(dom) with dom = { 0,1,2 }", [&]() {
        shared_levelized_file<bdd::node_t> nf_pow_dom;
        {
          const node n2 = node(2, node::MAX_ID, terminal_T, terminal_T);
          const node n1 = node(1, node::MAX_ID, n2.uid(), n2.uid());
          const node n0 = node(0, node::MAX_ID, n1.uid(), n1.uid());

          node_writer nw(nf_pow_dom);
          nw << n2 << n1 << n0;
        }
        zdd zdd_pow(nf_pow_dom);

        __bdd out = bdd_from(zdd_pow, dom_012);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream ms(out);
        AssertThat(ms.can_pull(), Is().False());
      });

      it("collapses to T terminal on pow(dom) with dom = { 0,2,4 }", [&]() {
        shared_levelized_file<zdd::node_t> nf_pow_dom;
        {
          const node n2 = node(4, node::MAX_ID, terminal_T, terminal_T);
          const node n1 = node(2, node::MAX_ID, n2.uid(), n2.uid());
          const node n0 = node(0, node::MAX_ID, n1.uid(), n1.uid());

          node_writer nw(nf_pow_dom);
          nw << n2 << n1 << n0;
        }
        zdd zdd_pow(nf_pow_dom);

        __bdd out = bdd_from(zdd_pow, dom_024);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream ms(out);
        AssertThat(ms.can_pull(), Is().False());
      });

      it("bridges internal arcs on { { 0,2 }, { 1,2 } { 0,1,2 } } with dom = { 0,1,2 } ", [&]() {
        /*
        //          _1_     ---- x0
        //         /   \
        //         2   3    ---- x1
        //        / \ //
        //        F  4      ---- x2
        //          / \
        //          F T
        */

        shared_levelized_file<zdd::node_t> nf;
        {
          const node n4 = node(2, node::MAX_ID,   terminal_F, terminal_T);
          const node n3 = node(1, node::MAX_ID,   n4.uid(), n4.uid());
          const node n2 = node(1, node::MAX_ID-1, terminal_F, n4.uid());
          const node n1 = node(0, node::MAX_ID,   n2.uid(), n3.uid());

          node_writer nw(nf);
          nw << n4 << n3 << n2 << n1;
        }
        zdd in(nf);

        __bdd out = bdd_from(in, dom_012);
        /*
        //        _1_     ---- x0
        //       /   \
        //       2   /    ---- x1
        //      / \ /
        //      F  4      ---- x2
        //        / \
        //        F T
        */

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), true,  terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(levels.can_pull(), Is().False());
      });

      // Minato examples
      it("converts [Minato] Fig. 5 into Fig. 3 with dom = { 0,1,2 } ", [&]() {
        __bdd out = bdd_from(zdd_minato_fig5, dom_012);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(1,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,1), true,  terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), true,  terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,2u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(levels.can_pull(), Is().False());
      });

      it("converts [Minato] Fig. 5 into Fig. 3 with dom = { 0,1,2,3 }", [&]() {
        __bdd out = bdd_from(zdd_minato_fig5, dom_0123);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(1,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(3,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,1), true,  terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), true,  terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), true,  terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,2u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3,1u)));

        AssertThat(levels.can_pull(), Is().False());
      });

      it("converts [Minato] Fig. 11 (dom = { 0,1,2,3 })", [&]() {
        shared_levelized_file<zdd::node_t> nf;
        {
          const node n3 = node(3, node::MAX_ID, terminal_T, terminal_T);
          const node n2 = node(2, node::MAX_ID, n3.uid(), n3.uid());
          const node n1 = node(1, node::MAX_ID, n2.uid(), terminal_T);

          node_writer nw(nf);
          nw << n3 << n2 << n1;
        }

        __bdd out = bdd_from(nf, dom_0123);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(3,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // produced out-of-order
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), true,  terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), true,  terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3,1u)));

        AssertThat(levels.can_pull(), Is().False());
      });

      it("converts [Minato] Fig. 15 (dom = { 0,1,2 })", [&]() {
        shared_levelized_file<zdd::node_t> nf;
        {
          const node n3 = node(2, node::MAX_ID, terminal_F, terminal_T);
          const node n2 = node(1, node::MAX_ID, n3.uid(), terminal_T);
          const node n1 = node(0, node::MAX_ID, n3.uid(), n2.uid());

          node_writer nw(nf);
          nw << n3 << n2 << n1;
        }

        __bdd out = bdd_from(nf, dom_012);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True()); // (1) -> (2)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (1) -- * -> (3)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (2) -> (3)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // * -> (3)
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (2) -- * -> T
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  ptr_uint64(2,1) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,1), true,  terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), true,  terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), true,  terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,2u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,2u)));

        AssertThat(levels.can_pull(), Is().False());
      });

      // Other large cases
      it("converts { Ø, { 0,1 }, { 0,2 }, { 1,2 } } with dom = { 0,1,2 } only adding true-chain", [&]() {
        shared_levelized_file<zdd::node_t> nf_in;
        // In dom = { 0,1,2 }
        /*
        //      _1_      ---- x0
        //     /   \
        //     2   3     ---- x1
        //    / \ / \
        //    T  4  T    ---- x2
        //      / \
        //      F T
        */
        {
          const node n4 = node(2, node::MAX_ID,   terminal_F, terminal_T);
          const node n3 = node(1, node::MAX_ID,   n4.uid(), terminal_T);
          const node n2 = node(1, node::MAX_ID-1, terminal_T, n4.uid());
          const node n1 = node(0, node::MAX_ID,   n2.uid(), n3.uid());

          node_writer nw(nf_in);
          nw << n4 << n3 << n2 << n1;
        }

        zdd in(nf_in);

        __bdd out = bdd_from(in, dom_012);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(1,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(2,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,1), true,  ptr_uint64(2,1) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), true,  terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,1), true,  terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,2u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,2u)));

        AssertThat(levels.can_pull(), Is().False());
      });

      it("bridges over root and others, and creates pre and post chains", [&]() {
        adiar::shared_file<node::label_t> dom;
        {
          label_writer w(dom);
          w << 0 << 1 << 2 << 3 << 4 << 5 << 6;
        }

        shared_levelized_file<zdd::node_t> in;
        /*
        //                      ---- x0
        //
        //             1        ---- x1
        //            / \
        //            | |       ---- x2
        //            \ /
        //            _2_       ---- x3
        //           /   \
        //           3   4      ---- x4
        //          / \ / \
        //          5  6  T     ---- x5
        //         / \//
        //         F  7         ---- x6
        //           / \
        //           T T
        */
        {
          const node n7 = node(6, node::MAX_ID,   terminal_T, terminal_T);
          const node n6 = node(5, node::MAX_ID,   n7.uid(), n7.uid());
          const node n5 = node(5, node::MAX_ID-1, terminal_F, n7.uid());
          const node n4 = node(4, node::MAX_ID,   n6.uid(), terminal_T);
          const node n3 = node(4, node::MAX_ID-1, n5.uid(), n6.uid());
          const node n2 = node(3, node::MAX_ID,   n3.uid(), n4.uid());
          const node n1 = node(1, node::MAX_ID,   n2.uid(), n2.uid());

          node_writer w(in);
          w << n7 << n6 << n5 << n4 << n3 << n2 << n1;
        }

        __bdd out = bdd_from(in, dom);
        /*
        //              1       ---- x0
        //             / \
        //             | F      ---- x1
        //             |
        //            _2_       ---- x2
        //           /   \
        //          _3_  F      ---- x3
        //         /   \
        //         4   5        ---- x4
        //        / \ / \
        //        6 T T 7       ---- x5
        //       / \   / \
        //       F T   8 F      ---- x6
        //            / \
        //            T F
        */

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True()); // 2
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // 3
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(3,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // 4
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(3,0), false, ptr_uint64(4,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // 5
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(3,0), true,  ptr_uint64(4,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // 6
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(4,0), false, ptr_uint64(5,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // 7
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(4,1), true,  ptr_uint64(5,1) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // 8
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(5,1), false, ptr_uint64(6,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True()); // 1
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // 2
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), true,  terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // 4
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(4,0), true,  terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // 5
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(4,1), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // 6
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(5,0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(5,0), true,  terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // 7
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(5,1), true,  terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // 8
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(6,0), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(6,0), true,  terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(4,2u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(5,2u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(6,1u)));

        AssertThat(levels.can_pull(), Is().False());
      });
    });

    describe("bdd_from(const zdd&)", [&]() {
      it("returns check-false chain to T terminal on { Ø } with set dom = { 0,1,2 }", [&]() {
        adiar_set_domain(dom_012);

        __bdd out = bdd_from(zdd_T);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                       terminal_T,
                                                       terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                       ptr_uint64(2, ptr_uint64::MAX_ID),
                                                       terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                       ptr_uint64(1, ptr_uint64::MAX_ID),
                                                       terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream ms(out);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(ms.can_pull(), Is().False());
      });

      it("kills root and bridges over it on { Ø, { 2 } } with set dom = { 0,2,4 }", [&]() {
        adiar_set_domain(dom_024);

        __bdd out = bdd_from(zdd_x2_null);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(4,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(4,0), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(4,0), true,  terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(4,1u)));

        AssertThat(levels.can_pull(), Is().False());
      });
    });

    describe("zdd_from(const bdd&, const label_file&)", [&]() {
      it("returns Ø on F terminal with dom = Ø", [&]() {
        adiar::shared_file<node::label_t> dom_empty;
        __zdd out = zdd_from(bdd_F, dom_empty);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream ms(out);
        AssertThat(ms.can_pull(), Is().False());
      });

      it("returns { Ø } on T terminal with dom = Ø", [&]() {
        adiar::shared_file<node::label_t> dom_empty;
        __zdd out = zdd_from(bdd_T, dom_empty);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream ms(out);
        AssertThat(ms.can_pull(), Is().False());
      });

      it("returns Ø on F terminal with dom = { 0,1,2 }", [&]() {
        __zdd out = zdd_from(bdd_F, dom_012);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream ms(out);
        AssertThat(ms.can_pull(), Is().False());
      });

      it("returns pow(dom) on T terminal with dom = { 0,1,2 }", [&]() {
        __zdd out = zdd_from(bdd_T, dom_012);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                       terminal_T,
                                                       terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                       ptr_uint64(2, ptr_uint64::MAX_ID),
                                                       ptr_uint64(2, ptr_uint64::MAX_ID))));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                       ptr_uint64(1, ptr_uint64::MAX_ID),
                                                       ptr_uint64(1, ptr_uint64::MAX_ID))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream ms(out);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(ms.can_pull(), Is().False());
      });

      it("adds don't care chain before root on x2 terminal with dom = { 0,1,2 }", [&]() {
        __zdd out = zdd_from(bdd_x2, dom_012);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), true,  terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(levels.can_pull(), Is().False());
      });

      it("adds don't care chain after root on x0 terminal with dom = { 0,1,2 }", [&]() {
        __zdd out = zdd_from(bdd_x0, dom_012);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), true,  terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(levels.can_pull(), Is().False());
      });

      it("kills and bridges root on ~x1 with dom = { 0,1,2 }", [&]() {
        shared_levelized_file<bdd::node_t> nf;
        {
          node_writer nw(nf);
          nw << node(1, node::MAX_ID, terminal_T, terminal_F);
        }
        bdd in(nf);

        __zdd out = zdd_from(in, dom_012);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), true,  terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(levels.can_pull(), Is().False());
      });

      it("kills root into { Ø } on ~x0 into with dom = { 0 }", [&]() {
        adiar::shared_file<node::label_t> dom;
        {
          label_writer w(dom);
          w << 0;
        }

        shared_levelized_file<bdd::node_t> nf;
        {
          node_writer nw(nf);
          nw << node(0, node::MAX_ID, terminal_T, terminal_F);
        }
        bdd in(nf);

        __zdd out = zdd_from(in, dom);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream ms(out);
        AssertThat(ms.can_pull(), Is().False());
      });

      it("collapses false-chain into { Ø } on with dom = { 0,2,4 }", [&]() {
        shared_levelized_file<bdd::node_t> nf;
        {
          const node n3 = node(4, node::MAX_ID, terminal_T, terminal_F);
          const node n2 = node(2, node::MAX_ID, n3.uid(), terminal_F);
          const node n1 = node(0, node::MAX_ID, n2.uid(), terminal_F);

          node_writer nw(nf);
          nw << n3 << n2 << n1;
        }
        bdd in(nf);

        __zdd out = zdd_from(in, dom_024);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream ms(out);
        AssertThat(ms.can_pull(), Is().False());
      });

      // Minato examples
      it("converts [Minato] Fig. 3 into Fig. 5 with dom = { 0,1,2 } ", [&]() {
        shared_levelized_file<bdd::node_t> nf;
        {
          const node n4 = node(2, node::MAX_ID,   terminal_T, terminal_F);
          const node n3 = node(1, node::MAX_ID,   n4.uid(), terminal_F);
          const node n2 = node(1, node::MAX_ID-1, terminal_F, n4.uid());
          const node n1 = node(0, node::MAX_ID,   n2.uid(), n3.uid());

          node_writer nw(nf);
          nw << n4 << n3 << n2 << n1;
        }
        bdd in(nf);

        __zdd out = zdd_from(in, dom_012);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(levels.can_pull(), Is().False());
      });

      it("converts [Minato] Fig. 3 into Fig. 5 with dom = { 0,1,2,3 } ", [&]() {
        shared_levelized_file<bdd::node_t> nf;
        {
          const node n5 = node(3, node::MAX_ID,   terminal_T, terminal_F);
          const node n4 = node(2, node::MAX_ID,   n5.uid(), terminal_F);
          const node n3 = node(1, node::MAX_ID,   n4.uid(), terminal_F);
          const node n2 = node(1, node::MAX_ID-1, terminal_F, n4.uid());
          const node n1 = node(0, node::MAX_ID,   n2.uid(), n3.uid());

          node_writer nw(nf);
          nw << n5 << n4 << n3 << n2 << n1;
        }
        bdd in(nf);

        __zdd out = zdd_from(in, dom_0123);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(levels.can_pull(), Is().False());
      });

      it("converts [Minato] Fig. 11 with dom = { 0,1,2,3 } ", [&]() {
        shared_levelized_file<bdd::node_t> nf;
        {
          const node n4 = node(3, node::MAX_ID,   terminal_T, terminal_F);
          const node n3 = node(2, node::MAX_ID,   n4.uid(), terminal_F);
          const node n2 = node(1, node::MAX_ID-1, terminal_T, n3.uid());
          const node n1 = node(0, node::MAX_ID,   n2.uid(), terminal_F);

          node_writer nw(nf);
          nw << n4 << n3 << n2 << n1;
        }
        bdd in(nf);

        __zdd out = zdd_from(in, dom_0123);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,0), false, ptr_uint64(3,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(2,0), true,  ptr_uint64(3,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(3,0), true,  terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3,1u)));

        AssertThat(levels.can_pull(), Is().False());
      });

      it("converts [Minato] Fig. 15 with dom = { 0,1,2 } ", [&]() {
        shared_levelized_file<bdd::node_t> nf;
        {
          const node n5 = node(2, node::MAX_ID,   terminal_F, terminal_T);
          const node n4 = node(2, node::MAX_ID-1, terminal_T, terminal_F);
          const node n3 = node(1, node::MAX_ID,   n5.uid(), terminal_F);
          const node n2 = node(1, node::MAX_ID-1, n5.uid(), n4.uid());
          const node n1 = node(0, node::MAX_ID,   n3.uid(), n2.uid());

          node_writer nw(nf);
          nw << n5 << n4 << n3 << n2 << n1;
        }
        bdd in(nf);

        __zdd out = zdd_from(in, dom_012);

        arc_test_stream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), true,  ptr_uint64(1,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(0,0), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc { ptr_uint64(1,0), false, ptr_uint64(2,0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(1,0), true,  terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc { ptr_uint64(2,0), true,  terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_stream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(levels.can_pull(), Is().False());
      });
    });

    describe("zdd_from(const bdd&)", [&]() {
      it("returns pow(dom) on T terminal with set dom = { 0,1,2 }", [&]() {
        adiar_set_domain(dom_012);

        __zdd out = zdd_from(bdd_T);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::MAX_ID,
                                                       terminal_T,
                                                       terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::MAX_ID,
                                                       ptr_uint64(2, ptr_uint64::MAX_ID),
                                                       ptr_uint64(2, ptr_uint64::MAX_ID))));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::MAX_ID,
                                                       ptr_uint64(1, ptr_uint64::MAX_ID),
                                                       ptr_uint64(1, ptr_uint64::MAX_ID))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream ms(out);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2,1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1,1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0,1u)));

        AssertThat(ms.can_pull(), Is().False());
      });

      it("collapses false-chain into { Ø } on with set dom = { 0,2,4 }", [&]() {
        shared_levelized_file<bdd::node_t> nf;
        {
          const node n3 = node(4, node::MAX_ID, terminal_T, terminal_F);
          const node n2 = node(2, node::MAX_ID, n3.uid(), terminal_F);
          const node n1 = node(0, node::MAX_ID, n2.uid(), terminal_F);

          node_writer nw(nf);
          nw << n3 << n2 << n1;
        }
        bdd in(nf);

        adiar_set_domain(dom_024);

        __zdd out = zdd_from(in);

        node_test_stream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_stream ms(out);
        AssertThat(ms.can_pull(), Is().False());
      });
    });
  });
 });
