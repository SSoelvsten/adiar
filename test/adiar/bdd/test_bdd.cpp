#include "../../test.h"

go_bandit([]() {
  describe("src/adiar/bdd.h", []() {
    shared_levelized_file<bdd::node_t> x0_nf;

    {
      node_writer nw_0(x0_nf);
      nw_0 << node(0, node::MAX_ID,
                          ptr_uint64(false),
                          ptr_uint64(true));
    }

    bdd x0(x0_nf);

    shared_levelized_file<bdd::node_t> x1_nf;

    {
      node_writer nw_1(x1_nf);
      nw_1 << node(1, node::MAX_ID,
                          ptr_uint64(false),
                          ptr_uint64(true));
    }

    bdd x1(x1_nf);

    shared_levelized_file<bdd::node_t> x0_and_x1_nf;

    {
      node_writer nw_01(x0_and_x1_nf);

      nw_01 << node(1, node::MAX_ID,
                           ptr_uint64(false),
                           ptr_uint64(true));

      nw_01 << node(0, node::MAX_ID,
                           ptr_uint64(false),
                           ptr_uint64(1, ptr_uint64::MAX_ID));
    }

    bdd x0_and_x1(x0_and_x1_nf);
    bdd x0_nand_x1(x0_and_x1_nf, true);

    shared_levelized_file<bdd::node_t> terminal_T_nf;

    {
      node_writer nw_T(terminal_T_nf);
      nw_T << node(true);
    }

    bdd terminal_T(terminal_T_nf);

    shared_levelized_file<bdd::node_t> terminal_F_nf;

    {
      node_writer nw_F(terminal_F_nf);
      nw_F << node(false);
    }

    bdd terminal_F(terminal_F_nf);

    describe("__bdd", [&]() {
      it("should copy-construct values from bdd", [&]() {
        __bdd t1 = bdd(x0_and_x1);
        AssertThat(t1.has<shared_levelized_file<bdd::node_t>>(), Is().True());
        AssertThat(t1.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(x0_and_x1_nf));
        AssertThat(t1.negate, Is().False());
      });

      it("should copy-construct values from negated bdd", [&]() {
        __bdd t2 = bdd(x0_nand_x1);
        AssertThat(t2.has<shared_levelized_file<bdd::node_t>>(), Is().True());
        AssertThat(t2.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(x0_and_x1_nf));
        AssertThat(t2.negate, Is().True());
      });

      it("should copy-construct values from __bdd", [&]() {
        __bdd t1 = x0_and_x1;
        __bdd t2 = t1;
        AssertThat(t2.has<shared_levelized_file<bdd::node_t>>(), Is().True());
        AssertThat(t2.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(x0_and_x1_nf));
        AssertThat(t2.negate, Is().False());
      });

      it("should copy-construct values from shared_levelized_file<bdd::node_t>", [&]() {
        __bdd t1 = x0_and_x1;
        AssertThat(t1.has<shared_levelized_file<bdd::node_t>>(), Is().True());
        AssertThat(t1.get<shared_levelized_file<bdd::node_t>>(), Is().EqualTo(x0_and_x1_nf));
        AssertThat(t1.negate, Is().False());
      });

      shared_levelized_file<arc> af;

      {
        arc_writer aw(af);
        aw.push_internal(arc {ptr_uint64(0,0), true,   ptr_uint64(1,0)});

        aw.push_terminal(arc {ptr_uint64(0,0), false, ptr_uint64(false)});
        aw.push_terminal(arc {ptr_uint64(1,0), false, ptr_uint64(true)});
        aw.push_terminal(arc {ptr_uint64(1,0), true,  ptr_uint64(true)});

        aw.push(level_info(0,1u));
        aw.push(level_info(1,1u));
      }

      af->max_1level_cut = 1;

      it("should copy-construct values from shared_levelized_file<arc>", [&]() {
        __bdd t1 = af;
        AssertThat(t1.has<shared_levelized_file<arc>>(), Is().True());
        AssertThat(t1.get<shared_levelized_file<arc>>(), Is().EqualTo(af));
        AssertThat(t1.negate, Is().False());
      });

      it("should reduce on copy construct to bdd with shared_levelized_file<arc>", [&]() {
        bdd out = __bdd(af);
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

    it("should copy-construct shared_levelized_file<bdd::node_t> and negation back to bdd", [&]() {
      bdd t2 = bdd(__bdd(x0_and_x1));
      AssertThat(t2.file_ptr(), Is().EqualTo(x0_and_x1_nf));
      AssertThat(t2.is_negated(), Is().False());
    });

    describe("operators", [&]() {
      it("should check terminal_F != terminal_T", [&]() {
        AssertThat(terminal_F, Is().Not().EqualTo(terminal_T));
      });

      it("should check terminal_F != ~terminal_F", [&]() {
        AssertThat(terminal_F, Is().Not().EqualTo(~terminal_F));
      });

      it("should check terminal_F == ~terminal_T", [&]() {
        AssertThat(terminal_F, Is().EqualTo(~terminal_T));
      });

      it("should check ~(x0 & x1) != (x0 & x1)", [&]() {
        AssertThat(x0_and_x1, Is().Not().EqualTo(x0_nand_x1));
      });

      shared_levelized_file<bdd::node_t> x0_and_x1_nf2;

      {
        node_writer nw_01(x0_and_x1_nf2);

        nw_01 << node(1, node::MAX_ID,
                             ptr_uint64(false),
                             ptr_uint64(true));

        nw_01 << node(0, node::MAX_ID,
                             ptr_uint64(false),
                             ptr_uint64(1, ptr_uint64::MAX_ID));
      }

      it("should check (x0 & x1) == (x0 & x1)", [&]() {
        bdd other(x0_and_x1_nf2);
        AssertThat(x0_and_x1, Is().EqualTo(other));
      });

      it("should compute x0 & x1", [&]() {
        AssertThat(x0_and_x1 == (x0 & x1), Is().True());
      });

      it("should compute bdd& in x0 ?= x1", [&]() {
        bdd out1 = x0; out1 &= x1;
        AssertThat(out1 == (x0 & x1), Is().True());
        AssertThat(out1 != (x0 & x1), Is().False());

        bdd out2 = x0; out2 |= x1;
        AssertThat(out2 == (x0 | x1), Is().True());
        AssertThat(out2 != (x0 | x1), Is().False());
        AssertThat(out2 != (x0 & x1), Is().True());

        bdd out3 = x0; out3 ^= x1;
        AssertThat(out3 == (x0 ^ x1), Is().True());
        AssertThat(out3 != (x0 ^ x1), Is().False());
        AssertThat(out3 != (x0 | x1), Is().True());
      });

      it("should negate __bdd&& in ~(x0 & x1)", [&]() {
        AssertThat(x0_nand_x1 == ~(x0 & x1), Is().True());
        AssertThat(x0_nand_x1 != ~(x0 & x1), Is().False());
      });

      it("should compute with __bdd&& operators", [&]() {
        bdd out = ((x0 & ~x1) | (~x0 & x1)) ^ ((x1 ^ x0) & (~x0 & x1));
        AssertThat((x0 & ~x1) == out, Is().True());
        AssertThat((x0 & ~x1) != out, Is().False());
      });

      it("should compute with __bdd&& and bdd& in operators [1]", [&]() {
        // Notice, that the two expressions with terminal_T and terminal_F
        // shortcut with the operator for a bdd with the negation flag
        // set correctly.
        bdd out = ((x0 & x1) | (~x0 & x1)) ^ ((terminal_T ^ x0) & (terminal_F | x1));
        AssertThat((x0 & x1) == out, Is().True());
        AssertThat((x0 & x1) != out, Is().False());
      });

      it("should compute with __bdd&& and bdd& in operators [2]", [&]() {
        // The right-hand-side will evaluate to a bdd, since terminal_F negates.
        bdd out = ((~x0 | (x0 & x1)) ^ (terminal_T ^ (x1 ^ x0)));
        AssertThat((~x0 & x1) == out, Is().True());
        AssertThat((~x0 & x1) != out, Is().False());
      });

      it("should x0 ?= __bdd&&", [&]() {
        bdd out = x0;
        out &= x0 & x1;
        AssertThat((x0 & x1) == out, Is().True());
        AssertThat((x0 & x1) != out, Is().False());
      });

      it("should check two derivations of same __bdd&&", [&]() {
        AssertThat((x0 ^ x1) == ((x0 & ~x1) | (~x0 & x1)), Is().True());
        AssertThat((x0 ^ x1) != ((x0 & ~x1) | (~x0 & x1)), Is().False());
      });

      it("should check two derivations of different __bdd&&", [&]() {
        AssertThat((x0 | x1) == ((x0 & x1) | (~x0 & x1)), Is().False());
        AssertThat((x0 | x1) != ((x0 & x1) | (~x0 & x1)), Is().True());
      });
    });

    describe("terminal predicates", [&]() {
      describe("is_terminal", [&]() {
        it("rejects x0 as a terminal file", [&]() {
          AssertThat(is_terminal(x0), Is().False());
        });

        it("rejects x0 & x1 as a terminal file", [&]() {
          AssertThat(is_terminal(x0_and_x1), Is().False());
        });

        it("accepts a true terminal", [&]() {
          AssertThat(is_terminal(terminal_T), Is().True());
        });

        it("accepts a false terminal", [&]() {
          AssertThat(is_terminal(terminal_F), Is().True());
        });

        it("accepts negation of a false terminal", [&]() {
          AssertThat(is_terminal(~ terminal_F), Is().True());
        });

        it("accepts negation of a true terminal", [&]() {
          AssertThat(is_terminal(~ terminal_T), Is().True());
        });
      });

      describe("value_of", [&]() {
        it("extracts from a true terminal", [&]() {
          AssertThat(value_of(terminal_T), Is().True());
        });

        it("extracts from a false terminal", [&]() {
          AssertThat(value_of(terminal_F), Is().False());
        });

        it("extracts from a negation of a false terminal", [&]() {
          AssertThat(value_of(~ terminal_F), Is().True());
        });

        it("extracts from a negation of a true terminal", [&]() {
          AssertThat(value_of(~ terminal_T), Is().False());
        });
      });

      describe("is_false", [&]() {
        it("rejects x0 as a false terminal file", [&]() {
          AssertThat(is_false(x0), Is().False());
        });

        it("rejects x0 & x1 as a false terminal file", [&]() {
          AssertThat(is_false(x0_and_x1), Is().False());
        });

        it("rejects a true terminal", [&]() {
          AssertThat(is_false(terminal_T), Is().False());
        });

        it("accepts a false terminal", [&]() {
          AssertThat(is_false(terminal_F), Is().True());
        });

        it("rejects negation of a false terminal", [&]() {
          AssertThat(is_false(~ terminal_F), Is().False());
        });

        it("accepts negation of a true terminal", [&]() {
          AssertThat(is_false(~ terminal_T), Is().True());
        });
      });

      describe("is_true", [&]() {
        it("rejects x0 as a true terminal file", [&]() {
          AssertThat(is_true(x0), Is().False());
        });

        it("rejects x0 & x1 as a terminal file", [&]() {
          AssertThat(is_true(x0_and_x1), Is().False());
        });

        it("accepts a true terminal", [&]() {
          AssertThat(is_true(terminal_T), Is().True());
        });

        it("rejects a false terminal", [&]() {
          AssertThat(is_true(terminal_F), Is().False());
        });

        it("accepts negation of a false terminal", [&]() {
          AssertThat(is_true(~ terminal_F), Is().True());
        });

        it("rejects negation of a true terminal", [&]() {
          AssertThat(is_true(~ terminal_T), Is().False());
        });
      });
    });
  });
 });
