#include "../test.h"

go_bandit([]() {
    describe("adiar/bool_op.h", []() {
        const ptr_uint64 terminal_F = ptr_uint64(false);
        const ptr_uint64 terminal_T = ptr_uint64(true);

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
        
        describe("flip operator", [&]() {
          it("resolves commutative operator (and)", [&]() {
              const bool_op &flip_and_op = flip(and_op);

              AssertThat(flip_and_op(terminal_T, terminal_T), Is().EqualTo(terminal_T));
              AssertThat(flip_and_op(terminal_T, terminal_F), Is().EqualTo(terminal_F));
              AssertThat(flip_and_op(terminal_F, terminal_T), Is().EqualTo(terminal_F));
              AssertThat(flip_and_op(terminal_F, terminal_F), Is().EqualTo(terminal_F));
            });

          it("resolves non-commutative operator (imp)", [&]() {
              const bool_op &flip_imp_op = flip(imp_op);

              AssertThat(flip_imp_op(terminal_T, terminal_T), Is().EqualTo(terminal_T));
              AssertThat(flip_imp_op(terminal_T, terminal_F), Is().EqualTo(terminal_T));
              AssertThat(flip_imp_op(terminal_F, terminal_T), Is().EqualTo(terminal_F));
              AssertThat(flip_imp_op(terminal_F, terminal_F), Is().EqualTo(terminal_T));
            });
        });
      });
  });
