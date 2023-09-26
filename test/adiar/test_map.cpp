#include "../test.h"

enum class test_map_value
{
  False   = 0,
  True    = 1,
  Other_1 = 2,
  Other_2 = 4,
};

using test_map_pair = map_pair<int, test_map_value>;

go_bandit([]() {
  describe("adiar/map.h", []() {
    describe("map_pair<key, value>", []() {
      describe("var_mapping<...>(label_type, enum), .key(), .value(), .raw_value()", []() {
        it("provides access to variable [42]", []() {
          test_map_pair a(42, test_map_value::False);
          AssertThat(a.key(), Is().EqualTo(42));
        });

        it("provides access to variable [21]", []() {
          test_map_pair a(21, test_map_value::True);
          AssertThat(a.key(), Is().EqualTo(21));
        });

        it("provides access to value [0]", []() {
          test_map_pair a(21, test_map_value::False);
          AssertThat(a.value(), Is().EqualTo(test_map_value::False));
        });

        it("provides access to value [1]", []() {
          test_map_pair a(21, test_map_value::True);
          AssertThat(a.value(), Is().EqualTo(test_map_value::True));
        });

        it("provides access to value [2]", []() {
          test_map_pair a(42, test_map_value::Other_1);
          AssertThat(a.value(), Is().EqualTo(test_map_value::Other_1));
        });

        it("provides access to value [4]", []() {
          test_map_pair a(42, test_map_value::Other_2);
          AssertThat(a.value(), Is().EqualTo(test_map_value::Other_2));
        });

        it("provides access to raw value [0]", []() {
          test_map_pair a(21, test_map_value::False);
          AssertThat(a.raw_value(), Is().EqualTo(0));
        });

        it("provides access to raw value [1]", []() {
          test_map_pair a(21, test_map_value::True);
          AssertThat(a.raw_value(), Is().EqualTo(1));
        });

        it("provides access to raw value [2]", []() {
          test_map_pair a(42, test_map_value::Other_1);
          AssertThat(a.raw_value(), Is().EqualTo(2));
        });

        it("provides access to raw value [4]", []() {
          test_map_pair a(42, test_map_value::Other_2);
          AssertThat(a.raw_value(), Is().EqualTo(4));
        });
      });

      describe(".is_false()", []() {
        it("is true for 'False'", []() {
          test_map_pair a(0, test_map_value::False);
          AssertThat(a.is_false(), Is().True());
        });

        it("is false for 'False'", []() {
          test_map_pair a(0, test_map_value::True);
          AssertThat(a.is_false(), Is().False());
        });

        it("is false for other", []() {
          test_map_pair a1(0, test_map_value::Other_1);
          AssertThat(a1.is_false(), Is().False());

          test_map_pair a2(0, test_map_value::Other_2);
          AssertThat(a2.is_false(), Is().False());

        });
      });

      describe(".is_true()", []() {
        it("is false for 'False'", []() {
          test_map_pair a(0, test_map_value::False);
          AssertThat(a.is_true(), Is().False());
        });

        it("is true for 'False'", []() {
          test_map_pair a(0, test_map_value::True);
          AssertThat(a.is_true(), Is().True());
        });

        it("is false for other", []() {
          test_map_pair a1(0, test_map_value::Other_1);
          AssertThat(a1.is_true(), Is().False());

          test_map_pair a2(0, test_map_value::Other_1);
          AssertThat(a2.is_true(), Is().False());
        });
      });

      describe("test_map_pair(label_type, bool)", []() {
        it("converts correctly from boolean value [0]", []() {
          test_map_pair a(42, false);
          AssertThat(a.key(), Is().EqualTo(42));
          AssertThat(a.value(), Is().EqualTo(test_map_value::False));
          AssertThat(a.raw_value(), Is().EqualTo(0));
        });

        it("converts correctly from boolean value [1]", []() {
          test_map_pair a(42, true);
          AssertThat(a.key(), Is().EqualTo(42));
          AssertThat(a.value(), Is().EqualTo(test_map_value::True));
          AssertThat(a.raw_value(), Is().EqualTo(1));
        });
      });

      describe("ordering '<' and '>'", []() {
        /*
        it("has 'False' values precede 'True' [<]", []() {
          AssertThat(test_map_pair(42, test_map_value::False),
                                               Is().LessThan(test_map_pair(42, test_map_value::True)));
          AssertThat(test_map_pair(42, test_map_value::True),
                                               Is().Not().LessThan(test_map_pair(42, test_map_value::False)));
        });

        it("has 'False' values precede 'True' [>]", []() {
          AssertThat(test_map_pair(42, test_map_value::True),
                                               Is().GreaterThan(test_map_pair(42, test_map_value::False)));
          AssertThat(test_map_pair(42, test_map_value::False),
                                               Is().Not().GreaterThan(test_map_pair(42, test_map_value::True)));
        });
        */

        it("sorts based on the variable order [1], [<]", []() {
          AssertThat(test_map_pair(21, test_map_value::True),
                     Is().LessThan(test_map_pair(42, test_map_value::Other_1)));
          AssertThat(test_map_pair(42, test_map_value::Other_1),
                     Is().Not().LessThan(test_map_pair(21, test_map_value::True)));
        });

        it("sorts based on the variable order [1], [>]", []() {
          AssertThat(test_map_pair(42, test_map_value::Other_1),
                     Is().GreaterThan(test_map_pair(21, test_map_value::True)));
          AssertThat(test_map_pair(21, test_map_value::True),
                     Is().Not().GreaterThan(test_map_pair(42, test_map_value::Other_1)));
        });

        it("sorts based on the variable order [2], [<]", []() {
          AssertThat(test_map_pair(20, test_map_value::False),
                     Is().LessThan(test_map_pair(21, test_map_value::Other_1)));
          AssertThat(test_map_pair(21, test_map_value::Other_1),
                     Is().Not().LessThan(test_map_pair(20, test_map_value::False)));
        });

        it("sorts based on the variable order [2], [>]", []() {
          AssertThat(test_map_pair(21, test_map_value::Other_1),
                     Is().GreaterThan(test_map_pair(20, test_map_value::False)));
          AssertThat(test_map_pair(20, test_map_value::False),
                     Is().Not().GreaterThan(test_map_pair(21, test_map_value::Other_1)));
        });
      });

      describe("equality '=='", []() {
        it("is true when both variable and value match [1]", [&]() {
          AssertThat(test_map_pair(1, test_map_value::Other_1),
                     Is().EqualTo(test_map_pair(1, test_map_value::Other_1)));
        });

        it("is true when both variable and value match [2]", [&]() {
          AssertThat(test_map_pair(2, test_map_value::False),
                     Is().EqualTo(test_map_pair(2, test_map_value::False)));
        });

        it("is true when both variable and value match [3]", [&]() {
          AssertThat(test_map_pair(42, test_map_value::True),
                     Is().EqualTo(test_map_pair(42, test_map_value::True)));
        });

        it("is false when the variable mismatches [1]", [&]() {
          AssertThat(test_map_pair(42, test_map_value::True),
                     Is().Not().EqualTo(test_map_pair(21, test_map_value::True)));
        });

        it("is false when the variable mismatches [2]", [&]() {
          AssertThat(test_map_pair(8, test_map_value::False),
                     Is().Not().EqualTo(test_map_pair(10, test_map_value::False)));
        });

        it("is false when the value mismatches [1]", [&]() {
          AssertThat(test_map_pair(42, test_map_value::Other_1),
                     Is().Not().EqualTo(test_map_pair(42, test_map_value::False)));
          AssertThat(test_map_pair(42, test_map_value::Other_1),
                     Is().Not().EqualTo(test_map_pair(42, test_map_value::True)));
        });

        it("is false when the value mismatches [2]", [&]() {
          AssertThat(test_map_pair(8, test_map_value::False),
                     Is().Not().EqualTo(test_map_pair(8, test_map_value::Other_1)));
          AssertThat(test_map_pair(8, test_map_value::False),
                     Is().Not().EqualTo(test_map_pair(8, test_map_value::True)));
        });

        it("is false when the value mismatches [3]", [&]() {
          AssertThat(test_map_pair(8, test_map_value::True),
                     Is().Not().EqualTo(test_map_pair(8, test_map_value::Other_1)));
          AssertThat(test_map_pair(13, test_map_value::True),
                     Is().Not().EqualTo(test_map_pair(13, test_map_value::False)));
        });
      });

      describe("negation '~", []() {
        it("turns 'False' into 'True' [1]", [&]() {
          AssertThat(~test_map_pair(1, test_map_value::False),
                     Is().EqualTo(test_map_pair(1, test_map_value::True)));
        });

        it("turns 'False' into 'True' [2]", [&]() {
          AssertThat(~test_map_pair(2, test_map_value::False),
                     Is().EqualTo(test_map_pair(2, test_map_value::True)));
        });

        it("turns 'True' into 'False' [1]", [&]() {
          AssertThat(~test_map_pair(3, test_map_value::False),
                     Is().EqualTo(test_map_pair(3, test_map_value::True)));
        });

        it("turns 'True' into 'False' [2]", [&]() {
          AssertThat(~test_map_pair(4, test_map_value::False),
                     Is().EqualTo(test_map_pair(4, test_map_value::True)));
        });

        it("keeps 'None' as-is [1]", [&]() {
          AssertThat(~test_map_pair(5, test_map_value::Other_1),
                     Is().EqualTo(test_map_pair(5, test_map_value::Other_1)));
        });

        it("keeps 'None' as-is [2]", [&]() {
          AssertThat(~test_map_pair(6, test_map_value::Other_1),
                     Is().EqualTo(test_map_pair(6, test_map_value::Other_1)));
        });
      });
    });
  });
 });
