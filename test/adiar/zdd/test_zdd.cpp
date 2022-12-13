#include "../../test.h"

go_bandit([]() {
  describe("adiar/zdd/zdd.h", [&]() {
    node_file x0_nf;
    {
      node_writer nw_0(x0_nf);
      nw_0 << node(0, node::MAX_ID,
                          ptr_uint64(false),
                          ptr_uint64(true));
    }

    zdd x0(x0_nf);

    node_file x1_nf;
    {
      node_writer nw_1(x1_nf);
      nw_1 << node(1, node::MAX_ID,
                          ptr_uint64(false),
                          ptr_uint64(true));
    }

    zdd x1(x1_nf);

    node_file x0_or_x1_nf;
    {
      node_writer nw_01(x0_or_x1_nf);

      nw_01 << node(1, node::MAX_ID,
                           ptr_uint64(false),
                           ptr_uint64(true));

      nw_01 << node(0, node::MAX_ID,
                           ptr_uint64(1, ptr_uint64::MAX_ID),
                           ptr_uint64(true));
    }

    zdd x0_or_x1(x0_or_x1_nf);

    node_file terminal_T_nf;
    {
      node_writer nw_T(terminal_T_nf);
      nw_T << node(true);
    }

    zdd terminal_T(terminal_T_nf);

    node_file terminal_F_nf;
    {
      node_writer nw_F(terminal_F_nf);
      nw_F << node(false);
    }

    zdd terminal_F(terminal_F_nf);

    describe("__zdd class", [&]() {
      it("should copy-construct values from zdd", [&]() {
        __zdd t1 = x0_or_x1;
        AssertThat(t1.has<node_file>(), Is().True());
        AssertThat(t1.get<node_file>(), Is().EqualTo(x0_or_x1_nf));
        AssertThat(t1.negate, Is().False());
      });

      it("should copy-construct values from __zdd", [&]() {
        __zdd t1 = x0_or_x1;
        __zdd t2 = t1;
        AssertThat(t2.has<node_file>(), Is().True());
        AssertThat(t2.get<node_file>(), Is().EqualTo(x0_or_x1_nf));
        AssertThat(t2.negate, Is().False());
      });

      it("should copy-construct values from node_file", [&]() {
        __zdd t1 = x0_or_x1;
        AssertThat(t1.has<node_file>(), Is().True());
        AssertThat(t1.get<node_file>(), Is().EqualTo(x0_or_x1_nf));
        AssertThat(t1.negate, Is().False());
      });

      arc_file af;

      {
        arc_writer aw(af);
        aw.push_internal(arc {ptr_uint64(0,0), ptr_uint64(1,0)});

        aw.push_terminal(arc {flag(ptr_uint64(0,0)), ptr_uint64(true)});
        aw.push_terminal(arc {ptr_uint64(1,0), ptr_uint64(false)});
        aw.push_terminal(arc {flag(ptr_uint64(1,0)), ptr_uint64(true)});

        aw.push(create_level_info(0,1u));
        aw.push(create_level_info(1,1u));
      }

      af->max_1level_cut = 1;

      it("should copy-construct values from arc_file", [&]() {
        __zdd t1 = af;
        AssertThat(t1.has<arc_file>(), Is().True());
        AssertThat(t1.get<arc_file>(), Is().EqualTo(af));
        AssertThat(t1.negate, Is().False());
      });

      it("should reduce on copy construct to zdd with arc_file", [&]() {
        zdd out = __zdd(af);
        AssertThat(out, Is().EqualTo(x0_or_x1));
      });
    });

    describe("operators", [&]() {
      it("should reject Ø == {Ø}", [&]() {
        AssertThat(terminal_F, Is().Not().EqualTo(terminal_T));
      });

      it("should accept {{0}} == {{0}} (different files)", [&]() {
        node_file x0_nf_2;

        { node_writer nw_0(x0_nf_2);
          nw_0 << node(0, node::MAX_ID,
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
        label_file dom;
        {
          label_writer lw(dom);
          lw << 0 << 1;
        }

        adiar_set_domain(dom);

        node_file expected;
        {
          node_writer nw(expected);
          nw << node(1, node::MAX_ID, ptr_uint64(false), ptr_uint64(true))
             << node(0, node::MAX_ID, ptr_uint64(true), ptr_uint64(1, ptr_uint64::MAX_ID));
        }

        AssertThat(~x0_or_x1 == zdd(expected), Is().True());
      });

      it("should compute with __zdd&& operators [|,&,-]", [&]() {
        zdd out = ((x0_or_x1 - x0) | ((x0 | x1) & (x0_or_x1 - x1))) - (x0_or_x1 - x0);
        AssertThat(x0, Is().EqualTo(out));
      });

      it("should compute with __zdd&& operators [|,~,-,]", [&]() {
          label_file dom;
          {
            label_writer lw(dom);
            lw << 0 << 1;
          }

          adiar_set_domain(dom);

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

    it("should copy-construct node_file and negation back to zdd", [&]() {
      zdd t2 = zdd(__zdd(x0_or_x1));
      AssertThat(t2.file_ptr(), Is().EqualTo(x0_or_x1_nf));
      AssertThat(t2.is_negated(), Is().False());
    });

    describe("terminal predicates", [&]() {
      describe("is_terminal", [&]() {
        it("rejects x0 as a terminal file", [&]() {
          AssertThat(is_terminal(x0), Is().False());
        });

        it("rejects x0 & x1 as a terminal file", [&]() {
          AssertThat(is_terminal(x0_or_x1), Is().False());
        });

        it("accepts a true terminal", [&]() {
          AssertThat(is_terminal(terminal_T), Is().True());
        });

        it("accepts a false terminal", [&]() {
          AssertThat(is_terminal(terminal_F), Is().True());
        });
      });

      describe("value_of", [&]() {
        it("extracts from a true terminal", [&]() {
          AssertThat(value_of(terminal_T), Is().True());
        });

        it("extracts from a false terminal", [&]() {
          AssertThat(value_of(terminal_F), Is().False());
        });
      });

      describe("is_false / is_empty", [&]() {
        it("rejects x0 as a false terminal file", [&]() {
          AssertThat(is_false(x0), Is().False());
          AssertThat(is_empty(x0), Is().False());
        });

        it("rejects x0 & x1 as a false terminal file", [&]() {
          AssertThat(is_false(x0_or_x1), Is().False());
          AssertThat(is_empty(x0_or_x1), Is().False());
        });

        it("rejects a true terminal", [&]() {
          AssertThat(is_false(terminal_T), Is().False());
          AssertThat(is_empty(terminal_T), Is().False());
        });

        it("accepts a false terminal", [&]() {
          AssertThat(is_false(terminal_F), Is().True());
          AssertThat(is_empty(terminal_F), Is().True());
        });
      });

      describe("is_true / is_null", [&]() {
        it("rejects x0 as a true terminal file", [&]() {
          AssertThat(is_true(x0), Is().False());
          AssertThat(is_null(x0), Is().False());
        });

        it("rejects x0 & x1 as a terminal file", [&]() {
          AssertThat(is_true(x0_or_x1), Is().False());
          AssertThat(is_null(x0_or_x1), Is().False());
        });

        it("accepts a true terminal", [&]() {
          AssertThat(is_true(terminal_T), Is().True());
          AssertThat(is_null(terminal_T), Is().True());
        });

        it("rejects a false terminal", [&]() {
          AssertThat(is_true(terminal_F), Is().False());
          AssertThat(is_null(terminal_F), Is().False());
        });
      });
    });
  });
 });
