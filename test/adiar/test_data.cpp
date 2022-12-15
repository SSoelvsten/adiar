go_bandit([]() {
  describe("adiar/data.h", []() {

    describe("NIL", [&](){
      it("should recognise NIL (unflagged)", [&]() {
        const ptr_t some_value = NIL;
        AssertThat(is_nil(some_value), Is().True());
      });

      it("should recognise NIL (flagged)", [&]() {
        const ptr_t some_value = flag(NIL);
        AssertThat(is_nil(some_value), Is().True());
      });


      it("can see whether the flag is set", [&]() {
        AssertThat(is_flagged(flag(NIL)), Is().True());
      });

      it("can see whether the flag is not set", [&]() {
        AssertThat(is_flagged(NIL), Is().False());
      });
    });

    describe("ptr_t (terminals)", [&](){
      it("should take up 8 bytes of memory", [&]() {
        const ptr_t terminal = create_terminal_ptr(false);
        AssertThat(sizeof(terminal), Is().EqualTo(8u));
      });

      describe("flag, unflag, is_flagged", []() {
        it("is unflagged by default", [&]() {
          ptr_t p = create_terminal_ptr(true);
          AssertThat(is_flagged(p), Is().False());

          p = create_terminal_ptr(true);
          AssertThat(is_flagged(p), Is().False());
        });

        it("can set the flag", [&]() {
          ptr_t p = flag(create_terminal_ptr(false));
          AssertThat(is_flagged(p), Is().True());

          p = flag(create_terminal_ptr(true));
          AssertThat(is_flagged(p), Is().True());
        });

        it("can unset the flag", [&]() {
          ptr_t p = flag(create_terminal_ptr(false));
          p = unflag(p);
          AssertThat(is_flagged(p), Is().False());

          p = flag(create_terminal_ptr(true));
          p = unflag(p);
          AssertThat(is_flagged(p), Is().False());
        });
      });

      describe("is_terminal", []() {
        it("should recognise Sinks as such", [&]() {
          const ptr_t terminal_F = create_terminal_ptr(false);
          const ptr_t terminal_T = create_terminal_ptr(true);

          AssertThat(is_terminal(terminal_F), Is().True());
          AssertThat(is_terminal(terminal_T), Is().True());
        });

        it("should not be confused with Node Ptr (unflagged)", [&]() {
          ptr_t arc_node_max = create_node_ptr(max_var,MAX_ID);
          AssertThat(is_terminal(arc_node_max), Is().False());
          AssertThat(is_false(arc_node_max), Is().False());
          AssertThat(is_true(arc_node_max), Is().False());

          ptr_t arc_node_min = create_node_ptr(0,0);
          AssertThat(is_terminal(arc_node_min), Is().False());
          AssertThat(is_false(arc_node_min), Is().False());
          AssertThat(is_true(arc_node_min), Is().False());

          ptr_t arc_node = create_node_ptr(42,18);
          AssertThat(is_terminal(arc_node), Is().False());
          AssertThat(is_false(arc_node), Is().False());
          AssertThat(is_true(arc_node), Is().False());
        });

        it("should not be confused with Node Ptr (flagged)", [&]() {
          ptr_t arc_node_max = flag(create_node_ptr(max_var,MAX_ID));
          AssertThat(is_terminal(arc_node_max), Is().False());
          AssertThat(is_false(arc_node_max), Is().False());
          AssertThat(is_true(arc_node_max), Is().False());

          ptr_t arc_node_min = flag(create_node_ptr(0,0));
          AssertThat(is_terminal(arc_node_min), Is().False());
          AssertThat(is_false(arc_node_min), Is().False());
          AssertThat(is_true(arc_node_min), Is().False());

          ptr_t arc_node = flag(create_node_ptr(42,18));
          AssertThat(is_terminal(arc_node), Is().False());
          AssertThat(is_false(arc_node), Is().False());
          AssertThat(is_true(arc_node), Is().False());
        });

        it("should not be confused with Nil (unflagged)", [&]() {
          AssertThat(is_terminal(NIL), Is().False());
        });

        it("should not be confused with Nil (flagged)", [&]() {
          AssertThat(is_terminal(flag(NIL)), Is().False());
        });
      });

      describe("value_of", []() {
        it("retrieves value from false terminal", [&]() {
          ptr_t p = create_terminal_ptr(false);
          AssertThat(value_of(p), Is().False());
        });

        it("retrieves value from true terminal", [&]() {
          ptr_t p = create_terminal_ptr(true);
          AssertThat(value_of(p), Is().True());
        });
      });

      describe("is_false", []() {
        it("should accept false terminal", [&]() {
          ptr_t terminal_F = create_terminal_ptr(false);
          AssertThat(is_false(terminal_F), Is().True());
        });

        it("should accept false terminal (flagged)", [&]() {
          ptr_t terminal_F = flag(create_terminal_ptr(false));
          AssertThat(is_false(terminal_F), Is().True());
        });

        it("should reject terminal", [&]() {
          ptr_t terminal_T = create_terminal_ptr(true);
          AssertThat(is_false(terminal_T), Is().False());
        });

        it("should reject non-terminal", [&]() {
          ptr_t p = create_node_ptr(0,0);
          AssertThat(is_false(p), Is().False());
        });

        it("should reject NIL", [&]() {
          ptr_t p = NIL;
          AssertThat(is_false(p), Is().False());
        });
      });

      describe("is_true", []() {
        it("should reject false terminal", [&]() {
          ptr_t terminal_F = create_terminal_ptr(false);
          AssertThat(is_true(terminal_F), Is().False());
        });

        it("should accept true terminal", [&]() {
          ptr_t terminal_T = create_terminal_ptr(true);
          AssertThat(is_true(terminal_T), Is().True());
        });

        it("should accept true terminal (flagged)", [&]() {
          ptr_t terminal_T = flag(create_terminal_ptr(true));
          AssertThat(is_true(terminal_T), Is().True());
        });

        it("should reject non-terminal", [&]() {
          ptr_t p = create_node_ptr(0,1);
          AssertThat(is_true(p), Is().False());
        });

        it("should reject NIL", [&]() {
          ptr_t p = NIL;
          AssertThat(is_true(p), Is().False());
        });
      });

      describe("negate", []() {
        it("should negate terminal (unflagged)", [&]() {
          ptr_t p1 = create_terminal_ptr(false);
          AssertThat(negate(p1), Is().EqualTo(create_terminal_ptr(true)));

          ptr_t p2 = create_terminal_ptr(true);
          AssertThat(negate(p2), Is().EqualTo(create_terminal_ptr(false)));
        });

        it("should negate terminal into terminal (flagged)", [&]() {
          ptr_t p1 = flag(create_terminal_ptr(false));
          AssertThat(negate(p1), Is().EqualTo(flag(create_terminal_ptr(true))));

          ptr_t p2 = flag(create_terminal_ptr(true));
          AssertThat(negate(p2), Is().EqualTo(flag(create_terminal_ptr(false))));
        });
      });

      describe("bool_op", []() {
        const ptr_t terminal_F = create_terminal_ptr(false);
        const ptr_t terminal_T = create_terminal_ptr(true);

        it("and_op", [&]() {
          AssertThat(and_op(terminal_T, terminal_T), Is().EqualTo(terminal_T));
          AssertThat(and_op(terminal_T, terminal_F), Is().EqualTo(terminal_F));
          AssertThat(and_op(terminal_F, terminal_T), Is().EqualTo(terminal_F));
          AssertThat(and_op(terminal_F, terminal_F), Is().EqualTo(terminal_F));
        });

        it("nand_op", [&]() {
          AssertThat(nand_op(terminal_T, terminal_T), Is().EqualTo(terminal_F));
          AssertThat(nand_op(terminal_T, terminal_F), Is().EqualTo(terminal_T));
          AssertThat(nand_op(terminal_F, terminal_T), Is().EqualTo(terminal_T));
          AssertThat(nand_op(terminal_F, terminal_F), Is().EqualTo(terminal_T));
        });

        it("or_op", [&]() {
          AssertThat(or_op(terminal_T, terminal_T), Is().EqualTo(terminal_T));
          AssertThat(or_op(terminal_T, terminal_F), Is().EqualTo(terminal_T));
          AssertThat(or_op(terminal_F, terminal_T), Is().EqualTo(terminal_T));
          AssertThat(or_op(terminal_F, terminal_F), Is().EqualTo(terminal_F));
        });

        it("nor_op", [&]() {
          AssertThat(nor_op(terminal_T, terminal_T), Is().EqualTo(terminal_F));
          AssertThat(nor_op(terminal_T, terminal_F), Is().EqualTo(terminal_F));
          AssertThat(nor_op(terminal_F, terminal_T), Is().EqualTo(terminal_F));
          AssertThat(nor_op(terminal_F, terminal_F), Is().EqualTo(terminal_T));
        });

        it("xor_op", [&]() {
          AssertThat(xor_op(terminal_T, terminal_T), Is().EqualTo(terminal_F));
          AssertThat(xor_op(terminal_T, terminal_F), Is().EqualTo(terminal_T));
          AssertThat(xor_op(terminal_F, terminal_T), Is().EqualTo(terminal_T));
          AssertThat(xor_op(terminal_F, terminal_F), Is().EqualTo(terminal_F));
        });

        it("xnor_op", [&]() {
          AssertThat(xnor_op(terminal_T, terminal_T), Is().EqualTo(terminal_T));
          AssertThat(xnor_op(terminal_T, terminal_F), Is().EqualTo(terminal_F));
          AssertThat(xnor_op(terminal_F, terminal_T), Is().EqualTo(terminal_F));
          AssertThat(xnor_op(terminal_F, terminal_F), Is().EqualTo(terminal_T));
        });

        it("imp_op", [&]() {
          AssertThat(imp_op(terminal_T, terminal_T), Is().EqualTo(terminal_T));
          AssertThat(imp_op(terminal_T, terminal_F), Is().EqualTo(terminal_F));
          AssertThat(imp_op(terminal_F, terminal_T), Is().EqualTo(terminal_T));
          AssertThat(imp_op(terminal_F, terminal_F), Is().EqualTo(terminal_T));
        });

        it("invimp_op", [&]() {
          AssertThat(invimp_op(terminal_T, terminal_T), Is().EqualTo(terminal_T));
          AssertThat(invimp_op(terminal_T, terminal_F), Is().EqualTo(terminal_T));
          AssertThat(invimp_op(terminal_F, terminal_T), Is().EqualTo(terminal_F));
          AssertThat(invimp_op(terminal_F, terminal_F), Is().EqualTo(terminal_T));
        });

        it("equiv_op", [&]() {
          AssertThat(equiv_op(terminal_T, terminal_T), Is().EqualTo(terminal_T));
          AssertThat(equiv_op(terminal_T, terminal_F), Is().EqualTo(terminal_F));
          AssertThat(equiv_op(terminal_F, terminal_T), Is().EqualTo(terminal_F));
          AssertThat(equiv_op(terminal_F, terminal_F), Is().EqualTo(terminal_T));
        });

        it("equiv_op (flags)", [&]() {
          AssertThat(equiv_op(flag(terminal_T), terminal_T), Is().EqualTo(terminal_T));
          AssertThat(equiv_op(terminal_T, flag(terminal_F)), Is().EqualTo(terminal_F));
          AssertThat(equiv_op(flag(terminal_F), terminal_T), Is().EqualTo(terminal_F));
          AssertThat(equiv_op(terminal_F, flag(terminal_F)), Is().EqualTo(terminal_T));
        });

        it("diff_op", [&]() {
          AssertThat(diff_op(terminal_T, terminal_T), Is().EqualTo(terminal_F));
          AssertThat(diff_op(terminal_T, terminal_F), Is().EqualTo(terminal_T));
          AssertThat(diff_op(terminal_F, terminal_T), Is().EqualTo(terminal_F));
          AssertThat(diff_op(terminal_F, terminal_F), Is().EqualTo(terminal_F));
        });

        it("less_op", [&]() {
          AssertThat(less_op(terminal_T, terminal_T), Is().EqualTo(terminal_F));
          AssertThat(less_op(terminal_T, terminal_F), Is().EqualTo(terminal_F));
          AssertThat(less_op(terminal_F, terminal_T), Is().EqualTo(terminal_T));
          AssertThat(less_op(terminal_F, terminal_F), Is().EqualTo(terminal_F));
        });

        describe("can_shortcut", [&]() {
          it("can check on T terminal on the left", [&]() {
            AssertThat(can_left_shortcut(and_op, terminal_T), Is().False());
            AssertThat(can_left_shortcut(or_op, terminal_T), Is().True());
            AssertThat(can_left_shortcut(xor_op, terminal_T), Is().False());
            AssertThat(can_left_shortcut(imp_op, terminal_T), Is().False());
          });

          it("can check on F terminal on the left", [&]() {
            AssertThat(can_left_shortcut(and_op, terminal_F), Is().True());
            AssertThat(can_left_shortcut(or_op, terminal_F), Is().False());
            AssertThat(can_left_shortcut(xor_op, terminal_F), Is().False());
            AssertThat(can_left_shortcut(imp_op, terminal_F), Is().True());
          });

          it("can check on T terminal on the right", [&]() {
            AssertThat(can_right_shortcut(and_op, terminal_T), Is().False());
            AssertThat(can_right_shortcut(or_op, terminal_T), Is().True());
            AssertThat(can_right_shortcut(xor_op, terminal_T), Is().False());
            AssertThat(can_right_shortcut(imp_op, terminal_T), Is().True());
          });

          it("can check on F terminal on the right", [&]() {
            AssertThat(can_right_shortcut(and_op, terminal_F), Is().True());
            AssertThat(can_right_shortcut(or_op, terminal_F), Is().False());
            AssertThat(can_right_shortcut(xor_op, terminal_F), Is().False());
            AssertThat(can_right_shortcut(imp_op, terminal_F), Is().False());
          });
        });

        describe("is_irrelevant", [&]() {
          it("can check on T terminal on the left", [&]() {
            AssertThat(is_left_irrelevant(and_op, terminal_T), Is().True());
            AssertThat(is_left_irrelevant(or_op, terminal_T), Is().False());
            AssertThat(is_left_irrelevant(xor_op, terminal_T), Is().False());
            AssertThat(is_left_irrelevant(imp_op, terminal_T), Is().True());
          });

          it("can check on F terminal on the left", [&]() {
            AssertThat(is_left_irrelevant(and_op, terminal_F), Is().False());
            AssertThat(is_left_irrelevant(or_op, terminal_F), Is().True());
            AssertThat(is_left_irrelevant(xor_op, terminal_F), Is().True());
            AssertThat(is_left_irrelevant(imp_op, terminal_F), Is().False());
          });

          it("can check on T terminal on the right", [&]() {
            AssertThat(is_right_irrelevant(and_op, terminal_T), Is().True());
            AssertThat(is_right_irrelevant(or_op, terminal_T), Is().False());
            AssertThat(is_right_irrelevant(xor_op, terminal_T), Is().False());
            AssertThat(is_right_irrelevant(imp_op, terminal_T), Is().False());
          });

          it("can check on F terminal on the right", [&]() {
            AssertThat(is_right_irrelevant(and_op, terminal_F), Is().False());
            AssertThat(is_right_irrelevant(or_op, terminal_F), Is().True());
            AssertThat(is_right_irrelevant(xor_op, terminal_F), Is().True());
            AssertThat(is_right_irrelevant(imp_op, terminal_F), Is().False());
          });
        });

        describe("is_negating", [&]() {
          it("can check on T terminal on the left", [&]() {
            AssertThat(is_left_negating(and_op, terminal_T), Is().False());
            AssertThat(is_left_negating(or_op, terminal_T), Is().False());
            AssertThat(is_left_negating(xor_op, terminal_T), Is().True());
            AssertThat(is_left_negating(imp_op, terminal_T), Is().False());
          });

          it("can check on F terminal on the left", [&]() {
            AssertThat(is_left_negating(and_op, terminal_F), Is().False());
            AssertThat(is_left_negating(or_op, terminal_F), Is().False());
            AssertThat(is_left_negating(xor_op, terminal_F), Is().False());
            AssertThat(is_left_negating(imp_op, terminal_F), Is().False());
          });

          it("can check on T terminal on the right", [&]() {
            AssertThat(is_right_negating(and_op, terminal_T), Is().False());
            AssertThat(is_right_negating(or_op, terminal_T), Is().False());
            AssertThat(is_right_negating(xor_op, terminal_T), Is().True());
            AssertThat(is_right_negating(imp_op, terminal_T), Is().False());
          });

          it("can check on F terminal on the right", [&]() {
            AssertThat(is_right_negating(and_op, terminal_F), Is().False());
            AssertThat(is_right_negating(or_op, terminal_F), Is().False());
            AssertThat(is_right_negating(xor_op, terminal_F), Is().False());
            AssertThat(is_right_negating(imp_op, terminal_F), Is().False());
          });
        });

        it("can check the operators for being commutative", []() {
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
      const ptr_t terminal_F = create_terminal_ptr(false);
      const ptr_t terminal_T = create_terminal_ptr(true);

      it("should take up 8 bytes of memory", [&]() {
        const ptr_t node_ptr = create_node_ptr(42,2);
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
          const ptr_t p = create_node_ptr(12,MAX_ID);
          AssertThat(label_of(p), Is().EqualTo(12u));
        });

        it("should store and retrieve 42 label Ptr (unflagged)", [&]() {
          const ptr_t p = create_node_ptr(42,2);
          AssertThat(label_of(p), Is().EqualTo(42u));
        });

        it("should store and retrieve 21 label Ptr (unflagged)", [&]() {
          const ptr_t p = create_node_ptr(21,2);
          AssertThat(label_of(p), Is().EqualTo(21u));
        });

        it("should store and retrieve MAX label Ptr (unflagged)", [&]() {
          const ptr_t p = create_node_ptr(max_var, MAX_ID);
          AssertThat(label_of(p), Is().EqualTo(max_var));
        });

        it("should store and retrieve label for Ptr with maximal id (flagged)", [&]() {
          const ptr_t p = flag(create_node_ptr(12,MAX_ID));
          AssertThat(label_of(p), Is().EqualTo(12u));
        });

        it("should store and retrieve 42 label Ptr (flagged)", [&]() {
          const ptr_t p = flag(create_node_ptr(42,2));
          AssertThat(label_of(p), Is().EqualTo(42u));
        });

        it("should store and retrieve 21 label Ptr (flagged)", [&]() {
          const ptr_t p = flag(create_node_ptr(21,2));
          AssertThat(label_of(p), Is().EqualTo(21u));
        });

        it("should store and retrieve MAX label Ptr (flagged)", [&]() {
          const ptr_t p = create_node_ptr(max_var, MAX_ID);
          AssertThat(label_of(p), Is().EqualTo(max_var));
        });

        it("should store and retrieve 42 id (unflagged)", [&]() {
          const ptr_t p = create_node_ptr(2,42);
          AssertThat(id_of(p), Is().EqualTo(42u));
        });

        it("should store and retrieve 21 id (unflagged)", [&]() {
          const ptr_t p = create_node_ptr(2,21);
          AssertThat(id_of(p), Is().EqualTo(21u));
        });

        it("should store and retrieve MAX id (unflagged)", [&]() {
          const ptr_t p = create_node_ptr(max_var, MAX_ID);
          AssertThat(id_of(p), Is().EqualTo(MAX_ID));
        });

        it("should store and retrieve 42 id (flagged)", [&]() {
          const  ptr_t p = flag(create_node_ptr(2,42));
          AssertThat(id_of(p), Is().EqualTo(42u));
        });

        it("should store and retrieve 21 id (flagged)", [&]() {
          const ptr_t p = flag(create_node_ptr(2,21));
          AssertThat(id_of(p), Is().EqualTo(21u));
        });

        it("should store and retrieve MAX id (flagged)", [&]() {
          const ptr_t p = create_node_ptr(max_var, MAX_ID);
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
          const ptr_t node_1_2 = create_node_ptr(1,2);
          const ptr_t node_2_1 = create_node_ptr(2,1);
          const ptr_t node_2_2 = create_node_ptr(2,2);

          AssertThat(node_1_2 < node_2_1, Is().True());
          AssertThat(node_2_1 < node_2_2, Is().True());
        });

        it("should sort F terminal after internal node", [&]() {
          // Create a node pointer with the highest possible raw value
          const ptr_t p_node = create_node_ptr(max_var,MAX_ID);

          AssertThat(p_node < terminal_F, Is().True());
          AssertThat(flag(p_node) < terminal_F, Is().True());
          AssertThat(p_node < flag(terminal_F), Is().True());
          AssertThat(flag(p_node) < flag(terminal_F), Is().True());
        });

        it("should sort T terminal after internal node (unflagged)", [&]() {
          // Create a node pointer with the highest possible raw value
          const ptr_t p_node = create_node_ptr(max_var,MAX_ID);

          AssertThat(p_node < terminal_T, Is().True());
          AssertThat(flag(p_node) < terminal_T, Is().True());
          AssertThat(p_node < flag(terminal_T), Is().True());
          AssertThat(flag(p_node) < flag(terminal_T), Is().True());
        });
      });

      describe("is_node", [&]() {
        it("should recognise Node Ptr (unflagged)", [&]() {
          const ptr_t p_node_max = create_node_ptr(max_var,MAX_ID);
          AssertThat(is_node(p_node_max), Is().True());

          const ptr_t p_node_min = create_node_ptr(0,0);
          AssertThat(is_node(p_node_min), Is().True());

          const ptr_t p_node = create_node_ptr(42,18);
          AssertThat(is_node(p_node), Is().True());
        });

        it("should recognise Node Ptr (flagged)", [&]() {
          const ptr_t p_node_max = flag(create_node_ptr(max_var,MAX_ID));
          AssertThat(is_node(p_node_max), Is().True());

          const ptr_t p_node_min = flag(create_node_ptr(0,0));
          AssertThat(is_node(p_node_min), Is().True());

          const ptr_t p_node = flag(create_node_ptr(42,18));
          AssertThat(is_node(p_node), Is().True());
        });

        it("should not be confused with Sinks", [&]() {
          const ptr_t terminal_F = create_terminal_ptr(false);
          const ptr_t terminal_T = create_terminal_ptr(true);

          AssertThat(is_node(terminal_F), Is().False());
          AssertThat(is_node(terminal_T), Is().False());
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
      const ptr_t terminal_F = create_terminal_ptr(false);
      const ptr_t terminal_T = create_terminal_ptr(true);

      it("should be a POD", [&]() {
        AssertThat(std::is_pod<node>::value, Is().True());
      });

      it("should take up 24 bytes of memory", [&]() {
        const ptr_t node_ptr = create_node_ptr(42,2);
        const node_t node = create_node(1,8, node_ptr, terminal_F);

        AssertThat(sizeof(node), Is().EqualTo(3u * 8u));
      });

      describe("create_node, label_of, id_of", [&]() {
        it("should create node [label_t, id_t, ptr_t, ptr_t] [1]", [&]() {
          const node_t n1 = create_node(3,12, terminal_F, terminal_T);
          AssertThat(n1.uid, Is().EqualTo(create_node_ptr(3,12)));
          AssertThat(label_of(n1), Is().EqualTo(3u));
          AssertThat(id_of(n1), Is().EqualTo(12u));

          AssertThat(n1.low, Is().EqualTo(terminal_F));
          AssertThat(n1.high, Is().EqualTo(terminal_T));
        });

        it("should create node [label_t, id_t, ptr_t, ptr_t] [2]", [&]() {
          const node_t n2 = create_node(3,42, terminal_T, terminal_F);
          AssertThat(n2.uid, Is().EqualTo(create_node_ptr(3,42)));
          AssertThat(label_of(n2), Is().EqualTo(3u));
          AssertThat(id_of(n2), Is().EqualTo(42u));

          AssertThat(n2.low, Is().EqualTo(terminal_T));
          AssertThat(n2.high, Is().EqualTo(terminal_F));
        });

        it("should create node [label_t, id_t, node_t&, node_t&]", [&]() {
          const node_t n_child1 = create_node(3,12, terminal_F, terminal_T);
          const node_t n_child2 = create_node(3,42, terminal_T, terminal_F);

          const node_t n = create_node(2,2, n_child1, n_child2);
          AssertThat(n.uid, Is().EqualTo(create_node_ptr(2,2)));
          AssertThat(label_of(n), Is().EqualTo(2u));
          AssertThat(id_of(n), Is().EqualTo(2u));

          AssertThat(n.low, Is().EqualTo(n_child1.uid));
          AssertThat(n.high, Is().EqualTo(n_child2.uid));
        });

        it("should create node [label_t, id_t, node_t&, ptr_t]", [&]() {
          const node_t n_child = create_node(2,2, terminal_F, terminal_T);

          const node_t n = create_node(1,7,terminal_T,n_child);
          AssertThat(n.uid, Is().EqualTo(create_node_ptr(1,7)));
          AssertThat(label_of(n), Is().EqualTo(1u));
          AssertThat(id_of(n), Is().EqualTo(7u));

          AssertThat(n.low, Is().EqualTo(terminal_T));
          AssertThat(n.high, Is().EqualTo(n_child.uid));
        });

        it("should create node [label_t, id_t, ptr_t, node_t&]", [&]() {
          const node_t n_child = create_node(2,2,terminal_F,terminal_T);

          const node_t n = create_node(0,3,terminal_T,n_child);
          AssertThat(n.uid, Is().EqualTo(create_node_ptr(0,3)));
          AssertThat(label_of(n), Is().EqualTo(0u));
          AssertThat(id_of(n), Is().EqualTo(3u));

          AssertThat(n.low, Is().EqualTo(terminal_T));
          AssertThat(n.high, Is().EqualTo(n_child.uid));
        });
      });

      it("should sort by label, then by id", [&]() {
        const node_t node_1_2 = create_node(1,2, terminal_F, terminal_T);
        const node_t node_2_1 = create_node(2,1, terminal_T, terminal_F);

        AssertThat(node_1_2 < node_2_1, Is().True());
        AssertThat(node_2_1 > node_1_2, Is().True());

        const node_t node_2_2 = create_node(2,2, terminal_F, terminal_F);

        AssertThat(node_2_1 < node_2_2, Is().True());
        AssertThat(node_2_2 > node_2_1, Is().True());
      });

      it("should be equal by their content", [&]() {
        const node_t node_1_v1 = create_node(42,2, terminal_F, terminal_T);
        const node_t node_1_v2 = create_node(42,2, terminal_F, terminal_T);

        AssertThat(node_1_v1 == node_1_v2, Is().True());
        AssertThat(node_1_v1 != node_1_v2, Is().False());
      });

      it("should be unequal by their content", [&]() {
        const node_t node_1 = create_node(42,2, terminal_F, terminal_T);
        const node_t node_2 = create_node(42,2, terminal_F, terminal_F);
        const node_t node_3 = create_node(42,3, terminal_F, terminal_T);
        const node_t node_4 = create_node(21,2, terminal_F, terminal_T);

        AssertThat(node_1 == node_2, Is().False());
        AssertThat(node_1 != node_2, Is().True());

        AssertThat(node_1 == node_3, Is().False());
        AssertThat(node_1 != node_3, Is().True());

        AssertThat(node_1 == node_4, Is().False());
        AssertThat(node_1 != node_4, Is().True());
      });

      describe("terminal nodes", [&]() {
        const node_t terminal_node_T = create_terminal(true);
        const node_t terminal_node_F = create_terminal(false);

        describe("is_terminal", [&]() {
          it("accepts true terminal", [&]() {
            AssertThat(is_terminal(terminal_node_T), Is().True());
          });

          it("accepts false terminal", [&]() {
            AssertThat(is_terminal(terminal_node_F), Is().True());
          });

          it("rejects non-terminal nodes [1]", [&]() {
            const node_t node_1 = create_node(42,2, terminal_F, terminal_T);
            AssertThat(is_terminal(node_1), Is().False());
          });

          it("rejects non-terminal nodes [2]", [&]() {
            const node_t almost_F_terminal = create_node(0,0, terminal_T, create_node_ptr(42,2));
            AssertThat(is_terminal(almost_F_terminal), Is().False());
          });

          it("rejects non-terminal nodes [2]", [&]() {
            const node_t almost_T_terminal = create_node(0,1, terminal_T, create_node_ptr(42,2));
            AssertThat(is_terminal(almost_T_terminal), Is().False());
          });
        });

        describe("value_of", [&]() {
          it("retrieves value of a true terminal node", [&]() {
            AssertThat(value_of(terminal_node_T), Is().True());
          });

          it("retrieves value of a false terminal node", [&]() {
            AssertThat(value_of(terminal_node_F), Is().False());
          });
        });

        describe("is_false", [&]() {
          it("rejects true terminal", [&]() {
            AssertThat(is_false(terminal_node_T), Is().False());
          });

          it("accepts false terminal", [&]() {
            AssertThat(is_false(terminal_node_F), Is().True());
          });

          it("rejects non-terminal nodes", [&]() {
            const node_t n = create_node(0,0, create_node_ptr(42,2), terminal_F);
            AssertThat(is_false(n), Is().False());
          });
        });

        describe("is_true", [&]() {
          it("accepts true terminal", [&]() {
            AssertThat(is_true(terminal_node_T), Is().True());
          });

          it("rejects false terminal", [&]() {
            AssertThat(is_true(terminal_node_F), Is().False());
          });

          it("rejects non-terminal nodes", [&]() {
            const node_t n = create_node(0,1, terminal_T, create_node_ptr(2,3));
            AssertThat(is_true(n), Is().False());
          });
        });
      });

      describe("negate (!)", [&]() {
        it("should leave node_ptr children unchanged", [&]() {
          const node_t node = create_node(2,2, create_node_ptr(42,3), create_node_ptr(8,2));

          AssertThat(!node, Is().EqualTo(node));
        });

        it("should negate terminal_ptr child", [&]() {
          const node_t node = create_node(2,2, terminal_F, create_node_ptr(8,2));

          AssertThat(!node, Is().EqualTo(create_node(2,2, terminal_T, create_node_ptr(8,2))));
        });

        it("should negate terminal_ptr children while preserving flags", [&]() {
          const node_t node = create_node(2,2, terminal_F, flag(terminal_T));

          AssertThat(!node, Is().EqualTo(create_node(2,2, terminal_T, flag(terminal_F))));
        });

        it("should negate terminal node", [&]() {
          AssertThat(!create_terminal(true), Is().EqualTo(create_terminal(false)));
          AssertThat(!create_terminal(false), Is().EqualTo(create_terminal(true)));
        });
      });
    });

    describe("arc_t", [&]() {
      const ptr_t terminal_F = create_terminal_ptr(false);
      const ptr_t terminal_T = create_terminal_ptr(true);

      it("should be equal by their content", [&]() {
        const ptr_t source = create_node_ptr(4,2);
        const ptr_t target = create_node_ptr(42,3);

        const arc_t arc_1 = { source, target };
        const arc_t arc_2 = { source, target };

        AssertThat(arc_1 == arc_2, Is().True());
        AssertThat(arc_1 != arc_2, Is().False());
      });

      it("should unequal by their content", [&]() {
        const ptr_t node_ptr_1 = create_node_ptr(4,2);
        const ptr_t node_ptr_2 = create_node_ptr(4,3);
        const ptr_t node_ptr_3 = create_node_ptr(3,2);

        const arc_t arc_1 = { node_ptr_1, node_ptr_2 };
        const arc_t arc_2 = { node_ptr_1, node_ptr_3 };

        AssertThat(arc_1 == arc_2, Is().False());
        AssertThat(arc_1 != arc_2, Is().True());

        const arc_t arc_3 = { node_ptr_1, node_ptr_2 };
        const arc_t arc_4 = { flag(node_ptr_1), node_ptr_2 };

        AssertThat(arc_3 == arc_4, Is().False());
        AssertThat(arc_3 != arc_4, Is().True());

        const arc_t arc_5 = { node_ptr_1, node_ptr_2 };
        const arc_t arc_6 = { node_ptr_3, node_ptr_2 };

        AssertThat(arc_5 == arc_6, Is().False());
        AssertThat(arc_5 != arc_6, Is().True());
      });

      it("should recognise low arcs from bit-flag on source", [&]() {
        const ptr_t node_ptr_1 = create_node_ptr(4,2);
        const ptr_t node_ptr_2 = create_node_ptr(4,3);

        const arc_t arc_low = { node_ptr_1, node_ptr_2 };
        AssertThat(is_high(arc_low), Is().False());
      });

      it("should recognise high arcs from bit-flag on source", [&]() {
        const ptr_t node_ptr_1 = create_node_ptr(4,2);
        const ptr_t node_ptr_2 = create_node_ptr(4,3);

        const arc_t arc_high = { flag(node_ptr_1), node_ptr_2 };
        AssertThat(is_high(arc_high), Is().True());
      });

      it("should leave node_ptr target unchanged", [&]() {
        const arc_t a = { create_node_ptr(1,0), create_node_ptr(2,0) };
        AssertThat(!a, Is().EqualTo(a));
      });

      it("should negate unflagged terminal_ptr target", [&]() {
        const arc_t a = { create_node_ptr(1,0), terminal_T };
        AssertThat(!a, Is().EqualTo(arc { create_node_ptr(1,0), terminal_F }));
      });

      it("should negate flagged terminal_ptr target", [&]() {
        const arc_t a = { create_node_ptr(1,0), flag(terminal_F) };
        AssertThat(!a, Is().EqualTo(arc { create_node_ptr(1,0), flag(terminal_T) }));
      });

      it("should be a POD", [&]() {
        AssertThat(std::is_pod<arc_t>::value, Is().True());
      });

      it("should take up 16 bytes of memory", [&]() {
        const arc_t arc = { create_node_ptr(42,2), terminal_F };
        AssertThat(sizeof(arc), Is().EqualTo(2u * 8u));
      });
    });

    describe("low_arc_of, high_arc_of, node_of", [&]() {
      it("should extract low arc from node", [&]() {
        const node_t node = create_node(7,42,
                                        create_node_ptr(8,21),
                                        create_node_ptr(9,8));

        const arc_t arc = low_arc_of(node);

        AssertThat(label_of(arc.source), Is().EqualTo(7u));
        AssertThat(id_of(arc.source), Is().EqualTo(42u));
        AssertThat(label_of(arc.target), Is().EqualTo(8u));
        AssertThat(id_of(arc.target), Is().EqualTo(21u));
      });

      it("should extract high arc from node", [&]() {
        const node_t node = create_node(6,13,
                                        create_node_ptr(8,21),
                                        create_node_ptr(9,8));

        const arc_t arc = high_arc_of(node);

        AssertThat(label_of(arc.source), Is().EqualTo(6u));
        AssertThat(id_of(arc.source), Is().EqualTo(13u));
        AssertThat(label_of(arc.target), Is().EqualTo(9u));
        AssertThat(id_of(arc.target), Is().EqualTo(8u));
      });

      it("should combine low and high arcs into single node", [&]() {
        const arc_t low_arc = { create_node_ptr(17,42), create_node_ptr(9,8) };
        const arc_t high_arc = { flag(create_node_ptr(17,42)), create_node_ptr(8,21) };

        const node_t node = node_of(low_arc, high_arc);

        AssertThat(label_of(node), Is().EqualTo(17u));
        AssertThat(id_of(node), Is().EqualTo(42u));

        AssertThat(label_of(node.low), Is().EqualTo(label_of(low_arc.target)));
        AssertThat(id_of(node.low), Is().EqualTo(id_of(low_arc.target)));

        AssertThat(label_of(node.high), Is().EqualTo(label_of(high_arc.target)));
        AssertThat(id_of(node.high), Is().EqualTo(id_of(high_arc.target)));
      });
    });

    describe("assignment_t", [&]() {
      const assignment_t a1 = create_assignment(2, false);
      const assignment_t a2 = create_assignment(2, true);
      const assignment_t a3 = create_assignment(3, false);

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

      const assignment_t b1 = create_assignment(2, false);
      const assignment_t b2 = create_assignment(2, true);
      const assignment_t b3 = create_assignment(3, false);

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
