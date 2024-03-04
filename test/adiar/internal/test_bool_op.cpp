#include "../../test.h"

#include <adiar/internal/bool_op.h>

go_bandit([]() {
  describe("adiar/internal/bool_op.h", []() {
    describe("can_shortcut", [&]() {
      it("can check on 'true' on the left", [&]() {
        AssertThat(can_left_shortcut(and_op, true), Is().False());
        AssertThat(can_left_shortcut(or_op, true), Is().True());
        AssertThat(can_left_shortcut(xor_op, true), Is().False());
        AssertThat(can_left_shortcut(imp_op, true), Is().False());
      });

      it("can check on 'false' on the left", [&]() {
        AssertThat(can_left_shortcut(and_op, false), Is().True());
        AssertThat(can_left_shortcut(or_op, false), Is().False());
        AssertThat(can_left_shortcut(xor_op, false), Is().False());
        AssertThat(can_left_shortcut(imp_op, false), Is().True());
      });

      it("can check on 'true' on the right", [&]() {
        AssertThat(can_right_shortcut(and_op, true), Is().False());
        AssertThat(can_right_shortcut(or_op, true), Is().True());
        AssertThat(can_right_shortcut(xor_op, true), Is().False());
        AssertThat(can_right_shortcut(imp_op, true), Is().True());
      });

      it("can check on 'false' on the right", [&]() {
        AssertThat(can_right_shortcut(and_op, false), Is().True());
        AssertThat(can_right_shortcut(or_op, false), Is().False());
        AssertThat(can_right_shortcut(xor_op, false), Is().False());
        AssertThat(can_right_shortcut(imp_op, false), Is().False());
      });
    });

    describe("is_idempotent", [&]() {
      it("can check on 'true' on the left", [&]() {
        AssertThat(is_left_idempotent(and_op, true), Is().True());
        AssertThat(is_left_idempotent(or_op, true), Is().False());
        AssertThat(is_left_idempotent(xor_op, true), Is().False());
        AssertThat(is_left_idempotent(imp_op, true), Is().True());
      });

      it("can check on 'false' on the left", [&]() {
        AssertThat(is_left_idempotent(and_op, false), Is().False());
        AssertThat(is_left_idempotent(or_op, false), Is().True());
        AssertThat(is_left_idempotent(xor_op, false), Is().True());
        AssertThat(is_left_idempotent(imp_op, false), Is().False());
      });

      it("can check on 'true' on the right", [&]() {
        AssertThat(is_right_idempotent(and_op, true), Is().True());
        AssertThat(is_right_idempotent(or_op, true), Is().False());
        AssertThat(is_right_idempotent(xor_op, true), Is().False());
        AssertThat(is_right_idempotent(imp_op, true), Is().False());
      });

      it("can check on 'false' on the right", [&]() {
        AssertThat(is_right_idempotent(and_op, false), Is().False());
        AssertThat(is_right_idempotent(or_op, false), Is().True());
        AssertThat(is_right_idempotent(xor_op, false), Is().True());
        AssertThat(is_right_idempotent(imp_op, false), Is().False());
      });
    });

    describe("is_negating", [&]() {
      it("can check on 'true' on the left", [&]() {
        AssertThat(is_left_negating(and_op, true), Is().False());
        AssertThat(is_left_negating(or_op, true), Is().False());
        AssertThat(is_left_negating(xor_op, true), Is().True());
        AssertThat(is_left_negating(imp_op, true), Is().False());
      });

      it("can check on 'false' on the left", [&]() {
        AssertThat(is_left_negating(and_op, false), Is().False());
        AssertThat(is_left_negating(or_op, false), Is().False());
        AssertThat(is_left_negating(xor_op, false), Is().False());
        AssertThat(is_left_negating(imp_op, false), Is().False());
      });

      it("can check on 'true' on the right", [&]() {
        AssertThat(is_right_negating(and_op, true), Is().False());
        AssertThat(is_right_negating(or_op, true), Is().False());
        AssertThat(is_right_negating(xor_op, true), Is().True());
        AssertThat(is_right_negating(imp_op, true), Is().False());
      });

      it("can check on 'false' on the right", [&]() {
        AssertThat(is_right_negating(and_op, false), Is().False());
        AssertThat(is_right_negating(or_op, false), Is().False());
        AssertThat(is_right_negating(xor_op, false), Is().False());
        AssertThat(is_right_negating(imp_op, false), Is().False());
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
        const predicate<bool, bool>& flip_and_op = flip(and_op);

        AssertThat(flip_and_op(true, true), Is().EqualTo(true));
        AssertThat(flip_and_op(true, false), Is().EqualTo(false));
        AssertThat(flip_and_op(false, true), Is().EqualTo(false));
        AssertThat(flip_and_op(false, false), Is().EqualTo(false));
      });

      it("resolves non-commutative operator (imp)", [&]() {
        const predicate<bool, bool>& flip_imp_op = flip(imp_op);

        AssertThat(flip_imp_op(true, true), Is().EqualTo(true));
        AssertThat(flip_imp_op(true, false), Is().EqualTo(true));
        AssertThat(flip_imp_op(false, true), Is().EqualTo(false));
        AssertThat(flip_imp_op(false, false), Is().EqualTo(true));
      });
    });
  });
 });
