#include "../../test.h"

go_bandit([]() {
  describe("src/adiar/bdd.h", []() {
    shared_levelized_file<bdd::node_type> x0_nf;

    {
      node_writer nw(x0_nf);
      nw << node(0, node::max_id, ptr_uint64(false), ptr_uint64(true));
    }

    bdd x0(x0_nf);

    shared_levelized_file<bdd::node_type> x1_nf;

    {
      node_writer nw(x1_nf);
      nw << node(1, node::max_id, ptr_uint64(false), ptr_uint64(true));
    }

    bdd x1(x1_nf);

    shared_levelized_file<bdd::node_type> x0_and_x1_nf;

    {
      node_writer nw(x0_and_x1_nf);
      nw << node(1, node::max_id, ptr_uint64(false), ptr_uint64(true))
         << node(0, node::max_id, ptr_uint64(false), ptr_uint64(1, ptr_uint64::max_id));
    }

    bdd x0_and_x1(x0_and_x1_nf);
    bdd x0_nand_x1(x0_and_x1_nf, true);

    shared_levelized_file<bdd::node_type> terminal_T_nf;
    {
      node_writer nw(terminal_T_nf);
      nw << node(true);
    }

    bdd terminal_T(terminal_T_nf);

    shared_levelized_file<bdd::node_type> terminal_F_nf;
    {
      node_writer nw(terminal_F_nf);
      nw << node(false);
    }

    bdd terminal_F(terminal_F_nf);

    describe("__bdd", [&]() {
      it("should copy-construct values from bdd", [&]() {
        __bdd t1 = bdd(x0_and_x1);
        AssertThat(t1.has<shared_levelized_file<bdd::node_type>>(), Is().True());
        AssertThat(t1.get<__bdd::shared_node_file_type>(), Is().EqualTo(x0_and_x1_nf));
        AssertThat(t1.negate, Is().False());
      });

      it("should copy-construct values from negated bdd", [&]() {
        __bdd t2 = bdd(x0_nand_x1);
        AssertThat(t2.has<shared_levelized_file<bdd::node_type>>(), Is().True());
        AssertThat(t2.get<__bdd::shared_node_file_type>(), Is().EqualTo(x0_and_x1_nf));
        AssertThat(t2.negate, Is().True());
      });

      it("should copy-construct values from __bdd", [&]() {
        __bdd t1 = x0_and_x1;
        __bdd t2 = t1;
        AssertThat(t2.has<shared_levelized_file<bdd::node_type>>(), Is().True());
        AssertThat(t2.get<__bdd::shared_node_file_type>(), Is().EqualTo(x0_and_x1_nf));
        AssertThat(t2.negate, Is().False());
      });

      it("should copy-construct values from shared_levelized_file<bdd::node_type>", [&]() {
        __bdd t1 = x0_and_x1;
        AssertThat(t1.has<shared_levelized_file<bdd::node_type>>(), Is().True());
        AssertThat(t1.get<__bdd::shared_node_file_type>(), Is().EqualTo(x0_and_x1_nf));
        AssertThat(t1.negate, Is().False());
      });

      shared_levelized_file<arc> af;

      {
        arc_writer aw(af);
        aw.push_internal(arc{ ptr_uint64(0, 0), true, ptr_uint64(1, 0) });

        aw.push_terminal(arc{ ptr_uint64(0, 0), false, ptr_uint64(false) });
        aw.push_terminal(arc{ ptr_uint64(1, 0), false, ptr_uint64(true) });
        aw.push_terminal(arc{ ptr_uint64(1, 0), true, ptr_uint64(true) });

        aw.push(level_info(0, 1u));
        aw.push(level_info(1, 1u));
      }

      af->max_1level_cut = 1;

      it("should copy-construct values from shared_levelized_file<arc>", [&]() {
        __bdd t1(af, exec_policy::memory::Internal);
        AssertThat(t1.has<shared_levelized_file<arc>>(), Is().True());
        AssertThat(t1.get<__bdd::shared_arc_file_type>(), Is().EqualTo(af));
        AssertThat(t1.negate, Is().False());
        AssertThat(t1._policy, Is().EqualTo(exec_policy(exec_policy::memory::Internal)));
      });

      it("should reduce on copy construct to bdd with shared_levelized_file<arc>", [&]() {
        bdd out = __bdd(af, exec_policy());
        AssertThat(out, Is().EqualTo(x0));
      });
    });

    it("should copy-construct boolean 'true' value as a T terminal", [&]() {
      bdd t1 = true;
      AssertThat(t1, Is().EqualTo(terminal_T));
    });

    it("should copy-construct boolean 'false' value as an F terminal", [&]() {
      bdd t2 = false;
      AssertThat(t2, Is().EqualTo(terminal_F));
    });

    it("should copy-construct shared_levelized_file<bdd::node_type> and negation back to bdd",
       [&]() {
         bdd t2 = bdd(__bdd(x0_and_x1));
         AssertThat(t2.file_ptr(), Is().EqualTo(x0_and_x1_nf));
         AssertThat(t2.is_negated(), Is().False());
       });

    describe("operator overloading", [&]() {
      describe("==, !=", [&]() {
        it("checks F != T", [&]() { AssertThat(terminal_F, Is().Not().EqualTo(terminal_T)); });

        it("checks F != ~F", [&]() { AssertThat(terminal_F, Is().Not().EqualTo(~terminal_F)); });

        it("checks F != (x0 & x1)",
           [&]() { AssertThat(terminal_F, Is().Not().EqualTo(x0_and_x1)); });

        it("checks (x0 & x1) != T",
           [&]() { AssertThat(x0_and_x1, Is().Not().EqualTo(terminal_T)); });

        it("checks '(x0 & x1) != ~(x0 & x1)'",
           [&]() { AssertThat(x0_and_x1, Is().Not().EqualTo(x0_nand_x1)); });

        it("checks (x0 & x1) matches itself",
           [&]() { AssertThat(x0_and_x1, Is().EqualTo(x0_and_x1)); });

        it("checks (x0 & x1) matches a copy", [&]() {
          shared_levelized_file<bdd::node_type> other_nf;
          {
            node_writer nw(other_nf);
            nw << node(1, node::max_id, ptr_uint64(false), ptr_uint64(true))
               << node(0, node::max_id, ptr_uint64(false), ptr_uint64(1, ptr_uint64::max_id));
          }
          const bdd other(other_nf);
          AssertThat(x0_and_x1, Is().EqualTo(other));
        });
      });

      describe("~, &, |, ^", [&]() {
        it("negates 'T' into 'F'", [&]() { AssertThat(terminal_F, Is().EqualTo(~terminal_T)); });

        it("negates '(x0 & x1)' into ~(x0 & x1)'",
           [&]() { AssertThat(~x0_and_x1, Is().EqualTo(x0_nand_x1)); });

        it("negates '~(x0 & x1)' into '(x0 & x1)'",
           [&]() { AssertThat(~x0_and_x1, Is().EqualTo(x0_nand_x1)); });

        it("computes 'x0 & x1'", [&]() {
          const bdd f = x0 & x1;
          AssertThat(f, Is().EqualTo(x0_and_x1));
        });

        it("computes '~(~x0 | ~x1)'", [&]() {
          const bdd f = ~(~x0 | ~x1);
          AssertThat(f, Is().EqualTo(x0_and_x1));
        });

        it("computes 'x0 ^ x1' to be '(~x0 & x1) | (x0 & ~x1)'", [&]() {
          const bdd f = x0 ^ x1;

          const bdd g1 = ~x0 & x1;
          const bdd g2 = x0 & ~x1;
          const bdd g  = g1 | g2;

          AssertThat(f, Is().EqualTo(g));
        });

        it("negates 'x0 & x1' __bdd&& into '~(x0 & x1)", [&]() {
          AssertThat(x0_nand_x1 == ~(x0 & x1), Is().True());
          AssertThat(x0_nand_x1 != ~(x0 & x1), Is().False());
        });

        it("resolves 'x0 ^ x1' to be '(~x0 & x1) | (x0 & ~x1)'", [&]() {
          const bdd f = x0 ^ x1;
          const bdd g = (~x0 & x1) | (x0 & ~x1);
          AssertThat(f, Is().EqualTo(g));
        });

        it("accumulates with '&=(bdd&)' operator", [&]() {
          bdd f = x0;
          AssertThat(f == x0, Is().True());

          f &= x1;
          AssertThat(f == (x0 & x1), Is().True());

          f &= terminal_F;
          AssertThat(f == terminal_F, Is().True());
        });

        it("accumulates with '&=(__bdd&&)' operator", [&]() {
          bdd f = terminal_T;
          f &= x0 & x1;
          AssertThat((x0 & x1) == f, Is().True());
        });

        it("accumulates with '|=(bdd&)' operator", [&]() {
          bdd f = x0;
          AssertThat(f == x0, Is().True());

          f |= x1;
          AssertThat(f == (x0 | x1), Is().True());

          f |= terminal_T;
          AssertThat(terminal_T == f, Is().True());
        });

        it("accumulates with '|=(__bdd&&)' operator", [&]() {
          bdd f = terminal_F;
          f |= x0 & x1;
          AssertThat(f == (x0 & x1), Is().True());
        });

        it("accumulates with '^=(bdd&)' operator", [&]() {
          bdd f = x0;
          AssertThat(f == x0, Is().True());

          f ^= x1;
          AssertThat(f == (x0 ^ x1), Is().True());

          f ^= x0;
          AssertThat(f == x1, Is().True());
        });

        it("accumulates with '^=(__bdd&&)' operator", [&]() {
          bdd f = terminal_T;
          f ^= x0 & x1;
          AssertThat(~(x0 & x1) == f, Is().True());
        });

        it("computes with __bdd&& operators [1]", [&]() {
          const bdd f = ((x0 & ~x1) | (~x0 & x1)) ^ ((x1 ^ x0) & (~x0 & x1));
          AssertThat(f == (x0 & ~x1), Is().True());
          AssertThat((x0 & ~x1) == f, Is().True());
        });

        it("computes with __bdd&& operators [2]", [&]() {
          const bdd f = ((x0 & ~x1) | (~x0 & x1)) ^ ((x1 ^ x0) & (~x0 & x1));
          AssertThat(f == (x0 & ~x1), Is().True());
          AssertThat((x0 & ~x1) == f, Is().True());
        });

        it("computes with __bdd&& and bdd& operators [1]", [&]() {
          // Notice, that the two expressions with terminal_T and terminal_F
          // shortcut with the operator for a bdd with the negation flag
          // set correctly.
          const bdd f = ((x0 & x1) | (~x0 & x1)) ^ ((terminal_T ^ x0) & (terminal_F | x1));
          AssertThat(f == (x0 & x1), Is().True());
          AssertThat((x0 & x1) == f, Is().True());
        });

        it("computes with __bdd&& and bdd& operators [2]", [&]() {
          // The right-hand-side will evaluate to a bdd, since terminal_T negates.
          const bdd f = ((~x0 | (x0 & x1)) ^ (terminal_T ^ (x1 ^ x0)));
          AssertThat(f == (~x0 & x1), Is().True());
          AssertThat((~x0 & x1) == f, Is().True());
        });
      });

      describe("==, !=", [&]() {
        it("check two derivations of same __bdd&& [==]",
           [&]() { AssertThat((x0 ^ x1) == ((x0 & ~x1) | (~x0 & x1)), Is().True()); });

        it("check two derivations of same __bdd&& [!=]",
           [&]() { AssertThat((x0 ^ x1) != ((x0 & ~x1) | (~x0 & x1)), Is().False()); });

        it("checks two derivations of different __bdd&& [==]",
           [&]() { AssertThat((x0 | x1) == ((x0 & x1) | (~x0 & x1)), Is().False()); });

        it("checks two derivations of different __bdd&& [!=]",
           [&]() { AssertThat((x0 | x1) != ((x0 & x1) | (~x0 & x1)), Is().True()); });
      });
    });
  });
});
