#include "../test.h"

#include <adiar/internal/bool_op.h>

go_bandit([]() {
  describe("adiar/bool_op.h / adiar/internal/bool_op.h", []() {
    it("and_op", [&]() {
      AssertThat(and_op(true, true), Is().EqualTo(true));
      AssertThat(and_op(true, false), Is().EqualTo(false));
      AssertThat(and_op(false, true), Is().EqualTo(false));
      AssertThat(and_op(false, false), Is().EqualTo(false));
    });

    it("nand_op", [&]() {
      AssertThat(nand_op(true, true), Is().EqualTo(false));
      AssertThat(nand_op(true, false), Is().EqualTo(true));
      AssertThat(nand_op(false, true), Is().EqualTo(true));
      AssertThat(nand_op(false, false), Is().EqualTo(true));
    });

    it("or_op", [&]() {
      AssertThat(or_op(true, true), Is().EqualTo(true));
      AssertThat(or_op(true, false), Is().EqualTo(true));
      AssertThat(or_op(false, true), Is().EqualTo(true));
      AssertThat(or_op(false, false), Is().EqualTo(false));
    });

    it("nor_op", [&]() {
      AssertThat(nor_op(true, true), Is().EqualTo(false));
      AssertThat(nor_op(true, false), Is().EqualTo(false));
      AssertThat(nor_op(false, true), Is().EqualTo(false));
      AssertThat(nor_op(false, false), Is().EqualTo(true));
    });

    it("xor_op", [&]() {
      AssertThat(xor_op(true, true), Is().EqualTo(false));
      AssertThat(xor_op(true, false), Is().EqualTo(true));
      AssertThat(xor_op(false, true), Is().EqualTo(true));
      AssertThat(xor_op(false, false), Is().EqualTo(false));
    });

    it("xnor_op", [&]() {
      AssertThat(xnor_op(true, true), Is().EqualTo(true));
      AssertThat(xnor_op(true, false), Is().EqualTo(false));
      AssertThat(xnor_op(false, true), Is().EqualTo(false));
      AssertThat(xnor_op(false, false), Is().EqualTo(true));
    });

    it("imp_op", [&]() {
      AssertThat(imp_op(true, true), Is().EqualTo(true));
      AssertThat(imp_op(true, false), Is().EqualTo(false));
      AssertThat(imp_op(false, true), Is().EqualTo(true));
      AssertThat(imp_op(false, false), Is().EqualTo(true));
    });

    it("invimp_op", [&]() {
      AssertThat(invimp_op(true, true), Is().EqualTo(true));
      AssertThat(invimp_op(true, false), Is().EqualTo(true));
      AssertThat(invimp_op(false, true), Is().EqualTo(false));
      AssertThat(invimp_op(false, false), Is().EqualTo(true));
    });

    it("equiv_op", [&]() {
      AssertThat(equiv_op(true, true), Is().EqualTo(true));
      AssertThat(equiv_op(true, false), Is().EqualTo(false));
      AssertThat(equiv_op(false, true), Is().EqualTo(false));
      AssertThat(equiv_op(false, false), Is().EqualTo(true));
    });

    it("diff_op", [&]() {
      AssertThat(diff_op(true, true), Is().EqualTo(false));
      AssertThat(diff_op(true, false), Is().EqualTo(true));
      AssertThat(diff_op(false, true), Is().EqualTo(false));
      AssertThat(diff_op(false, false), Is().EqualTo(false));
    });

    it("less_op", [&]() {
      AssertThat(less_op(true, true), Is().EqualTo(false));
      AssertThat(less_op(true, false), Is().EqualTo(false));
      AssertThat(less_op(false, true), Is().EqualTo(true));
      AssertThat(less_op(false, false), Is().EqualTo(false));
    });

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

    describe("is_irrelevant", [&]() {
      it("can check on 'true' on the left", [&]() {
        AssertThat(is_left_irrelevant(and_op, true), Is().True());
        AssertThat(is_left_irrelevant(or_op, true), Is().False());
        AssertThat(is_left_irrelevant(xor_op, true), Is().False());
        AssertThat(is_left_irrelevant(imp_op, true), Is().True());
      });

      it("can check on 'false' on the left", [&]() {
        AssertThat(is_left_irrelevant(and_op, false), Is().False());
        AssertThat(is_left_irrelevant(or_op, false), Is().True());
        AssertThat(is_left_irrelevant(xor_op, false), Is().True());
        AssertThat(is_left_irrelevant(imp_op, false), Is().False());
      });

      it("can check on 'true' on the right", [&]() {
        AssertThat(is_right_irrelevant(and_op, true), Is().True());
        AssertThat(is_right_irrelevant(or_op, true), Is().False());
        AssertThat(is_right_irrelevant(xor_op, true), Is().False());
        AssertThat(is_right_irrelevant(imp_op, true), Is().False());
      });

      it("can check on 'false' on the right", [&]() {
        AssertThat(is_right_irrelevant(and_op, false), Is().False());
        AssertThat(is_right_irrelevant(or_op, false), Is().True());
        AssertThat(is_right_irrelevant(xor_op, false), Is().True());
        AssertThat(is_right_irrelevant(imp_op, false), Is().False());
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
        const bool_op& flip_and_op = flip(and_op);

        AssertThat(flip_and_op(true, true), Is().EqualTo(true));
        AssertThat(flip_and_op(true, false), Is().EqualTo(false));
        AssertThat(flip_and_op(false, true), Is().EqualTo(false));
        AssertThat(flip_and_op(false, false), Is().EqualTo(false));
      });

      it("resolves non-commutative operator (imp)", [&]() {
        const bool_op& flip_imp_op = flip(imp_op);

        AssertThat(flip_imp_op(true, true), Is().EqualTo(true));
        AssertThat(flip_imp_op(true, false), Is().EqualTo(true));
        AssertThat(flip_imp_op(false, true), Is().EqualTo(false));
        AssertThat(flip_imp_op(false, false), Is().EqualTo(true));
      });
    });
  });
});
