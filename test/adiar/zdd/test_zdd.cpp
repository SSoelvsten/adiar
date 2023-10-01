#include "../../test.h"

go_bandit([]() {
  describe("adiar/zdd/zdd.h", [&]() {
    shared_levelized_file<zdd::node_type> x0_nf;
    {
      node_writer nw_0(x0_nf);
      nw_0 << node(0, node::max_id,
                          ptr_uint64(false),
                          ptr_uint64(true));
    }

    zdd x0(x0_nf);

    shared_levelized_file<zdd::node_type> x1_nf;
    {
      node_writer nw_1(x1_nf);
      nw_1 << node(1, node::max_id,
                          ptr_uint64(false),
                          ptr_uint64(true));
    }

    zdd x1(x1_nf);

    shared_levelized_file<zdd::node_type> x0_or_x1_nf;
    {
      node_writer nw_01(x0_or_x1_nf);

      nw_01 << node(1, node::max_id,
                           ptr_uint64(false),
                           ptr_uint64(true));

      nw_01 << node(0, node::max_id,
                           ptr_uint64(1, ptr_uint64::max_id),
                           ptr_uint64(true));
    }

    zdd x0_or_x1(x0_or_x1_nf);

    shared_levelized_file<zdd::node_type> terminal_T_nf;
    {
      node_writer nw_T(terminal_T_nf);
      nw_T << node(true);
    }

    zdd terminal_T(terminal_T_nf);

    shared_levelized_file<zdd::node_type> terminal_F_nf;
    {
      node_writer nw_F(terminal_F_nf);
      nw_F << node(false);
    }

    zdd terminal_F(terminal_F_nf);

    describe("__zdd class", [&]() {
      it("should copy-construct values from zdd", [&]() {
        __zdd t1 = x0_or_x1;
        AssertThat(t1.has<shared_levelized_file<zdd::node_type>>(), Is().True());
        AssertThat(t1.get<shared_levelized_file<zdd::node_type>>(), Is().EqualTo(x0_or_x1_nf));
        AssertThat(t1.negate, Is().False());
      });

      it("should copy-construct values from __zdd", [&]() {
        __zdd t1 = x0_or_x1;
        __zdd t2 = t1;
        AssertThat(t2.has<shared_levelized_file<zdd::node_type>>(), Is().True());
        AssertThat(t2.get<shared_levelized_file<zdd::node_type>>(), Is().EqualTo(x0_or_x1_nf));
        AssertThat(t2.negate, Is().False());
      });

      it("should copy-construct values from shared_levelized_file<zdd::node_type>", [&]() {
        __zdd t1 = x0_or_x1;
        AssertThat(t1.has<shared_levelized_file<zdd::node_type>>(), Is().True());
        AssertThat(t1.get<shared_levelized_file<zdd::node_type>>(), Is().EqualTo(x0_or_x1_nf));
        AssertThat(t1.negate, Is().False());
      });

      __zdd::shared_arc_file_type af;

      {
        arc_writer aw(af);
        aw.push_internal(arc {ptr_uint64(0,0), false, ptr_uint64(1,0)});

        aw.push_terminal(arc {ptr_uint64(0,0), true,  ptr_uint64(true)});
        aw.push_terminal(arc {ptr_uint64(1,0), false, ptr_uint64(false)});
        aw.push_terminal(arc {ptr_uint64(1,0), true,  ptr_uint64(true)});

        aw.push(level_info(0,1u));
        aw.push(level_info(1,1u));
      }

      af->max_1level_cut = 1;

      it("should copy-construct values from __zdd::shared_arc_file_type", [&]() {
        __zdd t1 = __zdd(af, exec_policy::access::Random_Access);
        AssertThat(t1.has<__zdd::shared_arc_file_type>(), Is().True());
        AssertThat(t1.get<__zdd::shared_arc_file_type>(), Is().EqualTo(af));
        AssertThat(t1.negate, Is().False());
        AssertThat(t1._policy, Is().EqualTo(exec_policy(exec_policy::access::Random_Access)));
      });

      it("should reduce on copy construct to zdd with __zdd::shared_arc_file_type", [&]() {
        zdd out = __zdd(af, exec_policy());
        AssertThat(out, Is().EqualTo(x0_or_x1));
      });
    });

    describe("operators", [&]() {
      it("should reject Ø == {Ø}", [&]() {
        AssertThat(terminal_F, Is().Not().EqualTo(terminal_T));
      });

      it("should accept {{0}} == {{0}} (different files)", [&]() {
        shared_levelized_file<zdd::node_type> x0_nf_2;

        { node_writer nw_0(x0_nf_2);
          nw_0 << node(0, node::max_id,
                              ptr_uint64(false),
                              ptr_uint64(true));
        }

        zdd x0_2(x0_nf);

        AssertThat(x0, Is().EqualTo(x0_2));
      });

      it("should compute {{0}} /\\ {{1}} == {{0}, {1}}", [&]() {
        AssertThat((x0 | x1) == x0_or_x1, Is().True());
      });

      it("should compute {{0}} \\/ {{0},{1}} == {{0}}", [&]() {
        AssertThat((x0 & x0_or_x1) == x0, Is().True());
      });

      it("should compute {{0},{1}} \\ {{0} == {{0}}", [&]() {
        AssertThat((x0_or_x1 - x0) == x1, Is().True());
      });

      it("should compute {{0},{1}}' == {Ø,{0,1}} with dom {0,1}", [&]() {
        adiar::shared_file<zdd::label_type> dom;
        {
          label_writer lw(dom);
          lw << 0 << 1;
        }

        domain_set(dom);

        shared_levelized_file<zdd::node_type> expected;
        {
          node_writer nw(expected);
          nw << node(1, node::max_id, ptr_uint64(false), ptr_uint64(true))
             << node(0, node::max_id, ptr_uint64(true), ptr_uint64(1, ptr_uint64::max_id));
        }

        AssertThat(~x0_or_x1 == zdd(expected), Is().True());
      });

      it("should compute with __zdd&& operators [|,&,-]", [&]() {
        zdd out = ((x0_or_x1 - x0) | ((x0 | x1) & (x0_or_x1 - x1))) - (x0_or_x1 - x0);
        AssertThat(x0, Is().EqualTo(out));
      });

      it("should compute with __zdd&& operators [|,~,-,]", [&]() {
          adiar::shared_file<zdd::label_type> dom;
          {
            label_writer lw(dom);
            lw << 0 << 1;
          }

          domain_set(dom);

          zdd out = ~(~(x0 | x1) - terminal_T);
          zdd expected = x0 | x1 | terminal_T;
          AssertThat(expected, Is().EqualTo(out));
        });

      it("should ?= __zdd&&", [&]() {
        zdd out = x0_or_x1;
        out -= x0_or_x1 & x1;
        AssertThat(out, Is().EqualTo(x0));

        out |= x0_or_x1 & x1;
        AssertThat(out, Is().EqualTo(x0_or_x1));

        out &= x0_or_x1 & x1;
        AssertThat(out, Is().EqualTo(x1));
      });

      it("should check two derivations of same __bdd&&", [&]() {
        AssertThat((x0 | x1) == ((x0_or_x1 - x1) | x1), Is().True());
        AssertThat((x0 | x1) != ((x0_or_x1 - x1) | x1), Is().False());
      });

      it("should check two derivations of different __bdd&&", [&]() {
        AssertThat((x0 | x1) == (x0_or_x1 - x1), Is().False());
        AssertThat((x0 | x1) != (x0_or_x1 - x1), Is().True());
      });
    });

    it("should copy-construct shared_levelized_file<zdd::node_type> and negation back to zdd", [&]() {
      zdd t2 = zdd(__zdd(x0_or_x1));
      AssertThat(t2.file_ptr(), Is().EqualTo(x0_or_x1_nf));
      AssertThat(t2.is_negated(), Is().False());
    });
  });
 });
