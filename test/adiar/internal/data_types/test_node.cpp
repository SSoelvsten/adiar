go_bandit([]() {
    describe("adiar/internal/data_types/node.h", []() {
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
      });
  });
