go_bandit([]() {
  describe("src/adiar/bdd.h", []() {
    node_file x0_nf;

    {
      node_writer nw_0(x0_nf);
      nw_0 << create_node(0,MAX_ID,
                          create_sink_ptr(false),
                          create_sink_ptr(true));
    }

    bdd x0(x0_nf);

    node_file x1_nf;

    {
      node_writer nw_1(x1_nf);
      nw_1 << create_node(1,MAX_ID,
                          create_sink_ptr(false),
                          create_sink_ptr(true));
    }

    bdd x1(x1_nf);

    node_file x0_and_x1_nf;

    {
      node_writer nw_01(x0_and_x1_nf);

      nw_01 << create_node(1, MAX_ID,
                           create_sink_ptr(false),
                           create_sink_ptr(true));

      nw_01 << create_node(0, MAX_ID,
                           create_sink_ptr(false),
                           create_node_ptr(1, MAX_ID));
    }

    bdd x0_and_x1(x0_and_x1_nf);
    bdd x0_nand_x1(x0_and_x1_nf, true);

    node_file sink_T_nf;

    {
      node_writer nw_T(sink_T_nf);
      nw_T << create_sink(true);
    }

    bdd sink_T(sink_T_nf);

    node_file sink_F_nf;

    {
      node_writer nw_F(sink_F_nf);
      nw_F << create_sink(false);
    }

    bdd sink_F(sink_F_nf);

    describe("__bdd", [&]() {
      it("should copy-construct values from bdd", [&]() {
        __bdd t1 = bdd(x0_and_x1);
        AssertThat(t1.has<node_file>(), Is().True());
        AssertThat(t1.get<node_file>()._file_ptr, Is().EqualTo(x0_and_x1_nf._file_ptr));
        AssertThat(t1.negate, Is().False());
      });

      it("should copy-construct values from negated bdd", [&]() {
        __bdd t2 = bdd(x0_nand_x1);
        AssertThat(t2.has<node_file>(), Is().True());
        AssertThat(t2.get<node_file>()._file_ptr, Is().EqualTo(x0_and_x1_nf._file_ptr));
        AssertThat(t2.negate, Is().True());
      });

      it("should copy-construct values from __bdd", [&]() {
        __bdd t1 = x0_and_x1;
        __bdd t2 = t1;
        AssertThat(t2.has<node_file>(), Is().True());
        AssertThat(t2.get<node_file>()._file_ptr, Is().EqualTo(x0_and_x1_nf._file_ptr));
        AssertThat(t2.negate, Is().False());
      });

      it("should copy-construct values from node_file", [&]() {
        __bdd t1 = x0_and_x1;
        AssertThat(t1.has<node_file>(), Is().True());
        AssertThat(t1.get<node_file>()._file_ptr, Is().EqualTo(x0_and_x1_nf._file_ptr));
        AssertThat(t1.negate, Is().False());
      });

      arc_file af;

      {
        arc_writer aw(af);
        aw.unsafe_push_node(arc {flag(create_node_ptr(0,0)), create_node_ptr(1,0)});

        aw.unsafe_push_sink(arc {create_node_ptr(0,0), create_sink_ptr(false)});
        aw.unsafe_push_sink(arc {create_node_ptr(1,0), create_sink_ptr(true)});
        aw.unsafe_push_sink(arc {flag(create_node_ptr(1,0)), create_sink_ptr(true)});

        aw.unsafe_push(create_level_info(0,1u));
        aw.unsafe_push(create_level_info(1,1u));
      }

      af->max_1level_cut = 1;

      it("should copy-construct values from arc_file", [&]() {
        __bdd t1 = af;
        AssertThat(t1.has<arc_file>(), Is().True());
        AssertThat(t1.get<arc_file>()._file_ptr, Is().EqualTo(af._file_ptr));
        AssertThat(t1.negate, Is().False());
      });

      it("should reduce on copy construct to bdd with arc_file", [&]() {
        bdd out = __bdd(af);
        AssertThat(out, Is().EqualTo(x0));
      });
    });

    it("should copy-construct boolean 'true' value as a T sink", [&]() {
      bdd t1 = true;
      AssertThat(t1, Is().EqualTo(sink_T));
    });

    it("should copy-construct boolean 'false' value as an F sink", [&]() {
      bdd t2 = false;
      AssertThat(t2, Is().EqualTo(sink_F));
    });

    it("should copy-construct node_file and negation back to bdd", [&]() {
      bdd t2 = bdd(__bdd(x0_and_x1));
      AssertThat(t2.file_ptr(), Is().EqualTo(x0_and_x1_nf._file_ptr));
      AssertThat(t2.is_negated(), Is().False());
    });

    describe("operators", [&]() {
      it("should check sink_F != sink_T", [&]() {
        AssertThat(sink_F, Is().Not().EqualTo(sink_T));
      });

      it("should check sink_F != ~sink_F", [&]() {
        AssertThat(sink_F, Is().Not().EqualTo(~sink_F));
      });

      it("should check sink_F == ~sink_T", [&]() {
        AssertThat(sink_F, Is().EqualTo(~sink_T));
      });

      it("should check ~(x0 & x1) != (x0 & x1)", [&]() {
        AssertThat(x0_and_x1, Is().Not().EqualTo(x0_nand_x1));
      });

      node_file x0_and_x1_nf2;

      {
        node_writer nw_01(x0_and_x1_nf2);

        nw_01 << create_node(1, MAX_ID,
                             create_sink_ptr(false),
                             create_sink_ptr(true));

        nw_01 << create_node(0, MAX_ID,
                             create_sink_ptr(false),
                             create_node_ptr(1, MAX_ID));
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
        // Notice, that the two expressions with sink_T and sink_F
        // shortcut with the operator for a bdd with the negation flag
        // set correctly.
        bdd out = ((x0 & x1) | (~x0 & x1)) ^ ((sink_T ^ x0) & (sink_F | x1));
        AssertThat((x0 & x1) == out, Is().True());
        AssertThat((x0 & x1) != out, Is().False());
      });

      it("should compute with __bdd&& and bdd& in operators [2]", [&]() {
        // The right-hand-side will evaluate to a bdd, since sink_F negates.
        bdd out = ((~x0 | (x0 & x1)) ^ (sink_T ^ (x1 ^ x0)));
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

    describe("sink predicates", [&]() {
      describe("is_sink", [&]() {
        it("rejects x0 as a sink file", [&]() {
          AssertThat(is_sink(x0), Is().False());
        });

        it("rejects x0 & x1 as a sink file", [&]() {
          AssertThat(is_sink(x0_and_x1), Is().False());
        });

        it("accepts a true sink", [&]() {
          AssertThat(is_sink(sink_T), Is().True());
        });

        it("accepts a false sink", [&]() {
          AssertThat(is_sink(sink_F), Is().True());
        });

        it("accepts negation of a false sink", [&]() {
          AssertThat(is_sink(~ sink_F), Is().True());
        });

        it("accepts negation of a true sink", [&]() {
          AssertThat(is_sink(~ sink_T), Is().True());
        });
      });

      describe("value_of", [&]() {
        it("extracts from a true sink", [&]() {
          AssertThat(value_of(sink_T), Is().True());
        });

        it("extracts from a false sink", [&]() {
          AssertThat(value_of(sink_F), Is().False());
        });

        it("extracts from a negation of a false sink", [&]() {
          AssertThat(value_of(~ sink_F), Is().True());
        });

        it("extracts from a negation of a true sink", [&]() {
          AssertThat(value_of(~ sink_T), Is().False());
        });
      });

      describe("is_false", [&]() {
        it("rejects x0 as a false sink file", [&]() {
          AssertThat(is_false(x0), Is().False());
        });

        it("rejects x0 & x1 as a false sink file", [&]() {
          AssertThat(is_false(x0_and_x1), Is().False());
        });

        it("rejects a true sink", [&]() {
          AssertThat(is_false(sink_T), Is().False());
        });

        it("accepts a false sink", [&]() {
          AssertThat(is_false(sink_F), Is().True());
        });

        it("rejects negation of a false sink", [&]() {
          AssertThat(is_false(~ sink_F), Is().False());
        });

        it("accepts negation of a true sink", [&]() {
          AssertThat(is_false(~ sink_T), Is().True());
        });
      });

      describe("is_true", [&]() {
        it("rejects x0 as a true sink file", [&]() {
          AssertThat(is_true(x0), Is().False());
        });

        it("rejects x0 & x1 as a sink file", [&]() {
          AssertThat(is_true(x0_and_x1), Is().False());
        });

        it("accepts a true sink", [&]() {
          AssertThat(is_true(sink_T), Is().True());
        });

        it("rejects a false sink", [&]() {
          AssertThat(is_true(sink_F), Is().False());
        });

        it("accepts negation of a false sink", [&]() {
          AssertThat(is_true(~ sink_F), Is().True());
        });

        it("rejects negation of a true sink", [&]() {
          AssertThat(is_true(~ sink_T), Is().False());
        });
      });
    });
  });
 });
