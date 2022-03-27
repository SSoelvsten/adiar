go_bandit([]() {
  describe("adiar/data.h", []() {

    describe("NIL", [&](){
      it("should recognise NIL (unflagged)", [&]() {
        auto some_value = NIL;
        AssertThat(is_nil(some_value), Is().True());
      });

      it("should recognise NIL (flagged)", [&]() {
        auto some_value = flag(NIL);
        AssertThat(is_nil(some_value), Is().True());
      });


      it("can see whether the flag is set", [&]() {
        AssertThat(is_flagged(flag(NIL)), Is().True());
      });

      it("can see whether the flag is not set", [&]() {
        AssertThat(is_flagged(NIL), Is().False());
      });
    });

    describe("ptr_t (sinks)", [&](){
      it("should take up 8 bytes of memory", [&]() {
        ptr_t sink = create_sink_ptr(false);
        AssertThat(sizeof(sink), Is().EqualTo(8u));
      });

      describe("flag, unflag, is_flagged", []() {
        it("is unflagged by default", [&]() {
          ptr_t p = create_sink_ptr(true);
          AssertThat(is_flagged(p), Is().False());

          p = create_sink_ptr(true);
          AssertThat(is_flagged(p), Is().False());
        });

        it("can set the flag", [&]() {
          ptr_t p = flag(create_sink_ptr(false));
          AssertThat(is_flagged(p), Is().True());

          p = flag(create_sink_ptr(true));
          AssertThat(is_flagged(p), Is().True());
        });

        it("can unset the flag", [&]() {
          ptr_t p = flag(create_sink_ptr(false));
          p = unflag(p);
          AssertThat(is_flagged(p), Is().False());

          p = flag(create_sink_ptr(true));
          p = unflag(p);
          AssertThat(is_flagged(p), Is().False());
        });
      });

      describe("is_sink", []() {
        it("should recognise Sinks as such", [&]() {
          ptr_t sink_f = create_sink_ptr(false);
          ptr_t sink_t = create_sink_ptr(true);

          AssertThat(is_sink(sink_f), Is().True());
          AssertThat(is_sink(sink_t), Is().True());
        });

        it("should not be confused with Node Ptr (unflagged)", [&]() {
          ptr_t arc_node_max = create_node_ptr(MAX_LABEL,MAX_ID);
          AssertThat(is_sink(arc_node_max), Is().False());
          AssertThat(is_false(arc_node_max), Is().False());
          AssertThat(is_true(arc_node_max), Is().False());

          ptr_t arc_node_min = create_node_ptr(0,0);
          AssertThat(is_sink(arc_node_min), Is().False());
          AssertThat(is_false(arc_node_min), Is().False());
          AssertThat(is_true(arc_node_min), Is().False());

          ptr_t arc_node = create_node_ptr(42,18);
          AssertThat(is_sink(arc_node), Is().False());
          AssertThat(is_false(arc_node), Is().False());
          AssertThat(is_true(arc_node), Is().False());
        });

        it("should not be confused with Node Ptr (flagged)", [&]() {
          ptr_t arc_node_max = flag(create_node_ptr(MAX_LABEL,MAX_ID));
          AssertThat(is_sink(arc_node_max), Is().False());
          AssertThat(is_false(arc_node_max), Is().False());
          AssertThat(is_true(arc_node_max), Is().False());

          ptr_t arc_node_min = flag(create_node_ptr(0,0));
          AssertThat(is_sink(arc_node_min), Is().False());
          AssertThat(is_false(arc_node_min), Is().False());
          AssertThat(is_true(arc_node_min), Is().False());

          ptr_t arc_node = flag(create_node_ptr(42,18));
          AssertThat(is_sink(arc_node), Is().False());
          AssertThat(is_false(arc_node), Is().False());
          AssertThat(is_true(arc_node), Is().False());
        });

        it("should not be confused with Nil (unflagged)", [&]() {
          AssertThat(is_sink(NIL), Is().False());
        });

        it("should not be confused with Nil (flagged)", [&]() {
          AssertThat(is_sink(flag(NIL)), Is().False());
        });
      });

      describe("value_of", []() {
        it("retrieves value from false sink", [&]() {
          ptr_t p = create_sink_ptr(false);
          AssertThat(value_of(p), Is().False());
        });

        it("retrieves value from true sink", [&]() {
          ptr_t p = create_sink_ptr(true);
          AssertThat(value_of(p), Is().True());
        });
      });

      describe("is_false", []() {
        it("should accept false sink", [&]() {
          ptr_t sink_f = create_sink_ptr(false);
          AssertThat(is_false(sink_f), Is().True());
        });

        it("should accept false sink (flagged)", [&]() {
          ptr_t sink_f = flag(create_sink_ptr(false));
          AssertThat(is_false(sink_f), Is().True());
        });

        it("should reject sink", [&]() {
          ptr_t sink_t = create_sink_ptr(true);
          AssertThat(is_false(sink_t), Is().False());
        });

        it("should reject non-sink", [&]() {
          ptr_t p = create_node_ptr(0,0);
          AssertThat(is_false(p), Is().False());
        });

        it("should reject NIL", [&]() {
          ptr_t p = NIL;
          AssertThat(is_false(p), Is().False());
        });
      });

      describe("is_true", []() {
        it("should reject false sink", [&]() {
          ptr_t sink_f = create_sink_ptr(false);
          AssertThat(is_true(sink_f), Is().False());
        });

        it("should accept true sink", [&]() {
          ptr_t sink_t = create_sink_ptr(true);
          AssertThat(is_true(sink_t), Is().True());
        });

        it("should accept true sink (flagged)", [&]() {
          ptr_t sink_t = flag(create_sink_ptr(true));
          AssertThat(is_true(sink_t), Is().True());
        });

        it("should reject non-sink", [&]() {
          ptr_t p = create_node_ptr(0,1);
          AssertThat(is_true(p), Is().False());
        });

        it("should reject NIL", [&]() {
          ptr_t p = NIL;
          AssertThat(is_true(p), Is().False());
        });
      });

      describe("negate", []() {
        it("should negate sink (unflagged)", [&]() {
          ptr_t p1 = create_sink_ptr(false);
          AssertThat(negate(p1), Is().EqualTo(create_sink_ptr(true)));

          ptr_t p2 = create_sink_ptr(true);
          AssertThat(negate(p2), Is().EqualTo(create_sink_ptr(false)));
        });

        it("should negate sink into sink (flagged)", [&]() {
          ptr_t p1 = flag(create_sink_ptr(false));
          AssertThat(negate(p1), Is().EqualTo(flag(create_sink_ptr(true))));

          ptr_t p2 = flag(create_sink_ptr(true));
          AssertThat(negate(p2), Is().EqualTo(flag(create_sink_ptr(false))));
        });
      });

      describe("bool_op", []() {
        it("and_op", []() {
          AssertThat(and_op(create_sink_ptr(true), create_sink_ptr(true)),
                     Is().EqualTo(create_sink_ptr(true)));
          AssertThat(and_op(create_sink_ptr(true), create_sink_ptr(false)),
                     Is().EqualTo(create_sink_ptr(false)));
          AssertThat(and_op(create_sink_ptr(false), create_sink_ptr(true)),
                     Is().EqualTo(create_sink_ptr(false)));
          AssertThat(and_op(create_sink_ptr(false), create_sink_ptr(false)),
                     Is().EqualTo(create_sink_ptr(false)));
        });

        it("nand_op", []() {
          AssertThat(nand_op(create_sink_ptr(true), create_sink_ptr(true)),
                     Is().EqualTo(create_sink_ptr(false)));
          AssertThat(nand_op(create_sink_ptr(true), create_sink_ptr(false)),
                     Is().EqualTo(create_sink_ptr(true)));
          AssertThat(nand_op(create_sink_ptr(false), create_sink_ptr(true)),
                     Is().EqualTo(create_sink_ptr(true)));
          AssertThat(nand_op(create_sink_ptr(false), create_sink_ptr(false)),
                     Is().EqualTo(create_sink_ptr(true)));
        });

        it("or_op", []() {
          AssertThat(or_op(create_sink_ptr(true), create_sink_ptr(true)),
                     Is().EqualTo(create_sink_ptr(true)));
          AssertThat(or_op(create_sink_ptr(true), create_sink_ptr(false)),
                     Is().EqualTo(create_sink_ptr(true)));
          AssertThat(or_op(create_sink_ptr(false), create_sink_ptr(true)),
                     Is().EqualTo(create_sink_ptr(true)));
          AssertThat(or_op(create_sink_ptr(false), create_sink_ptr(false)),
                     Is().EqualTo(create_sink_ptr(false)));
        });

        it("nor_op", []() {
          AssertThat(nor_op(create_sink_ptr(true), create_sink_ptr(true)),
                     Is().EqualTo(create_sink_ptr(false)));
          AssertThat(nor_op(create_sink_ptr(true), create_sink_ptr(false)),
                     Is().EqualTo(create_sink_ptr(false)));
          AssertThat(nor_op(create_sink_ptr(false), create_sink_ptr(true)),
                     Is().EqualTo(create_sink_ptr(false)));
          AssertThat(nor_op(create_sink_ptr(false), create_sink_ptr(false)),
                     Is().EqualTo(create_sink_ptr(true)));
        });

        it("xor_op", []() {
          AssertThat(xor_op(create_sink_ptr(true), create_sink_ptr(true)),
                     Is().EqualTo(create_sink_ptr(false)));
          AssertThat(xor_op(create_sink_ptr(true), create_sink_ptr(false)),
                     Is().EqualTo(create_sink_ptr(true)));
          AssertThat(xor_op(create_sink_ptr(false), create_sink_ptr(true)),
                     Is().EqualTo(create_sink_ptr(true)));
          AssertThat(xor_op(create_sink_ptr(false), create_sink_ptr(false)),
                     Is().EqualTo(create_sink_ptr(false)));
        });

        it("xnor_op", []() {
          AssertThat(xnor_op(create_sink_ptr(true), create_sink_ptr(true)),
                     Is().EqualTo(create_sink_ptr(true)));
          AssertThat(xnor_op(create_sink_ptr(true), create_sink_ptr(false)),
                     Is().EqualTo(create_sink_ptr(false)));
          AssertThat(xnor_op(create_sink_ptr(false), create_sink_ptr(true)),
                     Is().EqualTo(create_sink_ptr(false)));
          AssertThat(xnor_op(create_sink_ptr(false), create_sink_ptr(false)),
                     Is().EqualTo(create_sink_ptr(true)));
        });

        it("imp_op", []() {
          AssertThat(imp_op(create_sink_ptr(true), create_sink_ptr(true)),
                     Is().EqualTo(create_sink_ptr(true)));
          AssertThat(imp_op(create_sink_ptr(true), create_sink_ptr(false)),
                     Is().EqualTo(create_sink_ptr(false)));
          AssertThat(imp_op(create_sink_ptr(false), create_sink_ptr(true)),
                     Is().EqualTo(create_sink_ptr(true)));
          AssertThat(imp_op(create_sink_ptr(false), create_sink_ptr(false)),
                     Is().EqualTo(create_sink_ptr(true)));
        });

        it("invimp_op", []() {
          AssertThat(invimp_op(create_sink_ptr(true), create_sink_ptr(true)),
                     Is().EqualTo(create_sink_ptr(true)));
          AssertThat(invimp_op(create_sink_ptr(true), create_sink_ptr(false)),
                     Is().EqualTo(create_sink_ptr(true)));
          AssertThat(invimp_op(create_sink_ptr(false), create_sink_ptr(true)),
                     Is().EqualTo(create_sink_ptr(false)));
          AssertThat(invimp_op(create_sink_ptr(false), create_sink_ptr(false)),
                     Is().EqualTo(create_sink_ptr(true)));
        });

        it("equiv_op", []() {
          AssertThat(equiv_op(create_sink_ptr(true), create_sink_ptr(true)),
                     Is().EqualTo(create_sink_ptr(true)));
          AssertThat(equiv_op(create_sink_ptr(true), create_sink_ptr(false)),
                     Is().EqualTo(create_sink_ptr(false)));
          AssertThat(equiv_op(create_sink_ptr(false), create_sink_ptr(true)),
                     Is().EqualTo(create_sink_ptr(false)));
          AssertThat(equiv_op(create_sink_ptr(false), create_sink_ptr(false)),
                     Is().EqualTo(create_sink_ptr(true)));
        });

        it("equiv_op (flags)", []() {
          AssertThat(equiv_op(flag(create_sink_ptr(true)), create_sink_ptr(true)),
                     Is().EqualTo(create_sink_ptr(true)));
          AssertThat(equiv_op(create_sink_ptr(true), flag(create_sink_ptr(false))),
                     Is().EqualTo(create_sink_ptr(false)));
          AssertThat(equiv_op(flag(create_sink_ptr(false)), create_sink_ptr(true)),
                     Is().EqualTo(create_sink_ptr(false)));
          AssertThat(equiv_op(create_sink_ptr(false), flag(create_sink_ptr(false))),
                     Is().EqualTo(create_sink_ptr(true)));
        });

        it("diff_op", []() {
          AssertThat(diff_op(create_sink_ptr(true), create_sink_ptr(true)),
                     Is().EqualTo(create_sink_ptr(false)));
          AssertThat(diff_op(create_sink_ptr(true), create_sink_ptr(false)),
                     Is().EqualTo(create_sink_ptr(true)));
          AssertThat(diff_op(create_sink_ptr(false), create_sink_ptr(true)),
                     Is().EqualTo(create_sink_ptr(false)));
          AssertThat(diff_op(create_sink_ptr(false), create_sink_ptr(false)),
                     Is().EqualTo(create_sink_ptr(false)));
        });

        it("less_op", []() {
          AssertThat(less_op(create_sink_ptr(true), create_sink_ptr(true)),
                     Is().EqualTo(create_sink_ptr(false)));
          AssertThat(less_op(create_sink_ptr(true), create_sink_ptr(false)),
                     Is().EqualTo(create_sink_ptr(false)));
          AssertThat(less_op(create_sink_ptr(false), create_sink_ptr(true)),
                     Is().EqualTo(create_sink_ptr(true)));
          AssertThat(less_op(create_sink_ptr(false), create_sink_ptr(false)),
                     Is().EqualTo(create_sink_ptr(false)));
        });

        describe("can_shortcut", []() {
          it("can check on T sink on the left", [&]() {
            AssertThat(can_left_shortcut(and_op, create_sink_ptr(true)), Is().False());
            AssertThat(can_left_shortcut(or_op, create_sink_ptr(true)), Is().True());
            AssertThat(can_left_shortcut(xor_op, create_sink_ptr(true)), Is().False());
            AssertThat(can_left_shortcut(imp_op, create_sink_ptr(true)), Is().False());
          });

          it("can check on F sink on the left", [&]() {
            AssertThat(can_left_shortcut(and_op, create_sink_ptr(false)), Is().True());
            AssertThat(can_left_shortcut(or_op, create_sink_ptr(false)), Is().False());
            AssertThat(can_left_shortcut(xor_op, create_sink_ptr(false)), Is().False());
            AssertThat(can_left_shortcut(imp_op, create_sink_ptr(false)), Is().True());
          });

          it("can check on T sink on the right", [&]() {
            AssertThat(can_right_shortcut(and_op, create_sink_ptr(true)), Is().False());
            AssertThat(can_right_shortcut(or_op, create_sink_ptr(true)), Is().True());
            AssertThat(can_right_shortcut(xor_op, create_sink_ptr(true)), Is().False());
            AssertThat(can_right_shortcut(imp_op, create_sink_ptr(true)), Is().True());
          });

          it("can check on F sink on the right", [&]() {
            AssertThat(can_right_shortcut(and_op, create_sink_ptr(false)), Is().True());
            AssertThat(can_right_shortcut(or_op, create_sink_ptr(false)), Is().False());
            AssertThat(can_right_shortcut(xor_op, create_sink_ptr(false)), Is().False());
            AssertThat(can_right_shortcut(imp_op, create_sink_ptr(false)), Is().False());
          });
        });

        describe("is_irrelevant", []() {
          it("can check on T sink on the left", [&]() {
            AssertThat(is_left_irrelevant(and_op, create_sink_ptr(true)), Is().True());
            AssertThat(is_left_irrelevant(or_op, create_sink_ptr(true)), Is().False());
            AssertThat(is_left_irrelevant(xor_op, create_sink_ptr(true)), Is().False());
            AssertThat(is_left_irrelevant(imp_op, create_sink_ptr(true)), Is().True());
          });

          it("can check on F sink on the left", [&]() {
            AssertThat(is_left_irrelevant(and_op, create_sink_ptr(false)), Is().False());
            AssertThat(is_left_irrelevant(or_op, create_sink_ptr(false)), Is().True());
            AssertThat(is_left_irrelevant(xor_op, create_sink_ptr(false)), Is().True());
            AssertThat(is_left_irrelevant(imp_op, create_sink_ptr(false)), Is().False());
          });

          it("can check on T sink on the right", [&]() {
            AssertThat(is_right_irrelevant(and_op, create_sink_ptr(true)), Is().True());
            AssertThat(is_right_irrelevant(or_op, create_sink_ptr(true)), Is().False());
            AssertThat(is_right_irrelevant(xor_op, create_sink_ptr(true)), Is().False());
            AssertThat(is_right_irrelevant(imp_op, create_sink_ptr(true)), Is().False());
          });

          it("can check on F sink on the right", [&]() {
            AssertThat(is_right_irrelevant(and_op, create_sink_ptr(false)), Is().False());
            AssertThat(is_right_irrelevant(or_op, create_sink_ptr(false)), Is().True());
            AssertThat(is_right_irrelevant(xor_op, create_sink_ptr(false)), Is().True());
            AssertThat(is_right_irrelevant(imp_op, create_sink_ptr(false)), Is().False());
          });
        });

        describe("is_negating", []() {
          it("can check on T sink on the left", [&]() {
            AssertThat(is_left_negating(and_op, create_sink_ptr(true)), Is().False());
            AssertThat(is_left_negating(or_op, create_sink_ptr(true)), Is().False());
            AssertThat(is_left_negating(xor_op, create_sink_ptr(true)), Is().True());
            AssertThat(is_left_negating(imp_op, create_sink_ptr(true)), Is().False());
          });

          it("can check on F sink on the left", [&]() {
            AssertThat(is_left_negating(and_op, create_sink_ptr(false)), Is().False());
            AssertThat(is_left_negating(or_op, create_sink_ptr(false)), Is().False());
            AssertThat(is_left_negating(xor_op, create_sink_ptr(false)), Is().False());
            AssertThat(is_left_negating(imp_op, create_sink_ptr(false)), Is().False());
          });

          it("can check on T sink on the right", [&]() {
            AssertThat(is_right_negating(and_op, create_sink_ptr(true)), Is().False());
            AssertThat(is_right_negating(or_op, create_sink_ptr(true)), Is().False());
            AssertThat(is_right_negating(xor_op, create_sink_ptr(true)), Is().True());
            AssertThat(is_right_negating(imp_op, create_sink_ptr(true)), Is().False());
          });

          it("can check on F sink on the right", [&]() {
            AssertThat(is_right_negating(and_op, create_sink_ptr(false)), Is().False());
            AssertThat(is_right_negating(or_op, create_sink_ptr(false)), Is().False());
            AssertThat(is_right_negating(xor_op, create_sink_ptr(false)), Is().False());
            AssertThat(is_right_negating(imp_op, create_sink_ptr(false)), Is().False());
          });
        });

        it("can check the operators for being commutative", [&]() {
          AssertThat(is_commutative(and_op), Is().True());
          AssertThat(is_commutative(nand_op), Is().True());
          AssertThat(is_commutative(or_op), Is().True());
          AssertThat(is_commutative(nor_op), Is().True());
          AssertThat(is_commutative(xor_op), Is().True());
          AssertThat(is_commutative(imp_op), Is().False());
          AssertThat(is_commutative(invimp_op), Is().False());
          AssertThat(is_commutative(equiv_op), Is().True());
          AssertThat(is_commutative(diff_op), Is().False());
          AssertThat(is_commutative(diff_op), Is().False());
        });
      });
    });

    describe("ptr_t (nodes)", [&]() {
      it("should take up 8 bytes of memory", [&]() {
        ptr_t node_ptr = create_node_ptr(42,2);
        AssertThat(sizeof(node_ptr), Is().EqualTo(8u));
      });

      describe("flag, unflag, is_flagged", [&]() {
        it("can recognise the flag is set", [&]() {
          ptr_t p = flag(create_node_ptr(42,2));
          AssertThat(is_flagged(p), Is().True());

          p = flag(create_node_ptr(17,3));
          AssertThat(is_flagged(p), Is().True());
        });

        it("can recognise the flag is not set", [&]() {
          ptr_t p = create_node_ptr(42,2);
          AssertThat(is_flagged(p), Is().False());

          p = create_node_ptr(17,3);
          AssertThat(is_flagged(p), Is().False());
        });
      });

      describe("create_node_ptr, label_of, id_of", [&]() {
        it("should store and retrieve label for Ptr with maximal id (unflagged)", [&]() {
          ptr_t p = create_node_ptr(12,MAX_ID);
          AssertThat(label_of(p), Is().EqualTo(12u));
        });

        it("should store and retrieve 42 label Ptr (unflagged)", [&]() {
          ptr_t p = create_node_ptr(42,2);
          AssertThat(label_of(p), Is().EqualTo(42u));
        });

        it("should store and retrieve 21 label Ptr (unflagged)", [&]() {
          ptr_t p = create_node_ptr(21,2);
          AssertThat(label_of(p), Is().EqualTo(21u));
        });

        it("should store and retrieve MAX label Ptr (unflagged)", [&]() {
          ptr_t p = create_node_ptr(MAX_LABEL, MAX_ID);
          AssertThat(label_of(p), Is().EqualTo(MAX_LABEL));
        });

        it("should store and retrieve label for Ptr with maximal id (flagged)", [&]() {
          ptr_t p = flag(create_node_ptr(12,MAX_ID));
          AssertThat(label_of(p), Is().EqualTo(12u));
        });

        it("should store and retrieve 42 label Ptr (flagged)", [&]() {
          ptr_t p = flag(create_node_ptr(42,2));
          AssertThat(label_of(p), Is().EqualTo(42u));
        });

        it("should store and retrieve 21 label Ptr (flagged)", [&]() {
          ptr_t p = flag(create_node_ptr(21,2));
          AssertThat(label_of(p), Is().EqualTo(21u));
        });

        it("should store and retrieve MAX label Ptr (flagged)", [&]() {
          ptr_t p = create_node_ptr(MAX_LABEL, MAX_ID);
          AssertThat(label_of(p), Is().EqualTo(MAX_LABEL));
        });

        it("should store and retrieve 42 id (unflagged)", [&]() {
          ptr_t p = create_node_ptr(2,42);
          AssertThat(id_of(p), Is().EqualTo(42u));
        });

        it("should store and retrieve 21 id (unflagged)", [&]() {
          ptr_t p = create_node_ptr(2,21);
          AssertThat(id_of(p), Is().EqualTo(21u));
        });

        it("should store and retrieve MAX id (unflagged)", [&]() {
          ptr_t p = create_node_ptr(MAX_LABEL, MAX_ID);
          AssertThat(id_of(p), Is().EqualTo(MAX_ID));
        });

        it("should store and retrieve 42 id (flagged)", [&]() {
          ptr_t p = flag(create_node_ptr(2,42));
          AssertThat(id_of(p), Is().EqualTo(42u));
        });

        it("should store and retrieve 21 id (flagged)", [&]() {
          ptr_t p = flag(create_node_ptr(2,21));
          AssertThat(id_of(p), Is().EqualTo(21u));
        });

        it("should store and retrieve MAX id (flagged)", [&]() {
          ptr_t p = create_node_ptr(MAX_LABEL, MAX_ID);
          AssertThat(id_of(p), Is().EqualTo(MAX_ID));
        });

        it("should create uid without being flagged", [&]() {
          adiar::uid_t n_uid = create_node_uid(53, 4);
          AssertThat(is_flagged(n_uid), Is().False());

          n_uid = create_node_uid(42, 9);
          AssertThat(is_flagged(n_uid), Is().False());
        });
      });

      describe("ordering ( < )", [&]() {
        it("should sort by label, then by id", [&]() {
          ptr_t node_1_2 = create_node_ptr(1,2);
          ptr_t node_2_1 = create_node_ptr(2,1);
          ptr_t node_2_2 = create_node_ptr(2,2);

          AssertThat(node_1_2 < node_2_1, Is().True());
          AssertThat(node_2_1 < node_2_2, Is().True());
        });

        it("should sort Sink arcs after Node Ptr (unflagged)", [&]() {
          // Create a node pointer with the highest possible raw value
          ptr_t p_node = create_node_ptr(MAX_LABEL,MAX_ID);

          // Create a sink pointer with the lowest raw value
          ptr_t p_sink = create_sink_ptr(false);

          AssertThat(p_node < p_sink, Is().True());
          AssertThat(flag(p_node) < p_sink, Is().True());
        });

        it("should sort Sink arcs after Node Ptr (flagged)", [&]() {
          // Create a node pointer with the highest possible raw value
          ptr_t p_node = flag(create_node_ptr(MAX_LABEL,MAX_ID));

          // Create a sink pointer with the lowest raw value
          ptr_t p_sink = create_sink_ptr(false);

          AssertThat(p_node < p_sink, Is().True());
          AssertThat(flag(p_node) < p_sink, Is().True());
        });
      });

      describe("is_node", [&]() {
        it("should recognise Node Ptr (unflagged)", [&]() {
          ptr_t p_node_max = create_node_ptr(MAX_LABEL,MAX_ID);
          AssertThat(is_node(p_node_max), Is().True());

          ptr_t p_node_min = create_node_ptr(0,0);
          AssertThat(is_node(p_node_min), Is().True());

          ptr_t p_node = create_node_ptr(42,18);
          AssertThat(is_node(p_node), Is().True());
        });

        it("should recognise Node Ptr (flagged)", [&]() {
          ptr_t p_node_max = flag(create_node_ptr(MAX_LABEL,MAX_ID));
          AssertThat(is_node(p_node_max), Is().True());

          ptr_t p_node_min = flag(create_node_ptr(0,0));
          AssertThat(is_node(p_node_min), Is().True());

          ptr_t p_node = flag(create_node_ptr(42,18));
          AssertThat(is_node(p_node), Is().True());
        });

        it("should not be confused with Sinks", [&]() {
          ptr_t sink_f = create_sink_ptr(false);
          ptr_t sink_t = create_sink_ptr(true);

          AssertThat(is_node(sink_f), Is().False());
          AssertThat(is_node(sink_t), Is().False());
        });

        it("should not be confused with Nil (unflagged)", [&]() {
          AssertThat(is_node(NIL), Is().False());
        });

        it("should not be confused with Nil (flagged)", [&]() {
          AssertThat(is_node(flag(NIL)), Is().False());
        });
      });
    });

    describe("node_t", [&]() {
      it("should be a POD", [&]() {
        AssertThat(std::is_pod<node>::value, Is().True());
      });

      it("should take up 24 bytes of memory", [&]() {
        ptr_t node_ptr = create_node_ptr(42,2);
        ptr_t sink = create_sink_ptr(false);
        node_t node = create_node(1, 8, node_ptr, sink);

        AssertThat(sizeof(node), Is().EqualTo(3u * 8u));
      });

      describe("create_node, label_of, id_of", [&]() {
        it("should create node [label_t, id_t, ptr_t, ptr_t] [1]", [&]() {
          ptr_t sink_f = create_sink_ptr(false);
          ptr_t sink_t = create_sink_ptr(true);

          node_t n1 = create_node(3,12,sink_f,sink_t);
          AssertThat(n1.uid, Is().EqualTo(create_node_ptr(3,12)));
          AssertThat(label_of(n1), Is().EqualTo(3u));
          AssertThat(id_of(n1), Is().EqualTo(12u));

          AssertThat(n1.low, Is().EqualTo(sink_f));
          AssertThat(n1.high, Is().EqualTo(sink_t));
        });

        it("should create node [label_t, id_t, ptr_t, ptr_t] [2]", [&]() {
          ptr_t sink_f = create_sink_ptr(false);
          ptr_t sink_t = create_sink_ptr(true);

          node_t n2 = create_node(3,42,sink_t,sink_f);
          AssertThat(n2.uid, Is().EqualTo(create_node_ptr(3,42)));
          AssertThat(label_of(n2), Is().EqualTo(3u));
          AssertThat(id_of(n2), Is().EqualTo(42u));

          AssertThat(n2.low, Is().EqualTo(sink_t));
          AssertThat(n2.high, Is().EqualTo(sink_f));
        });

        it("should create node [label_t, id_t, node_t&, node_t&]", [&]() {
          ptr_t sink_f = create_sink_ptr(false);
          ptr_t sink_t = create_sink_ptr(true);

          node_t n_child1 = create_node(3,12,sink_f,sink_t);
          node_t n_child2 = create_node(3,42,sink_t,sink_f);

          node_t n = create_node(2,2,n_child1,n_child2);
          AssertThat(n.uid, Is().EqualTo(create_node_ptr(2,2)));
          AssertThat(label_of(n), Is().EqualTo(2u));
          AssertThat(id_of(n), Is().EqualTo(2u));

          AssertThat(n.low, Is().EqualTo(n_child1.uid));
          AssertThat(n.high, Is().EqualTo(n_child2.uid));
        });

        it("should create node [label_t, id_t, node_t&, ptr_t]", [&]() {
          ptr_t sink_f = create_sink_ptr(false);
          ptr_t sink_t = create_sink_ptr(true);

          node_t n_child = create_node(2,2,sink_f,sink_t);

          node_t n = create_node(1,7,sink_t,n_child);
          AssertThat(n.uid, Is().EqualTo(create_node_ptr(1,7)));
          AssertThat(label_of(n), Is().EqualTo(1u));
          AssertThat(id_of(n), Is().EqualTo(7u));

          AssertThat(n.low, Is().EqualTo(sink_t));
          AssertThat(n.high, Is().EqualTo(n_child.uid));
        });

        it("should create node [label_t, id_t, ptr_t, node_t&]", [&]() {
          ptr_t sink_f = create_sink_ptr(false);
          ptr_t sink_t = create_sink_ptr(true);

          node_t n_child = create_node(2,2,sink_f,sink_t);

          node_t n = create_node(0,3,sink_t,n_child);
          AssertThat(n.uid, Is().EqualTo(create_node_ptr(0,3)));
          AssertThat(label_of(n), Is().EqualTo(0u));
          AssertThat(id_of(n), Is().EqualTo(3u));

          AssertThat(n.low, Is().EqualTo(sink_t));
          AssertThat(n.high, Is().EqualTo(n_child.uid));
        });
      });

      it("should sort by label, then by id", [&]() {
        auto sink_f = create_sink_ptr(false);
        auto sink_t = create_sink_ptr(true);

        auto node_1_2 = create_node(1,2,sink_f,sink_t);
        auto node_2_1 = create_node(2,1,sink_t,sink_f);

        AssertThat(node_1_2 < node_2_1, Is().True());
        AssertThat(node_2_1 > node_1_2, Is().True());

        auto node_2_2 = create_node(2,2,sink_f,sink_f);

        AssertThat(node_2_1 < node_2_2, Is().True());
        AssertThat(node_2_2 > node_2_1, Is().True());
      });

      it("should be equal by their content", [&]() {
        auto sink_f = create_sink_ptr(false);
        auto sink_t = create_sink_ptr(true);

        auto node_1_v1 = create_node(42,2,sink_f,sink_t);
        auto node_1_v2 = create_node(42,2,sink_f,sink_t);

        AssertThat(node_1_v1 == node_1_v2, Is().True());
        AssertThat(node_1_v1 != node_1_v2, Is().False());
      });

      it("should be unequal by their content", [&]() {
        auto sink_f = create_sink_ptr(false);
        auto sink_t = create_sink_ptr(true);

        auto node_1 = create_node(42,2,sink_f,sink_t);
        auto node_2 = create_node(42,2,sink_f,sink_f);
        auto node_3 = create_node(42,3,sink_f,sink_t);
        auto node_4 = create_node(21,2,sink_f,sink_t);

        AssertThat(node_1 == node_2, Is().False());
        AssertThat(node_1 != node_2, Is().True());

        AssertThat(node_1 == node_3, Is().False());
        AssertThat(node_1 != node_3, Is().True());

        AssertThat(node_1 == node_4, Is().False());
        AssertThat(node_1 != node_4, Is().True());
      });

      describe("sink nodes", [&]() {
        describe("is_sink", [&]() {
          it("accepts true sink", [&]() {
            node_t sink_node_T = create_sink(true);
            AssertThat(is_sink(sink_node_T), Is().True());
          });

          it("accepts false sink", [&]() {
            node_t sink_node_F = create_sink(false);
            AssertThat(is_sink(sink_node_F), Is().True());
          });

          it("rejects non-sink nodes [1]", [&]() {
            node_t node_1 = create_node(42,2, create_sink_ptr(false), create_sink_ptr(true));
            AssertThat(is_sink(node_1), Is().False());
          });

          it("rejects non-sink nodes [2]", [&]() {
            node_t almost_F_sink = create_node(0,0,
                                               create_sink_ptr(true),
                                               create_node_ptr(42,2));
            AssertThat(is_sink(almost_F_sink), Is().False());
          });

          it("rejects non-sink nodes [2]", [&]() {
            node_t almost_T_sink = create_node(0,1,
                                               create_sink_ptr(true),
                                               create_node_ptr(42,2));
            AssertThat(is_sink(almost_T_sink), Is().False());
          });
        });

        describe("value_of", [&]() {
          it("retrives value of a true sink node", [&]() {
            node_t sink_node_T = create_sink(true);
            AssertThat(value_of(sink_node_T), Is().True());
          });

          it("retrives value of a false sink node", [&]() {
            node_t sink_node_F = create_sink(false);
            AssertThat(value_of(sink_node_F), Is().False());
          });
        });

        describe("is_false", [&]() {
          it("rejects true sink", [&]() {
            node_t sink_node_T = create_sink(true);
            AssertThat(is_false(sink_node_T), Is().False());
          });

          it("accepts false sink", [&]() {
            node_t sink_node_F = create_sink(false);
            AssertThat(is_false(sink_node_F), Is().True());
          });

          it("rejects non-sink nodes", [&]() {
            node_t n = create_node(0,0,create_node_ptr(42,2),create_sink_ptr(false));
            AssertThat(is_false(n), Is().False());
          });
        });

        describe("is_true", [&]() {
          it("accepts true sink", [&]() {
            node_t sink_node_T = create_sink(true);
            AssertThat(is_true(sink_node_T), Is().True());
          });

          it("rejects false sink", [&]() {
            node_t sink_node_F = create_sink(false);
            AssertThat(is_true(sink_node_F), Is().False());
          });

          it("rejects non-sink nodes", [&]() {
            node_t n = create_node(0,1,create_sink_ptr(true),create_node_ptr(2,3));
            AssertThat(is_true(n), Is().False());
          });
        });
      });

      describe("negate (!)", [&]() {
        it("should leave node_ptr children unchanged", [&]() {
          auto node = create_node(2,2,
                                  create_node_ptr(42,3),
                                  create_node_ptr(8,2));

          AssertThat(!node, Is().EqualTo(node));
        });

        it("should negate sink_ptr child", [&]() {
          auto node = create_node(2,2,
                                  create_sink_ptr(false),
                                  create_node_ptr(8,2));

          AssertThat(!node, Is().EqualTo(create_node(2,2,
                                                     create_sink_ptr(true),
                                                     create_node_ptr(8,2))));
        });

        it("should negate sink_ptr children while preserving flags", [&]() {
          auto node = create_node(2,2,
                                  create_sink_ptr(false),
                                  flag(create_sink_ptr(true)));

          AssertThat(!node, Is().EqualTo(create_node(2,2,
                                                     create_sink_ptr(true),
                                                     flag(create_sink_ptr(false)))));
        });

        it("should negate sink node", [&]() {
          AssertThat(!create_sink(true), Is().EqualTo(create_sink(false)));
          AssertThat(!create_sink(false), Is().EqualTo(create_sink(true)));
        });
      });
    });

    describe("arc_t", [&]() {
      it("should be equal by their content", [&]() {
        ptr_t source = create_node_ptr(4,2);
        ptr_t target = create_node_ptr(42,3);

        arc_t arc_1 = { source, target };
        arc_t arc_2 = { source, target };

        AssertThat(arc_1 == arc_2, Is().True());
        AssertThat(arc_1 != arc_2, Is().False());
      });

      it("should unequal by their content", [&]() {
        ptr_t node_ptr_1 = create_node_ptr(4,2);
        ptr_t node_ptr_2 = create_node_ptr(4,3);
        ptr_t node_ptr_3 = create_node_ptr(3,2);

        arc_t arc_1 = { node_ptr_1, node_ptr_2 };
        arc_t arc_2 = { node_ptr_1, node_ptr_3 };

        AssertThat(arc_1 == arc_2, Is().False());
        AssertThat(arc_1 != arc_2, Is().True());

        arc_t arc_3 = { node_ptr_1, node_ptr_2 };
        arc_t arc_4 = { flag(node_ptr_1), node_ptr_2 };

        AssertThat(arc_3 == arc_4, Is().False());
        AssertThat(arc_3 != arc_4, Is().True());

        arc_t arc_5 = { node_ptr_1, node_ptr_2 };
        arc_t arc_6 = { node_ptr_3, node_ptr_2 };

        AssertThat(arc_5 == arc_6, Is().False());
        AssertThat(arc_5 != arc_6, Is().True());
      });

      it("should recognise low arcs from bit-flag on source", [&]() {
        ptr_t node_ptr_1 = create_node_ptr(4,2);
        ptr_t node_ptr_2 = create_node_ptr(4,3);

        arc_t arc_low = { node_ptr_1, node_ptr_2 };
        AssertThat(is_high(arc_low), Is().False());
      });

      it("should recognise high arcs from bit-flag on source", [&]() {
        ptr_t node_ptr_1 = create_node_ptr(4,2);
        ptr_t node_ptr_2 = create_node_ptr(4,3);

        arc_t arc_high = { flag(node_ptr_1), node_ptr_2 };
        AssertThat(is_high(arc_high), Is().True());
      });

      it("should leave node_ptr target unchanged", [&]() {
        arc_t a = { create_node_ptr(1,0), create_node_ptr(2,0) };
        AssertThat(!a, Is().EqualTo(a));
      });

      it("should negate unflagged sink_ptr target", [&]() {
        arc_t a = { create_node_ptr(1,0), create_sink_ptr(true) };
        AssertThat(!a, Is().EqualTo(arc { create_node_ptr(1,0), create_sink_ptr(false) }));
      });

      it("should negate flagged sink_ptr target", [&]() {
        arc_t a = { create_node_ptr(1,0), flag(create_sink_ptr(false)) };
        AssertThat(!a, Is().EqualTo(arc { create_node_ptr(1,0), flag(create_sink_ptr(true)) }));
      });

      it("should be a POD", [&]() {
        AssertThat(std::is_pod<arc_t>::value, Is().True());
      });

      it("should take up 16 bytes of memory", [&]() {
        ptr_t node_ptr = create_node_ptr(42,2);
        ptr_t sink = create_sink_ptr(false);
        arc_t arc = { node_ptr, sink };

        AssertThat(sizeof(arc), Is().EqualTo(2u * 8u));
      });
    });

    describe("low_arc_of, high_arc_of, node_of", [&]() {
      it("should extract low arc from node", [&]() {
        node_t node = create_node(7,42,
                                  create_node_ptr(8,21),
                                  create_node_ptr(9,8));

        arc_t arc = low_arc_of(node);

        AssertThat(label_of(arc.source), Is().EqualTo(7u));
        AssertThat(id_of(arc.source), Is().EqualTo(42u));
        AssertThat(label_of(arc.target), Is().EqualTo(8u));
        AssertThat(id_of(arc.target), Is().EqualTo(21u));
      });

      it("should extract high arc from node", [&]() {
        node_t node = create_node(6,13,
                                  create_node_ptr(8,21),
                                  create_node_ptr(9,8));

        arc_t arc = high_arc_of(node);

        AssertThat(label_of(arc.source), Is().EqualTo(6u));
        AssertThat(id_of(arc.source), Is().EqualTo(13u));
        AssertThat(label_of(arc.target), Is().EqualTo(9u));
        AssertThat(id_of(arc.target), Is().EqualTo(8u));
      });

      it("should combine low and high arcs into single node", [&]() {
        arc_t low_arc = { create_node_ptr(17,42), create_node_ptr(9,8) };
        arc_t high_arc = { flag(create_node_ptr(17,42)), create_node_ptr(8,21) };

        node_t node = node_of(low_arc, high_arc);

        AssertThat(label_of(node), Is().EqualTo(17u));
        AssertThat(id_of(node), Is().EqualTo(42u));

        AssertThat(label_of(node.low), Is().EqualTo(label_of(low_arc.target)));
        AssertThat(id_of(node.low), Is().EqualTo(id_of(low_arc.target)));

        AssertThat(label_of(node.high), Is().EqualTo(label_of(high_arc.target)));
        AssertThat(id_of(node.high), Is().EqualTo(id_of(high_arc.target)));
      });
    });

    describe("assignment_t", [&]() {
      assignment_t a1 = create_assignment(2, false);
      assignment_t a2 = create_assignment(2, true);
      assignment_t a3 = create_assignment(3, false);

      it("is sorted first by label", [&]() {
        // Less than
        AssertThat(a1 < a3, Is().True());
        AssertThat(a2 < a3, Is().True());
        AssertThat(a3 < a1, Is().False());
        AssertThat(a3 < a2, Is().False());

        // Greater than
        AssertThat(a3 > a1, Is().True());
        AssertThat(a3 > a2, Is().True());
        AssertThat(a1 > a3, Is().False());
        AssertThat(a2 > a3, Is().False());
      });

      it("is not sorted by value second", [&]() {
        // Less than
        AssertThat(a1 < a2, Is().False());

        // Greater than
        AssertThat(a2 > a1, Is().False());
      });

      assignment_t b1 = create_assignment(2, false);
      assignment_t b2 = create_assignment(2, true);
      assignment_t b3 = create_assignment(3, false);

      it("should be equal by content", [&]() {
        AssertThat(a1 == b1, Is().True());
        AssertThat(a2 == b2, Is().True());
        AssertThat(a3 == b3, Is().True());
        AssertThat(a2 == b1, Is().False());
        AssertThat(a3 == b1, Is().False());
        AssertThat(a3 == b2, Is().False());
      });

      it("should be equal by content", [&]() {
        AssertThat(a2 != b1, Is().True());
        AssertThat(a3 != b1, Is().True());
        AssertThat(a3 != b2, Is().True());
        AssertThat(a1 != b1, Is().False());
        AssertThat(a2 != b2, Is().False());
        AssertThat(a3 != b3, Is().False());
      });

      it("can be negated on its value", [&]() {
        AssertThat(!a1, Is().EqualTo(create_assignment(2, true)));
        AssertThat(!a2, Is().EqualTo(create_assignment(2, false)));
        AssertThat(!a3, Is().EqualTo(create_assignment(3, true)));
      });
    });
  });
 });
