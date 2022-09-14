go_bandit([]() {
    describe("adiar/internal/data_types/uid.h", []() {
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

        describe("terminals", [&](){
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
                    ptr_t arc_node_max = create_node_ptr(MAX_LABEL,MAX_ID);
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
                    ptr_t arc_node_max = flag(create_node_ptr(MAX_LABEL,MAX_ID));
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
          });

        describe("internal nodes", [&]() {
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
                    const ptr_t p = create_node_ptr(MAX_LABEL, MAX_ID);
                    AssertThat(label_of(p), Is().EqualTo(MAX_LABEL));
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
                    const ptr_t p = create_node_ptr(MAX_LABEL, MAX_ID);
                    AssertThat(label_of(p), Is().EqualTo(MAX_LABEL));
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
                    const ptr_t p = create_node_ptr(MAX_LABEL, MAX_ID);
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
                    const ptr_t p = create_node_ptr(MAX_LABEL, MAX_ID);
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
                    const ptr_t p_node = create_node_ptr(MAX_LABEL,MAX_ID);

                    AssertThat(p_node < terminal_F, Is().True());
                    AssertThat(flag(p_node) < terminal_F, Is().True());
                    AssertThat(p_node < flag(terminal_F), Is().True());
                    AssertThat(flag(p_node) < flag(terminal_F), Is().True());
                  });

                it("should sort T terminal after internal node (unflagged)", [&]() {
                    // Create a node pointer with the highest possible raw value
                    const ptr_t p_node = create_node_ptr(MAX_LABEL,MAX_ID);

                    AssertThat(p_node < terminal_T, Is().True());
                    AssertThat(flag(p_node) < terminal_T, Is().True());
                    AssertThat(p_node < flag(terminal_T), Is().True());
                    AssertThat(flag(p_node) < flag(terminal_T), Is().True());
                  });
              });

            describe("is_node", [&]() {
                it("should recognise Node Ptr (unflagged)", [&]() {
                    const ptr_t p_node_max = create_node_ptr(MAX_LABEL,MAX_ID);
                    AssertThat(is_node(p_node_max), Is().True());

                    const ptr_t p_node_min = create_node_ptr(0,0);
                    AssertThat(is_node(p_node_min), Is().True());

                    const ptr_t p_node = create_node_ptr(42,18);
                    AssertThat(is_node(p_node), Is().True());
                  });

                it("should recognise Node Ptr (flagged)", [&]() {
                    const ptr_t p_node_max = flag(create_node_ptr(MAX_LABEL,MAX_ID));
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
      });
  });
