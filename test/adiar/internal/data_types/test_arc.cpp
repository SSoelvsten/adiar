go_bandit([]() {
    describe("adiar/internal/data_types/arc.h", []() {
        describe("arc_t", [&]() {
            const ptr_uint64 terminal_F = ptr_uint64(false);
            const ptr_uint64 terminal_T = ptr_uint64(true);

            it("should be equal by their content", [&]() {
                const ptr_uint64 source = ptr_uint64(4,2);
                const ptr_uint64 target = ptr_uint64(42,3);

                const arc_t arc_1 = { source, target };
                const arc_t arc_2 = { source, target };

                AssertThat(arc_1 == arc_2, Is().True());
                AssertThat(arc_1 != arc_2, Is().False());
              });

            it("should unequal by their content", [&]() {
                const ptr_uint64 node_ptr_1 = ptr_uint64(4,2);
                const ptr_uint64 node_ptr_2 = ptr_uint64(4,3);
                const ptr_uint64 node_ptr_3 = ptr_uint64(3,2);

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
                const ptr_uint64 node_ptr_1 = ptr_uint64(4,2);
                const ptr_uint64 node_ptr_2 = ptr_uint64(4,3);

                const arc_t arc_low = { node_ptr_1, node_ptr_2 };
                AssertThat(arc_low.is_high(), Is().False());
              });

            it("should recognise high arcs from bit-flag on source", [&]() {
                const ptr_uint64 node_ptr_1 = ptr_uint64(4,2);
                const ptr_uint64 node_ptr_2 = ptr_uint64(4,3);

                const arc_t arc_high = { flag(node_ptr_1), node_ptr_2 };
                AssertThat(arc_high.is_high(), Is().True());
              });

            it("should leave node_ptr target unchanged", [&]() {
                const arc_t a = { ptr_uint64(1,0), ptr_uint64(2,0) };
                AssertThat(!a, Is().EqualTo(a));
              });

            it("should negate unflagged terminal_ptr target", [&]() {
                const arc_t a = { ptr_uint64(1,0), terminal_T };
                AssertThat(!a, Is().EqualTo(arc { ptr_uint64(1,0), terminal_F }));
              });

            it("should negate flagged terminal_ptr target", [&]() {
                const arc_t a = { ptr_uint64(1,0), flag(terminal_F) };
                AssertThat(!a, Is().EqualTo(arc { ptr_uint64(1,0), flag(terminal_T) }));
              });

            it("should be a POD", [&]() {
                AssertThat(std::is_pod<arc_t>::value, Is().True());
              });

            it("should take up 16 bytes of memory", [&]() {
                const arc_t arc = { ptr_uint64(42,2), terminal_F };
                AssertThat(sizeof(arc), Is().EqualTo(2u * 8u));
              });
          });
      });
  });
