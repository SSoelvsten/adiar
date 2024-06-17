#include "../../test.h"

go_bandit([]() {
  describe("adiar/zdd/zdd.h", [&]() {
    shared_levelized_file<zdd::node_type> x0_nf;
    {
      node_writer nw_0(x0_nf);
      nw_0 << node(0, node::max_id, ptr_uint64(false), ptr_uint64(true));
    }

    zdd x0(x0_nf);

    shared_levelized_file<zdd::node_type> x1_nf;
    {
      node_writer nw_1(x1_nf);
      nw_1 << node(1, node::max_id, ptr_uint64(false), ptr_uint64(true));
    }

    zdd x1(x1_nf);

    shared_levelized_file<zdd::node_type> x0_or_x1_nf;
    {
      node_writer nw_01(x0_or_x1_nf);

      nw_01 << node(1, node::max_id, ptr_uint64(false), ptr_uint64(true));

      nw_01 << node(0, node::max_id, ptr_uint64(1, ptr_uint64::max_id), ptr_uint64(true));
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
      it("copy-constructs values from zdd", [&]() {
        __zdd t1 = x0_or_x1;
        AssertThat(t1.has<shared_levelized_file<zdd::node_type>>(), Is().True());
        AssertThat(t1.get<shared_levelized_file<zdd::node_type>>(), Is().EqualTo(x0_or_x1_nf));
        AssertThat(t1._negate, Is().False());
      });

      it("copy-constructs values from __zdd", [&]() {
        __zdd t1 = x0_or_x1;
        __zdd t2 = t1;
        AssertThat(t2.has<shared_levelized_file<zdd::node_type>>(), Is().True());
        AssertThat(t2.get<shared_levelized_file<zdd::node_type>>(), Is().EqualTo(x0_or_x1_nf));
        AssertThat(t2._negate, Is().False());
      });

      it("copy-constructs values from shared_levelized_file<zdd::node_type>", [&]() {
        __zdd t1 = x0_or_x1;
        AssertThat(t1.has<shared_levelized_file<zdd::node_type>>(), Is().True());
        AssertThat(t1.get<shared_levelized_file<zdd::node_type>>(), Is().EqualTo(x0_or_x1_nf));
        AssertThat(t1._negate, Is().False());
      });

      __zdd::shared_arc_file_type af;

      {
        arc_writer aw(af);
        aw.push_internal(arc{ ptr_uint64(0, 0), false, ptr_uint64(1, 0) });

        aw.push_terminal(arc{ ptr_uint64(0, 0), true, ptr_uint64(true) });
        aw.push_terminal(arc{ ptr_uint64(1, 0), false, ptr_uint64(false) });
        aw.push_terminal(arc{ ptr_uint64(1, 0), true, ptr_uint64(true) });

        aw.push(level_info(0, 1u));
        aw.push(level_info(1, 1u));
      }

      af->max_1level_cut = 1;

      it("copy-constructs values from __zdd::shared_arc_file_type", [&]() {
        __zdd t1 = __zdd(af, exec_policy::access::Random_Access);
        AssertThat(t1.has<__zdd::shared_arc_file_type>(), Is().True());
        AssertThat(t1.get<__zdd::shared_arc_file_type>(), Is().EqualTo(af));
        AssertThat(t1._negate, Is().False());
        AssertThat(t1._policy, Is().EqualTo(exec_policy(exec_policy::access::Random_Access)));
      });

      it("reduces on copy construct to zdd with __zdd::shared_arc_file_type", [&]() {
        zdd out = __zdd(af, exec_policy());
        AssertThat(out, Is().EqualTo(x0_or_x1));
      });
    });

    describe("operators", [&]() {
      describe("==, !=", [&]() {
        it("rejects Ø == {Ø}", [&]() { AssertThat(terminal_F, Is().Not().EqualTo(terminal_T)); });

        it("accepts {{0}} == {{0}} (different files)", [&]() {
          shared_levelized_file<zdd::node_type> other_nf;
          {
            node_writer nw(other_nf);
            nw << node(0, node::max_id, ptr_uint64(false), ptr_uint64(true));
          }
          zdd other(other_nf);

          AssertThat(x0, Is().EqualTo(other));
        });

        it("rejects {{0}} == {{1}}", [&]() { AssertThat(x0, Is().Not().EqualTo(x1)); });

        it("rejects {{0}, {1}} == {{0}}", [&]() { AssertThat(x0_or_x1, Is().Not().EqualTo(x0)); });
      });

      describe("~, &, |", [&]() {
        it("computes ~{{0},{1}} == {Ø,{0,1}} with {0,1} domain", [&]() {
          const std::vector<int> dom = { 0, 1 };
          domain_set(dom.begin(), dom.end());

          shared_levelized_file<zdd::node_type> expected;
          {
            node_writer nw(expected);
            nw << node(1, node::max_id, ptr_uint64(false), ptr_uint64(true))
               << node(0, node::max_id, ptr_uint64(true), ptr_uint64(1, ptr_uint64::max_id));
          }
          AssertThat(~x0_or_x1 == zdd(expected), Is().True());
        });

        it("computes {{0}} | {{1}} == {{0}, {1}}",
           [&]() { AssertThat((x0 | x1) == x0_or_x1, Is().True()); });

        it("accumulates with '|=(zdd&)' operator", [&]() {
          zdd A = terminal_F;
          A |= x0;
          AssertThat(A, Is().EqualTo(x0));

          A |= x1;
          AssertThat(A, Is().EqualTo(x0_or_x1));
        });

        it("computes with __zdd&& operators [|]", [&]() {
          const zdd A = (x0_or_x1 | (x0 | x1)) | x0;
          AssertThat(A, Is().EqualTo(x0_or_x1));
        });

        it("computes {{0}} & {{0},{1}} == {{0}}",
           [&]() { AssertThat((x0 & x0_or_x1) == x0, Is().True()); });

        it("accumulates with '&=(zdd&)' operator", [&]() {
          zdd A = x0_or_x1;
          A &= x0;
          AssertThat(A, Is().EqualTo(x0));

          A &= x1;
          AssertThat(A, Is().EqualTo(terminal_F));
        });

        it("computes with __zdd&& operators [&]", [&]() {
          const zdd A = (x0 & (x0_or_x1 & x0)) & x1;
          AssertThat(A, Is().EqualTo(terminal_F));
        });

        it("accumulates with '|=(__zdd&&)' and '&=(__zdd&&)'", [&]() {
          zdd A = x0;

          A |= x0_or_x1 & x1;
          AssertThat(A, Is().EqualTo(x0_or_x1));

          A &= x0_or_x1 & x1;
          AssertThat(A, Is().EqualTo(x1));
        });

        it("computes with __zdd&& operators [|,&]", [&]() {
          const zdd A = ((x0_or_x1 | x0) | ((x0 & x1) | x1)) & x0;
          AssertThat(A, Is().EqualTo(x0));
        });

        it("computes with __zdd&& operators [~]", [&]() {
          const std::vector<int> dom = { 0, 1 };
          domain_set(dom.begin(), dom.end());

          const zdd A = ~(x0 | x1);
          AssertThat(~x0_or_x1 == A, Is().True());
        });
      });

      describe("-, +, *", [&]() {
        it("computes -{{0},{1}} == {Ø,{0,1}} [zdd&] with {0,1} domain", [&]() {
          const std::vector<int> dom = { 0, 1 };
          domain_set(dom.begin(), dom.end());

          shared_levelized_file<zdd::node_type> expected;
          {
            node_writer nw(expected);
            nw << node(1, node::max_id, ptr_uint64(false), ptr_uint64(true))
               << node(0, node::max_id, ptr_uint64(true), ptr_uint64(1, ptr_uint64::max_id));
          }
          AssertThat(-x0_or_x1 == zdd(expected), Is().True());
        });

        it("computes -{{0},{1}} == {Ø,{0,1}} [__zdd&&] with {0,1} domain", [&]() {
          const std::vector<int> dom = { 0, 1 };
          domain_set(dom.begin(), dom.end());

          shared_levelized_file<zdd::node_type> expected;
          {
            node_writer nw(expected);
            nw << node(1, node::max_id, ptr_uint64(false), ptr_uint64(true))
               << node(0, node::max_id, ptr_uint64(true), ptr_uint64(1, ptr_uint64::max_id));
          }
          AssertThat(-(x0 | x1) == zdd(expected), Is().True());
        });

        it("computes {{0},{1}} - {{0}} == {{1}}",
           [&]() { AssertThat((x0_or_x1 - x0) == x1, Is().True()); });

        it("accumulates with '-=(zdd&)'", [&]() {
          zdd A = x0_or_x1;

          A -= x0;
          AssertThat(A == x1, Is().True());

          A -= x1;
          AssertThat(A == terminal_F, Is().True());
        });

        it("accumulates with '-=(__zdd&&)'", [&]() {
          zdd A = x0_or_x1;

          A -= x0 | x1;
          AssertThat(A == terminal_F, Is().True());
        });

        it("computes +{{0}} == {{0}} [zdd&]", [&]() { AssertThat(+x0 == x0, Is().True()); });

        it("computes +{{0}} == {{0}} [__zdd&&]",
           [&]() { AssertThat(+(x0 & x0_or_x1) == x0, Is().True()); });

        it("computes {{0}} + {{1}} == {{0}, {1}}",
           [&]() { AssertThat((x0 + x1) == x0_or_x1, Is().True()); });

        it("accumulates with '+=(zdd&)' operator", [&]() {
          zdd A = terminal_F;
          A += x0;
          AssertThat(A, Is().EqualTo(x0));

          A += x1;
          AssertThat(A, Is().EqualTo(x0_or_x1));
        });

        it("computes with __zdd&& operators [+]", [&]() {
          const zdd A = (x0_or_x1 + (x0 + x1)) + x0;
          AssertThat(A, Is().EqualTo(x0_or_x1));
        });

        it("computes {{0}} * {{0},{1}} == {{0}}",
           [&]() { AssertThat((x0 * x0_or_x1) == x0, Is().True()); });

        it("accumulates with '*=(zdd&)' operator", [&]() {
          zdd A = x0_or_x1;
          A *= x0;
          AssertThat(A, Is().EqualTo(x0));

          A *= x1;
          AssertThat(A, Is().EqualTo(terminal_F));
        });

        it("computes with __zdd&& operators [*]", [&]() {
          const zdd A = (x0 * (x0_or_x1 & x0)) & x1;
          AssertThat(A, Is().EqualTo(terminal_F));
        });

        it("accumulates with '+=(__zdd&&)' and '*=(__zdd&&)'", [&]() {
          zdd A = x0;

          A += x0_or_x1 & x1;
          AssertThat(A, Is().EqualTo(x0_or_x1));

          A *= x0_or_x1 & x1;
          AssertThat(A, Is().EqualTo(x1));
        });

        it("computes with __zdd&& operators [+,*]", [&]() {
          const zdd A = ((x0_or_x1 + x0) + ((x0 * x1) + x1)) * x0;
          AssertThat(A, Is().EqualTo(x0));
        });

        it("computes with __zdd&& operators [-]", [&]() {
          const std::vector<int> dom = { 0, 1 };
          domain_set(dom.begin(), dom.end());

          const zdd A = -(x0 | x1);
          AssertThat(-x0_or_x1 == A, Is().True());
        });

        it("computes with __zdd&& operators [|,-,]", [&]() {
          const std::vector<int> dom = { 0, 1 };
          domain_set(dom.begin(), dom.end());

          const zdd out      = -(-(x0 | x1) - terminal_T);
          const zdd expected = x0 | x1 | terminal_T;
          AssertThat(expected, Is().EqualTo(out));
        });

        it("computes with __zdd&& operators [-,+,*]", [&]() {
          const zdd A = ((x0_or_x1 - x0) + ((x0 + x1) * (x0_or_x1 + x1))) - (x0_or_x1 - x0);
          AssertThat(x0, Is().EqualTo(A));
        });
      });

      describe("==, !=", [&]() {
        it("checks two derivations of same __bdd&& [==]",
           [&]() { AssertThat((x0 | x1) == ((x0_or_x1 - x1) | x1), Is().True()); });

        it("checks two derivations of same __bdd&& [!=]",
           [&]() { AssertThat((x0 | x1) != ((x0_or_x1 - x1) | x1), Is().False()); });

        it("checks two derivations of different __bdd&& [==]",
           [&]() { AssertThat((x0 | x1) == (x0_or_x1 - x1), Is().False()); });

        it("checks two derivations of different __bdd&& [!=]",
           [&]() { AssertThat((x0 | x1) != (x0_or_x1 - x1), Is().True()); });
      });

      describe("<, <=, >, >=", [&]() {
        it("checks {{0}} < {{0}, {1}}", [&]() {
          AssertThat(x0 < x0_or_x1, Is().True());
          AssertThat(x0 < (x0 | x1), Is().True());
        });

        it("checks {{0}} < {{1}}", [&]() { AssertThat(x0 < x1, Is().False()); });

        it("checks {{0}, {1}} < {{0}, {1}}", [&]() {
          AssertThat((x0 | x1) < x0_or_x1, Is().False());
          AssertThat(x0_or_x1 < (x0 | x1), Is().False());
          AssertThat((x0 | x1) < (x0 | x1), Is().False());
        });

        it("checks {{0}} <= {{0}, {1}}", [&]() {
          AssertThat(x0 <= x0_or_x1, Is().True());
          AssertThat(x0 <= (x0 | x1), Is().True());
        });

        it("checks {{0}, {1}} <= {{0}, {1}}", [&]() {
          AssertThat((x0 | x1) <= x0_or_x1, Is().True());
          AssertThat((x0 | x1) <= (x0 | x1), Is().True());
        });

        it("checks {{0}} <= {{1}}", [&]() { AssertThat(x0 <= x1, Is().False()); });

        it("checks {{0}, {1}} > {{0}}", [&]() { AssertThat(x0_or_x1 > x0, Is().True()); });

        it("checks {{0}} > {{1}}", [&]() { AssertThat(x0 > x1, Is().False()); });

        it("checks {{0}, {1}} > {{0}, {1}}", [&]() {
          AssertThat((x0 | x1) > x0_or_x1, Is().False());
          AssertThat(x0_or_x1 > (x0 | x1), Is().False());
          AssertThat((x0 | x1) > (x0 | x1), Is().False());
        });

        it("checks {{0}, {1}} >= {{0}}", [&]() { AssertThat(x0_or_x1 >= x0, Is().True()); });

        it("checks {{0}, {1}} >= {{0}, {1}}", [&]() {
          AssertThat((x0 | x1) >= x0_or_x1, Is().True());
          AssertThat(x0_or_x1 >= (x0 | x1), Is().True());
          AssertThat((x0 | x1) >= (x0 | x1), Is().True());
        });

        it("checks {{0}} >= {{1}}", [&]() {
          AssertThat(x0 >= x1, Is().False());
          AssertThat(x0 >= (x0_or_x1 & x1), Is().False());
          AssertThat((x0_or_x1 & x0) >= x1, Is().False());
          AssertThat((x0_or_x1 & x0) >= (x0_or_x1 & x1), Is().False());
        });
      });
    });

    it("should copy-construct shared_levelized_file<zdd::node_type> and negation back to zdd",
       [&]() {
         zdd t2 = zdd(__zdd(x0_or_x1));
         AssertThat(t2.file_ptr(), Is().EqualTo(x0_or_x1_nf));
         AssertThat(t2.is_negated(), Is().False());
       });
  });
});
