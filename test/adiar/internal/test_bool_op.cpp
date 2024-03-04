#include "../../test.h"

#include <adiar/internal/bool_op.h>

go_bandit([]() {
  describe("adiar/internal/bool_op.h", []() {
    describe("predicate<bool, bool> predicates", []() {
      describe("can_shortcut", []() {
        it("can check on 'false' on the left", []() {
          AssertThat(can_left_shortcut(and_op, false), Is().True());
          AssertThat(can_left_shortcut(or_op, false), Is().False());
          AssertThat(can_left_shortcut(xor_op, false), Is().False());
          AssertThat(can_left_shortcut(imp_op, false), Is().True());
        });

        it("can check on 'true' on the left", []() {
          AssertThat(can_left_shortcut(and_op, true), Is().False());
          AssertThat(can_left_shortcut(or_op, true), Is().True());
          AssertThat(can_left_shortcut(xor_op, true), Is().False());
          AssertThat(can_left_shortcut(imp_op, true), Is().False());
        });

        it("can check on 'false' on the right", []() {
          AssertThat(can_right_shortcut(and_op, false), Is().True());
          AssertThat(can_right_shortcut(or_op, false), Is().False());
          AssertThat(can_right_shortcut(xor_op, false), Is().False());
          AssertThat(can_right_shortcut(imp_op, false), Is().False());
        });

        it("can check on 'true' on the right", []() {
          AssertThat(can_right_shortcut(and_op, true), Is().False());
          AssertThat(can_right_shortcut(or_op, true), Is().True());
          AssertThat(can_right_shortcut(xor_op, true), Is().False());
          AssertThat(can_right_shortcut(imp_op, true), Is().True());
        });
      });

      describe("is_idempotent", []() {
        it("can check on 'false' on the left", []() {
          AssertThat(is_left_idempotent(and_op, false), Is().False());
          AssertThat(is_left_idempotent(or_op, false), Is().True());
          AssertThat(is_left_idempotent(xor_op, false), Is().True());
          AssertThat(is_left_idempotent(imp_op, false), Is().False());
        });

        it("can check on 'true' on the left", []() {
          AssertThat(is_left_idempotent(and_op, true), Is().True());
          AssertThat(is_left_idempotent(or_op, true), Is().False());
          AssertThat(is_left_idempotent(xor_op, true), Is().False());
          AssertThat(is_left_idempotent(imp_op, true), Is().True());
        });

        it("can check on 'false' on the right", []() {
          AssertThat(is_right_idempotent(and_op, false), Is().False());
          AssertThat(is_right_idempotent(or_op, false), Is().True());
          AssertThat(is_right_idempotent(xor_op, false), Is().True());
          AssertThat(is_right_idempotent(imp_op, false), Is().False());
        });

        it("can check on 'true' on the right", []() {
          AssertThat(is_right_idempotent(and_op, true), Is().True());
          AssertThat(is_right_idempotent(or_op, true), Is().False());
          AssertThat(is_right_idempotent(xor_op, true), Is().False());
          AssertThat(is_right_idempotent(imp_op, true), Is().False());
        });
      });

      describe("is_negating", []() {
        it("can check on 'false' on the left", []() {
          AssertThat(is_left_negating(and_op, false), Is().False());
          AssertThat(is_left_negating(or_op, false), Is().False());
          AssertThat(is_left_negating(xor_op, false), Is().False());
          AssertThat(is_left_negating(imp_op, false), Is().False());
          AssertThat(is_left_negating(invimp_op, false), Is().True());
        });

        it("can check on 'true' on the left", []() {
          AssertThat(is_left_negating(and_op, true), Is().False());
          AssertThat(is_left_negating(or_op, true), Is().False());
          AssertThat(is_left_negating(xor_op, true), Is().True());
          AssertThat(is_left_negating(imp_op, true), Is().False());
          AssertThat(is_left_negating(invimp_op, true), Is().False());
        });

        it("can check on 'false' on the right", []() {
          AssertThat(is_right_negating(and_op, false), Is().False());
          AssertThat(is_right_negating(or_op, false), Is().False());
          AssertThat(is_right_negating(xor_op, false), Is().False());
          AssertThat(is_right_negating(imp_op, false), Is().True());
          AssertThat(is_right_negating(invimp_op, false), Is().False());
        });

        it("can check on 'true' on the right", []() {
          AssertThat(is_right_negating(and_op, true), Is().False());
          AssertThat(is_right_negating(or_op, true), Is().False());
          AssertThat(is_right_negating(xor_op, true), Is().True());
          AssertThat(is_right_negating(imp_op, true), Is().False());
          AssertThat(is_right_negating(invimp_op, true), Is().False());
        });
      });

      describe("is_commutative(...)", []() {
        it("identifies commutative operators", []() {
          AssertThat(is_commutative(and_op), Is().True());
          AssertThat(is_commutative(nand_op), Is().True());
          AssertThat(is_commutative(or_op), Is().True());
          AssertThat(is_commutative(nor_op), Is().True());
          AssertThat(is_commutative(xor_op), Is().True());
          AssertThat(is_commutative(equiv_op), Is().True());
        });

        it("identifies non-commutative operators", []() {
          AssertThat(is_commutative(imp_op), Is().False());
          AssertThat(is_commutative(invimp_op), Is().False());
          AssertThat(is_commutative(diff_op), Is().False());
          AssertThat(is_commutative(diff_op), Is().False());
        });
      });

      describe("flip(...)", []() {
        it("resolves commutative operator (and)", []() {
          const predicate<bool, bool>& flip_and_op = flip(and_op);

          AssertThat(flip_and_op(true, true), Is().EqualTo(true));
          AssertThat(flip_and_op(true, false), Is().EqualTo(false));
          AssertThat(flip_and_op(false, true), Is().EqualTo(false));
          AssertThat(flip_and_op(false, false), Is().EqualTo(false));
        });

        it("resolves non-commutative operator (imp)", []() {
          const predicate<bool, bool>& flip_imp_op = flip(imp_op);

          AssertThat(flip_imp_op(true, true), Is().EqualTo(true));
          AssertThat(flip_imp_op(true, false), Is().EqualTo(true));
          AssertThat(flip_imp_op(false, true), Is().EqualTo(false));
          AssertThat(flip_imp_op(false, false), Is().EqualTo(true));
        });
      });
    });

    describe("binary_op<predicate<bool, bool>>", []() {
      it("uses expected number of bytes", []() {
        AssertThat(sizeof(binary_op<predicate<bool, bool>>), Is().EqualTo(17u));
      });

      constexpr ptr_uint64 ptr_false(false);
      constexpr ptr_uint64 ptr_true(true);

      describe("operator(ptr_uint64, ptr_uint64)", [&]() {
        it("has (pre-computed) values for 'adiar::and_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::and_op);

          AssertThat(op(ptr_false, ptr_false), Is().EqualTo(ptr_false));
          AssertThat(op(ptr_false, ptr_true), Is().EqualTo(ptr_false));
          AssertThat(op(ptr_true, ptr_false), Is().EqualTo(ptr_false));
          AssertThat(op(ptr_true, ptr_true), Is().EqualTo(ptr_true));
        });

        it("has (pre-computed) values for 'adiar::or_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::or_op);

          AssertThat(op(ptr_false, ptr_false), Is().EqualTo(ptr_false));
          AssertThat(op(ptr_false, ptr_true), Is().EqualTo(ptr_true));
          AssertThat(op(ptr_true, ptr_false), Is().EqualTo(ptr_true));
          AssertThat(op(ptr_true, ptr_true), Is().EqualTo(ptr_true));
        });

        it("has (pre-computed) values for 'adiar::xor_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::xor_op);

          AssertThat(op(ptr_false, ptr_false), Is().EqualTo(ptr_false));
          AssertThat(op(ptr_false, ptr_true), Is().EqualTo(ptr_true));
          AssertThat(op(ptr_true, ptr_false), Is().EqualTo(ptr_true));
          AssertThat(op(ptr_true, ptr_true), Is().EqualTo(ptr_false));
        });

        it("has (pre-computed) values for 'adiar::imp_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::imp_op);

          AssertThat(op(ptr_false, ptr_false), Is().EqualTo(ptr_true));
          AssertThat(op(ptr_false, ptr_true), Is().EqualTo(ptr_true));
          AssertThat(op(ptr_true, ptr_false), Is().EqualTo(ptr_false));
          AssertThat(op(ptr_true, ptr_true), Is().EqualTo(ptr_true));
        });

        it("has (pre-computed) values for 'adiar::invimp_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::invimp_op);

          AssertThat(op(ptr_false, ptr_false), Is().EqualTo(ptr_true));
          AssertThat(op(ptr_false, ptr_true), Is().EqualTo(ptr_false));
          AssertThat(op(ptr_true, ptr_false), Is().EqualTo(ptr_true));
          AssertThat(op(ptr_true, ptr_true), Is().EqualTo(ptr_true));
        });
      });

      describe(".can_shortcut", [&]() {
        it("has (pre-computed) values for 'adiar::and_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::and_op);

          AssertThat(op.can_left_shortcut(ptr_false), Is().True());
          AssertThat(op.can_right_shortcut(ptr_false), Is().True());

          AssertThat(op.can_left_shortcut(ptr_true), Is().False());
          AssertThat(op.can_right_shortcut(ptr_true), Is().False());
        });

        it("has (pre-computed) values for 'adiar::or_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::or_op);

          AssertThat(op.can_left_shortcut(ptr_false), Is().False());
          AssertThat(op.can_right_shortcut(ptr_false), Is().False());

          AssertThat(op.can_left_shortcut(ptr_true), Is().True());
          AssertThat(op.can_right_shortcut(ptr_true), Is().True());
        });

        it("has (pre-computed) values for 'adiar::xor_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::xor_op);

          AssertThat(op.can_left_shortcut(ptr_false), Is().False());
          AssertThat(op.can_right_shortcut(ptr_false), Is().False());

          AssertThat(op.can_left_shortcut(ptr_true), Is().False());
          AssertThat(op.can_right_shortcut(ptr_true), Is().False());
        });

        it("has (pre-computed) values for 'adiar::imp_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::imp_op);

          AssertThat(op.can_left_shortcut(ptr_false), Is().True());
          AssertThat(op.can_right_shortcut(ptr_false), Is().False());

          AssertThat(op.can_left_shortcut(ptr_true), Is().False());
          AssertThat(op.can_right_shortcut(ptr_true), Is().True());
        });

        it("has (pre-computed) values for 'adiar::invimp_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::invimp_op);

          AssertThat(op.can_left_shortcut(ptr_false), Is().False());
          AssertThat(op.can_right_shortcut(ptr_false), Is().True());

          AssertThat(op.can_left_shortcut(ptr_true), Is().True());
          AssertThat(op.can_right_shortcut(ptr_true), Is().False());
        });
      });

      describe(".is_idempotent", [&]() {
        it("has (pre-computed) values for 'adiar::and_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::and_op);

          AssertThat(op.is_left_idempotent(ptr_false), Is().False());
          AssertThat(op.is_right_idempotent(ptr_false), Is().False());

          AssertThat(op.is_left_idempotent(ptr_true), Is().True());
          AssertThat(op.is_right_idempotent(ptr_true), Is().True());
        });

        it("has (pre-computed) values for 'adiar::or_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::or_op);

          AssertThat(op.is_left_idempotent(ptr_false), Is().True());
          AssertThat(op.is_right_idempotent(ptr_false), Is().True());

          AssertThat(op.is_left_idempotent(ptr_true), Is().False());
          AssertThat(op.is_right_idempotent(ptr_true), Is().False());
        });

        it("has (pre-computed) values for 'adiar::xor_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::xor_op);

          AssertThat(op.is_left_idempotent(ptr_false), Is().True());
          AssertThat(op.is_right_idempotent(ptr_false), Is().True());

          AssertThat(op.is_left_idempotent(ptr_true), Is().False());
          AssertThat(op.is_right_idempotent(ptr_true), Is().False());
        });

        it("has (pre-computed) values for 'adiar::imp_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::imp_op);

          AssertThat(op.is_left_idempotent(ptr_false), Is().False());
          AssertThat(op.is_right_idempotent(ptr_false), Is().False());

          AssertThat(op.is_left_idempotent(ptr_true), Is().True());
          AssertThat(op.is_right_idempotent(ptr_true), Is().False());
        });

        it("has (pre-computed) values for 'adiar::invimp_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::invimp_op);

          AssertThat(op.is_left_idempotent(ptr_false), Is().False());
          AssertThat(op.is_right_idempotent(ptr_false), Is().False());

          AssertThat(op.is_left_idempotent(ptr_true), Is().False());
          AssertThat(op.is_right_idempotent(ptr_true), Is().True());
        });
      });

      describe(".is_negating", [&]() {
        it("has (pre-computed) values for 'adiar::and_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::and_op);

          AssertThat(op.is_left_negating(ptr_false), Is().False());
          AssertThat(op.is_right_negating(ptr_false), Is().False());

          AssertThat(op.is_left_negating(ptr_true), Is().False());
          AssertThat(op.is_right_negating(ptr_true), Is().False());
        });

        it("has (pre-computed) values for 'adiar::or_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::or_op);

          AssertThat(op.is_left_negating(ptr_false), Is().False());
          AssertThat(op.is_right_negating(ptr_false), Is().False());

          AssertThat(op.is_left_negating(ptr_true), Is().False());
          AssertThat(op.is_right_negating(ptr_true), Is().False());
        });

        it("has (pre-computed) values for 'adiar::xor_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::xor_op);

          AssertThat(op.is_left_negating(ptr_false), Is().False());
          AssertThat(op.is_right_negating(ptr_false), Is().False());

          AssertThat(op.is_left_negating(ptr_true), Is().True());
          AssertThat(op.is_right_negating(ptr_true), Is().True());
        });

        it("has (pre-computed) values for 'adiar::imp_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::imp_op);

          AssertThat(op.is_left_negating(ptr_false), Is().False());
          AssertThat(op.is_right_negating(ptr_false), Is().True());

          AssertThat(op.is_left_negating(ptr_true), Is().False());
          AssertThat(op.is_right_negating(ptr_true), Is().False());
        });

        it("has (pre-computed) values for 'adiar::invimp_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::invimp_op);

          AssertThat(op.is_left_negating(ptr_false), Is().True());
          AssertThat(op.is_right_negating(ptr_false), Is().False());

          AssertThat(op.is_left_negating(ptr_true), Is().False());
          AssertThat(op.is_right_negating(ptr_true), Is().False());
        });
      });

      // TODO
    });
  });
 });
