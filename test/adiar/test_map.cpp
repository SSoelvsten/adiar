#include "../test.h"

enum class test_map_value
{
  FALSE   = 0,
  TRUE    = 1,
  OTHER_1 = 2,
  OTHER_2 = 4,
};

using test_map_pair = map_pair<int, test_map_value>;

go_bandit([]() {
  describe("adiar/map.h", []() {
    describe("map_pair<key, value>", []() {
      describe("var_mapping<...>(label_t, enum), .key(), .value(), .raw_value()", []() {
        it("provides access to variable [42]", []() {
          test_map_pair a(42, test_map_value::FALSE);
          AssertThat(a.key(), Is().EqualTo(42));
        });

        it("provides access to variable [21]", []() {
          test_map_pair a(21, test_map_value::TRUE);
          AssertThat(a.key(), Is().EqualTo(21));
        });

        it("provides access to value [0]", []() {
          test_map_pair a(21, test_map_value::FALSE);
          AssertThat(a.value(), Is().EqualTo(test_map_value::FALSE));
        });

        it("provides access to value [1]", []() {
          test_map_pair a(21, test_map_value::TRUE);
          AssertThat(a.value(), Is().EqualTo(test_map_value::TRUE));
        });

        it("provides access to value [2]", []() {
          test_map_pair a(42, test_map_value::OTHER_1);
          AssertThat(a.value(), Is().EqualTo(test_map_value::OTHER_1));
        });

        it("provides access to value [4]", []() {
          test_map_pair a(42, test_map_value::OTHER_2);
          AssertThat(a.value(), Is().EqualTo(test_map_value::OTHER_2));
        });

        it("provides access to raw value [0]", []() {
          test_map_pair a(21, test_map_value::FALSE);
          AssertThat(a.raw_value(), Is().EqualTo(0));
        });

        it("provides access to raw value [1]", []() {
          test_map_pair a(21, test_map_value::TRUE);
          AssertThat(a.raw_value(), Is().EqualTo(1));
        });

        it("provides access to raw value [2]", []() {
          test_map_pair a(42, test_map_value::OTHER_1);
          AssertThat(a.raw_value(), Is().EqualTo(2));
        });

        it("provides access to raw value [4]", []() {
          test_map_pair a(42, test_map_value::OTHER_2);
          AssertThat(a.raw_value(), Is().EqualTo(4));
        });
      });

      describe(".is_false()", []() {
        it("is true for 'FALSE'", []() {
          test_map_pair a(0, test_map_value::FALSE);
          AssertThat(a.is_false(), Is().True());
        });

        it("is false for 'FALSE'", []() {
          test_map_pair a(0, test_map_value::TRUE);
          AssertThat(a.is_false(), Is().False());
        });

        it("is false for other", []() {
          test_map_pair a1(0, test_map_value::OTHER_1);
          AssertThat(a1.is_false(), Is().False());

          test_map_pair a2(0, test_map_value::OTHER_2);
          AssertThat(a2.is_false(), Is().False());

        });
      });

      describe(".is_true()", []() {
        it("is false for 'FALSE'", []() {
          test_map_pair a(0, test_map_value::FALSE);
          AssertThat(a.is_true(), Is().False());
        });

        it("is true for 'FALSE'", []() {
          test_map_pair a(0, test_map_value::TRUE);
          AssertThat(a.is_true(), Is().True());
        });

        it("is false for other", []() {
          test_map_pair a1(0, test_map_value::OTHER_1);
          AssertThat(a1.is_true(), Is().False());

          test_map_pair a2(0, test_map_value::OTHER_1);
          AssertThat(a2.is_true(), Is().False());
        });
      });

      describe("test_map_pair(label_t, bool)", []() {
        it("converts correctly from boolean value [0]", []() {
          test_map_pair a(42, false);
          AssertThat(a.key(), Is().EqualTo(42));
          AssertThat(a.value(), Is().EqualTo(test_map_value::FALSE));
          AssertThat(a.raw_value(), Is().EqualTo(0));
        });

        it("converts correctly from boolean value [1]", []() {
          test_map_pair a(42, true);
          AssertThat(a.key(), Is().EqualTo(42));
          AssertThat(a.value(), Is().EqualTo(test_map_value::TRUE));
          AssertThat(a.raw_value(), Is().EqualTo(1));
        });
      });

      describe("ordering '<' and '>'", []() {
        /*
        it("has 'FALSE' values precede 'TRUE' [<]", []() {
          AssertThat(test_map_pair(42, test_map_value::FALSE),
                                               Is().LessThan(test_map_pair(42, test_map_value::TRUE)));
          AssertThat(test_map_pair(42, test_map_value::TRUE),
                                               Is().Not().LessThan(test_map_pair(42, test_map_value::FALSE)));
        });

        it("has 'FALSE' values precede 'TRUE' [>]", []() {
          AssertThat(test_map_pair(42, test_map_value::TRUE),
                                               Is().GreaterThan(test_map_pair(42, test_map_value::FALSE)));
          AssertThat(test_map_pair(42, test_map_value::FALSE),
                                               Is().Not().GreaterThan(test_map_pair(42, test_map_value::TRUE)));
        });
        */

        it("sorts based on the variable order [1], [<]", []() {
          AssertThat(test_map_pair(21, test_map_value::TRUE),
                     Is().LessThan(test_map_pair(42, test_map_value::OTHER_1)));
          AssertThat(test_map_pair(42, test_map_value::OTHER_1),
                     Is().Not().LessThan(test_map_pair(21, test_map_value::TRUE)));
        });

        it("sorts based on the variable order [1], [>]", []() {
          AssertThat(test_map_pair(42, test_map_value::OTHER_1),
                     Is().GreaterThan(test_map_pair(21, test_map_value::TRUE)));
          AssertThat(test_map_pair(21, test_map_value::TRUE),
                     Is().Not().GreaterThan(test_map_pair(42, test_map_value::OTHER_1)));
        });

        it("sorts based on the variable order [2], [<]", []() {
          AssertThat(test_map_pair(20, test_map_value::FALSE),
                     Is().LessThan(test_map_pair(21, test_map_value::OTHER_1)));
          AssertThat(test_map_pair(21, test_map_value::OTHER_1),
                     Is().Not().LessThan(test_map_pair(20, test_map_value::FALSE)));
        });

        it("sorts based on the variable order [2], [>]", []() {
          AssertThat(test_map_pair(21, test_map_value::OTHER_1),
                     Is().GreaterThan(test_map_pair(20, test_map_value::FALSE)));
          AssertThat(test_map_pair(20, test_map_value::FALSE),
                     Is().Not().GreaterThan(test_map_pair(21, test_map_value::OTHER_1)));
        });
      });

      describe("equality '=='", []() {
        it("is true when both variable and value match [1]", [&]() {
          AssertThat(test_map_pair(1, test_map_value::OTHER_1),
                     Is().EqualTo(test_map_pair(1, test_map_value::OTHER_1)));
        });

        it("is true when both variable and value match [2]", [&]() {
          AssertThat(test_map_pair(2, test_map_value::FALSE),
                     Is().EqualTo(test_map_pair(2, test_map_value::FALSE)));
        });

        it("is true when both variable and value match [3]", [&]() {
          AssertThat(test_map_pair(42, test_map_value::TRUE),
                     Is().EqualTo(test_map_pair(42, test_map_value::TRUE)));
        });

        it("is false when the variable mismatches [1]", [&]() {
          AssertThat(test_map_pair(42, test_map_value::TRUE),
                     Is().Not().EqualTo(test_map_pair(21, test_map_value::TRUE)));
        });

        it("is false when the variable mismatches [2]", [&]() {
          AssertThat(test_map_pair(8, test_map_value::FALSE),
                     Is().Not().EqualTo(test_map_pair(10, test_map_value::FALSE)));
        });

        it("is false when the value mismatches [1]", [&]() {
          AssertThat(test_map_pair(42, test_map_value::OTHER_1),
                     Is().Not().EqualTo(test_map_pair(42, test_map_value::FALSE)));
          AssertThat(test_map_pair(42, test_map_value::OTHER_1),
                     Is().Not().EqualTo(test_map_pair(42, test_map_value::TRUE)));
        });

        it("is false when the value mismatches [2]", [&]() {
          AssertThat(test_map_pair(8, test_map_value::FALSE),
                     Is().Not().EqualTo(test_map_pair(8, test_map_value::OTHER_1)));
          AssertThat(test_map_pair(8, test_map_value::FALSE),
                     Is().Not().EqualTo(test_map_pair(8, test_map_value::TRUE)));
        });

        it("is false when the value mismatches [3]", [&]() {
          AssertThat(test_map_pair(8, test_map_value::TRUE),
                     Is().Not().EqualTo(test_map_pair(8, test_map_value::OTHER_1)));
          AssertThat(test_map_pair(13, test_map_value::TRUE),
                     Is().Not().EqualTo(test_map_pair(13, test_map_value::FALSE)));
        });
      });

      describe("negation '~", []() {
        it("turns 'FALSE' into 'TRUE' [1]", [&]() {
          AssertThat(~test_map_pair(1, test_map_value::FALSE),
                     Is().EqualTo(test_map_pair(1, test_map_value::TRUE)));
        });

        it("turns 'FALSE' into 'TRUE' [2]", [&]() {
          AssertThat(~test_map_pair(2, test_map_value::FALSE),
                     Is().EqualTo(test_map_pair(2, test_map_value::TRUE)));
        });

        it("turns 'TRUE' into 'FALSE' [1]", [&]() {
          AssertThat(~test_map_pair(3, test_map_value::FALSE),
                     Is().EqualTo(test_map_pair(3, test_map_value::TRUE)));
        });

        it("turns 'TRUE' into 'FALSE' [2]", [&]() {
          AssertThat(~test_map_pair(4, test_map_value::FALSE),
                     Is().EqualTo(test_map_pair(4, test_map_value::TRUE)));
        });

        it("keeps 'NONE' as-is [1]", [&]() {
          AssertThat(~test_map_pair(5, test_map_value::OTHER_1),
                     Is().EqualTo(test_map_pair(5, test_map_value::OTHER_1)));
        });

        it("keeps 'NONE' as-is [2]", [&]() {
          AssertThat(~test_map_pair(6, test_map_value::OTHER_1),
                     Is().EqualTo(test_map_pair(6, test_map_value::OTHER_1)));
        });
      });
    });
  });
 });
