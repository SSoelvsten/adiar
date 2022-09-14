go_bandit([]() {
    describe("adiar/internal/data_types/arc.h", []() {
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
      });
  });
