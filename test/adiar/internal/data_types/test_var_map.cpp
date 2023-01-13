#include "../../../test.h"

enum class test_var_map
{
  FALSE   = 0,
  TRUE    = 1,
  OTHER_1 = 2,
  OTHER_2 = 4,
};

go_bandit([]() {
  describe("adiar/internal/data_types/var_map.h", []() {
    describe("var_mapping<...>", []() {
      describe("var_mapping<...>(label_t, enum), .var(), .value(), .raw()", []() {
        it("provides access to variable [42]", []() {
          var_mapping<test_var_map> a(42, test_var_map::FALSE);
          AssertThat(a.var(), Is().EqualTo(42u));
        });

        it("provides access to variable [21]", []() {
          var_mapping<test_var_map> a(21, test_var_map::TRUE);
          AssertThat(a.var(), Is().EqualTo(21u));
        });

        it("provides access to value [0]", []() {
          var_mapping<test_var_map> a(21, test_var_map::FALSE);
          AssertThat(a.value(), Is().EqualTo(test_var_map::FALSE));
        });

        it("provides access to value [1]", []() {
          var_mapping<test_var_map> a(21, test_var_map::TRUE);
          AssertThat(a.value(), Is().EqualTo(test_var_map::TRUE));
        });

        it("provides access to value [2]", []() {
          var_mapping<test_var_map> a(42, test_var_map::OTHER_1);
          AssertThat(a.value(), Is().EqualTo(test_var_map::OTHER_1));
        });

        it("provides access to value [4]", []() {
          var_mapping<test_var_map> a(42, test_var_map::OTHER_2);
          AssertThat(a.value(), Is().EqualTo(test_var_map::OTHER_2));
        });

        it("provides access to raw value [0]", []() {
          var_mapping<test_var_map> a(21, test_var_map::FALSE);
          AssertThat(a.raw(), Is().EqualTo(0));
        });

        it("provides access to raw value [1]", []() {
          var_mapping<test_var_map> a(21, test_var_map::TRUE);
          AssertThat(a.raw(), Is().EqualTo(1));
        });

        it("provides access to raw value [2]", []() {
          var_mapping<test_var_map> a(42, test_var_map::OTHER_1);
          AssertThat(a.raw(), Is().EqualTo(2));
        });

        it("provides access to raw value [4]", []() {
          var_mapping<test_var_map> a(42, test_var_map::OTHER_2);
          AssertThat(a.raw(), Is().EqualTo(4));
        });
      });

      describe(".is_false()", []() {
        it("is true for 'FALSE'", []() {
          var_mapping<test_var_map> a(0, test_var_map::FALSE);
          AssertThat(a.is_false(), Is().True());
        });

        it("is false for 'FALSE'", []() {
          var_mapping<test_var_map> a(0, test_var_map::TRUE);
          AssertThat(a.is_false(), Is().False());
        });

        it("is false for other", []() {
          var_mapping<test_var_map> a1(0, test_var_map::OTHER_1);
          AssertThat(a1.is_false(), Is().False());

          var_mapping<test_var_map> a2(0, test_var_map::OTHER_2);
          AssertThat(a2.is_false(), Is().False());

        });
      });

      describe(".is_true()", []() {
        it("is false for 'FALSE'", []() {
          var_mapping<test_var_map> a(0, test_var_map::FALSE);
          AssertThat(a.is_true(), Is().False());
        });

        it("is true for 'FALSE'", []() {
          var_mapping<test_var_map> a(0, test_var_map::TRUE);
          AssertThat(a.is_true(), Is().True());
        });

        it("is false for other", []() {
          var_mapping<test_var_map> a1(0, test_var_map::OTHER_1);
          AssertThat(a1.is_true(), Is().False());

          var_mapping<test_var_map> a2(0, test_var_map::OTHER_1);
          AssertThat(a2.is_true(), Is().False());
        });
      });

      describe("var_mapping<test_var_map>(label_t, bool)", []() {
        it("converts correctly from boolean value [0]", []() {
          var_mapping<test_var_map> a(42, false);
          AssertThat(a.var(), Is().EqualTo(42u));
          AssertThat(a.value(), Is().EqualTo(test_var_map::FALSE));
          AssertThat(a.raw(), Is().EqualTo(0));
        });

        it("converts correctly from boolean value [1]", []() {
          var_mapping<test_var_map> a(42, true);
          AssertThat(a.var(), Is().EqualTo(42u));
          AssertThat(a.value(), Is().EqualTo(test_var_map::TRUE));
          AssertThat(a.raw(), Is().EqualTo(1));
        });
      });

      describe("ordering '<' and '>'", []() {
        /*
        it("has 'FALSE' values precede 'TRUE' [<]", []() {
          AssertThat(var_mapping<test_var_map>(42, test_var_map::FALSE),
                                               Is().LessThan(var_mapping<test_var_map>(42, test_var_map::TRUE)));
          AssertThat(var_mapping<test_var_map>(42, test_var_map::TRUE),
                                               Is().Not().LessThan(var_mapping<test_var_map>(42, test_var_map::FALSE)));
        });

        it("has 'FALSE' values precede 'TRUE' [>]", []() {
          AssertThat(var_mapping<test_var_map>(42, test_var_map::TRUE),
                                               Is().GreaterThan(var_mapping<test_var_map>(42, test_var_map::FALSE)));
          AssertThat(var_mapping<test_var_map>(42, test_var_map::FALSE),
                                               Is().Not().GreaterThan(var_mapping<test_var_map>(42, test_var_map::TRUE)));
        });
        */

        it("sorts based on the variable order [1], [<]", []() {
          AssertThat(var_mapping<test_var_map>(21, test_var_map::TRUE),
                     Is().LessThan(var_mapping<test_var_map>(42, test_var_map::OTHER_1)));
          AssertThat(var_mapping<test_var_map>(42, test_var_map::OTHER_1),
                     Is().Not().LessThan(var_mapping<test_var_map>(21, test_var_map::TRUE)));
        });

        it("sorts based on the variable order [1], [>]", []() {
          AssertThat(var_mapping<test_var_map>(42, test_var_map::OTHER_1),
                     Is().GreaterThan(var_mapping<test_var_map>(21, test_var_map::TRUE)));
          AssertThat(var_mapping<test_var_map>(21, test_var_map::TRUE),
                     Is().Not().GreaterThan(var_mapping<test_var_map>(42, test_var_map::OTHER_1)));
        });

        it("sorts based on the variable order [2], [<]", []() {
          AssertThat(var_mapping<test_var_map>(20, test_var_map::FALSE),
                     Is().LessThan(var_mapping<test_var_map>(21, test_var_map::OTHER_1)));
          AssertThat(var_mapping<test_var_map>(21, test_var_map::OTHER_1),
                     Is().Not().LessThan(var_mapping<test_var_map>(20, test_var_map::FALSE)));
        });

        it("sorts based on the variable order [2], [>]", []() {
          AssertThat(var_mapping<test_var_map>(21, test_var_map::OTHER_1),
                     Is().GreaterThan(var_mapping<test_var_map>(20, test_var_map::FALSE)));
          AssertThat(var_mapping<test_var_map>(20, test_var_map::FALSE),
                     Is().Not().GreaterThan(var_mapping<test_var_map>(21, test_var_map::OTHER_1)));
        });
      });

      describe("equality '=='", []() {
        it("is true when both variable and value match [1]", [&]() {
          AssertThat(var_mapping<test_var_map>(1, test_var_map::OTHER_1),
                     Is().EqualTo(var_mapping<test_var_map>(1, test_var_map::OTHER_1)));
        });

        it("is true when both variable and value match [2]", [&]() {
          AssertThat(var_mapping<test_var_map>(2, test_var_map::FALSE),
                     Is().EqualTo(var_mapping<test_var_map>(2, test_var_map::FALSE)));
        });

        it("is true when both variable and value match [3]", [&]() {
          AssertThat(var_mapping<test_var_map>(42, test_var_map::TRUE),
                     Is().EqualTo(var_mapping<test_var_map>(42, test_var_map::TRUE)));
        });

        it("is false when the variable mismatches [1]", [&]() {
          AssertThat(var_mapping<test_var_map>(42, test_var_map::TRUE),
                     Is().Not().EqualTo(var_mapping<test_var_map>(21, test_var_map::TRUE)));
        });

        it("is false when the variable mismatches [2]", [&]() {
          AssertThat(var_mapping<test_var_map>(8, test_var_map::FALSE),
                     Is().Not().EqualTo(var_mapping<test_var_map>(10, test_var_map::FALSE)));
        });

        it("is false when the value mismatches [1]", [&]() {
          AssertThat(var_mapping<test_var_map>(42, test_var_map::OTHER_1),
                     Is().Not().EqualTo(var_mapping<test_var_map>(42, test_var_map::FALSE)));
          AssertThat(var_mapping<test_var_map>(42, test_var_map::OTHER_1),
                     Is().Not().EqualTo(var_mapping<test_var_map>(42, test_var_map::TRUE)));
        });

        it("is false when the value mismatches [2]", [&]() {
          AssertThat(var_mapping<test_var_map>(8, test_var_map::FALSE),
                     Is().Not().EqualTo(var_mapping<test_var_map>(8, test_var_map::OTHER_1)));
          AssertThat(var_mapping<test_var_map>(8, test_var_map::FALSE),
                     Is().Not().EqualTo(var_mapping<test_var_map>(8, test_var_map::TRUE)));
        });

        it("is false when the value mismatches [3]", [&]() {
          AssertThat(var_mapping<test_var_map>(8, test_var_map::TRUE),
                     Is().Not().EqualTo(var_mapping<test_var_map>(8, test_var_map::OTHER_1)));
          AssertThat(var_mapping<test_var_map>(13, test_var_map::TRUE),
                     Is().Not().EqualTo(var_mapping<test_var_map>(13, test_var_map::FALSE)));
        });
      });

      describe("negation '~", []() {
        it("turns 'FALSE' into 'TRUE' [1]", [&]() {
          AssertThat(~var_mapping<test_var_map>(1, test_var_map::FALSE),
                     Is().EqualTo(var_mapping<test_var_map>(1, test_var_map::TRUE)));
        });

        it("turns 'FALSE' into 'TRUE' [2]", [&]() {
          AssertThat(~var_mapping<test_var_map>(2, test_var_map::FALSE),
                     Is().EqualTo(var_mapping<test_var_map>(2, test_var_map::TRUE)));
        });

        it("turns 'TRUE' into 'FALSE' [1]", [&]() {
          AssertThat(~var_mapping<test_var_map>(3, test_var_map::FALSE),
                     Is().EqualTo(var_mapping<test_var_map>(3, test_var_map::TRUE)));
        });

        it("turns 'TRUE' into 'FALSE' [2]", [&]() {
          AssertThat(~var_mapping<test_var_map>(4, test_var_map::FALSE),
                     Is().EqualTo(var_mapping<test_var_map>(4, test_var_map::TRUE)));
        });

        it("keeps 'NONE' as-is [1]", [&]() {
          AssertThat(~var_mapping<test_var_map>(5, test_var_map::OTHER_1),
                     Is().EqualTo(var_mapping<test_var_map>(5, test_var_map::OTHER_1)));
        });

        it("keeps 'NONE' as-is [2]", [&]() {
          AssertThat(~var_mapping<test_var_map>(6, test_var_map::OTHER_1),
                     Is().EqualTo(var_mapping<test_var_map>(6, test_var_map::OTHER_1)));
        });
      });
    });
  });
 });
