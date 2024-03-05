#include "../test.h"

go_bandit([]() {
  describe("adiar/bool_op.h", []() {
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
      AssertThat(adiar::imp_op(true, true), Is().EqualTo(true));
      AssertThat(adiar::imp_op(true, false), Is().EqualTo(false));
      AssertThat(adiar::imp_op(false, true), Is().EqualTo(true));
      AssertThat(adiar::imp_op(false, false), Is().EqualTo(true));
    });

    it("invimp_op", [&]() {
      AssertThat(adiar::invimp_op(true, true), Is().EqualTo(true));
      AssertThat(adiar::invimp_op(true, false), Is().EqualTo(true));
      AssertThat(adiar::invimp_op(false, true), Is().EqualTo(false));
      AssertThat(adiar::invimp_op(false, false), Is().EqualTo(true));
    });

    it("equiv_op", [&]() {
      AssertThat(equiv_op(true, true), Is().EqualTo(true));
      AssertThat(equiv_op(true, false), Is().EqualTo(false));
      AssertThat(equiv_op(false, true), Is().EqualTo(false));
      AssertThat(equiv_op(false, false), Is().EqualTo(true));
    });

    it("diff_op", [&]() {
      AssertThat(adiar::diff_op(true, true), Is().EqualTo(false));
      AssertThat(adiar::diff_op(true, false), Is().EqualTo(true));
      AssertThat(adiar::diff_op(false, true), Is().EqualTo(false));
      AssertThat(adiar::diff_op(false, false), Is().EqualTo(false));
    });

    it("less_op", [&]() {
      AssertThat(adiar::less_op(true, true), Is().EqualTo(false));
      AssertThat(adiar::less_op(true, false), Is().EqualTo(false));
      AssertThat(adiar::less_op(false, true), Is().EqualTo(true));
      AssertThat(adiar::less_op(false, false), Is().EqualTo(false));
    });
  });
});
