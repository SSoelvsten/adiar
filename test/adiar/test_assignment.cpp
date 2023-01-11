#include "../test.h"

go_bandit([]() {
  describe("adiar/assignment.h", []() {
    describe("assignment", []() {
      it("'value_t' enum preserves truthity of a boolean", []() {
        AssertThat(assignment::FALSE, Is().False());
        AssertThat(assignment::TRUE, Is().True());
      });

      describe("assignment(label_t, enum), .var(), .value()", []() {
        it("provides access to variable [1]", []() {
          assignment a(42, assignment::FALSE);
          AssertThat(a.var(), Is().EqualTo(42u));
        });

        it("provides access to variable [2]", []() {
          assignment a(21, assignment::TRUE);
          AssertThat(a.var(), Is().EqualTo(21u));
        });

        it("provides access to variable [1]", []() {
          assignment a(42, assignment::NONE);
          AssertThat(a.value(), Is().EqualTo(assignment::NONE));
        });

        it("provides access to variable [2]", []() {
          assignment a(21, assignment::FALSE);
          AssertThat(a.value(), Is().EqualTo(assignment::FALSE));
        });

        it("provides access to variable [3]", []() {
          assignment a(21, assignment::TRUE);
          AssertThat(a.value(), Is().EqualTo(assignment::TRUE));
        });
      });

      describe("assignment(label_t, bool)", []() {
        it("converts correctly boolean into enum [0]", []() {
          assignment a(42, false);
          AssertThat(a.var(), Is().EqualTo(42u));
          AssertThat(a.value(), Is().EqualTo(assignment::FALSE));
        });

        it("converts correctly boolean into enum [1]", []() {
          assignment a(42, true);
          AssertThat(a.var(), Is().EqualTo(42u));
          AssertThat(a.value(), Is().EqualTo(assignment::TRUE));
        });
      });

      describe("ordering '<' and '>'", []() {
        /*
        it("has 'NONE' values precede 'FALSE' [<]", []() {
          AssertThat(assignment(42, assignment::NONE), Is().LessThan(assignment(42, assignment::FALSE)));
          AssertThat(assignment(42, assignment::FALSE), Is().Not().LessThan(assignment(42, assignment::NONE)));
        });

        it("has 'NONE' values precede 'FALSE' [>]", []() {
          AssertThat(assignment(42, assignment::FALSE), Is().GreaterThan(assignment(42, assignment::NONE)));
          AssertThat(assignment(42, assignment::NONE), Is().Not().GreaterThan(assignment(42, assignment::FALSE)));
        });

        it("has 'NONE' values precede 'TRUE' [<]", []() {
          AssertThat(assignment(42, assignment::NONE), Is().LessThan(assignment(42, assignment::TRUE)));
          AssertThat(assignment(42, assignment::TRUE), Is().Not().LessThan(assignment(42, assignment::NONE)));
        });

        it("has 'NONE' values precede 'TRUE' [>]", []() {
          AssertThat(assignment(42, assignment::TRUE), Is().GreaterThan(assignment(42, assignment::NONE)));
          AssertThat(assignment(42, assignment::NONE), Is().Not().GreaterThan(assignment(42, assignment::TRUE)));
        });

        it("has 'FALSE' values precede 'TRUE' [<]", []() {
          AssertThat(assignment(42, assignment::FALSE), Is().LessThan(assignment(42, assignment::TRUE)));
          AssertThat(assignment(42, assignment::TRUE), Is().Not().LessThan(assignment(42, assignment::FALSE)));
        });

        it("has 'FALSE' values precede 'TRUE' [>]", []() {
          AssertThat(assignment(42, assignment::TRUE), Is().GreaterThan(assignment(42, assignment::FALSE)));
          AssertThat(assignment(42, assignment::FALSE), Is().Not().GreaterThan(assignment(42, assignment::TRUE)));
        });
        */

        it("sorts based on the variable order [1], [<]", []() {
          AssertThat(assignment(21, assignment::TRUE), Is().LessThan(assignment(42, assignment::NONE)));
          AssertThat(assignment(42, assignment::NONE), Is().Not().LessThan(assignment(21, assignment::TRUE)));
        });

        it("sorts based on the variable order [1], [>]", []() {
          AssertThat(assignment(42, assignment::NONE), Is().GreaterThan(assignment(21, assignment::TRUE)));
          AssertThat(assignment(21, assignment::TRUE), Is().Not().GreaterThan(assignment(42, assignment::NONE)));
        });

        it("sorts based on the variable order [2], [<]", []() {
          AssertThat(assignment(20, assignment::FALSE), Is().LessThan(assignment(21, assignment::NONE)));
          AssertThat(assignment(21, assignment::NONE), Is().Not().LessThan(assignment(20, assignment::FALSE)));
        });

        it("sorts based on the variable order [2], [>]", []() {
          AssertThat(assignment(21, assignment::NONE), Is().GreaterThan(assignment(20, assignment::FALSE)));
          AssertThat(assignment(20, assignment::FALSE), Is().Not().GreaterThan(assignment(21, assignment::NONE)));
        });
      });

      describe("equality '=='", []() {
        it("is true when both variable and value match [1]", [&]() {
          AssertThat(assignment(1, assignment::NONE), Is().EqualTo(assignment(1, assignment::NONE)));
        });

        it("is true when both variable and value match [2]", [&]() {
          AssertThat(assignment(2, assignment::FALSE), Is().EqualTo(assignment(2, assignment::FALSE)));
        });

        it("is true when both variable and value match [3]", [&]() {
          AssertThat(assignment(42, assignment::TRUE), Is().EqualTo(assignment(42, assignment::TRUE)));
        });

        it("is false when the variable mismatches [1]", [&]() {
          AssertThat(assignment(42, assignment::TRUE), Is().Not().EqualTo(assignment(21, assignment::TRUE)));
        });

        it("is false when the variable mismatches [2]", [&]() {
          AssertThat(assignment(8, assignment::FALSE), Is().Not().EqualTo(assignment(10, assignment::FALSE)));
        });

        it("is false when the value mismatches [1]", [&]() {
          AssertThat(assignment(42, assignment::NONE), Is().Not().EqualTo(assignment(42, assignment::FALSE)));
          AssertThat(assignment(42, assignment::NONE), Is().Not().EqualTo(assignment(42, assignment::TRUE)));
        });

        it("is false when the value mismatches [2]", [&]() {
          AssertThat(assignment(8, assignment::FALSE), Is().Not().EqualTo(assignment(8, assignment::NONE)));
          AssertThat(assignment(8, assignment::FALSE), Is().Not().EqualTo(assignment(8, assignment::TRUE)));
        });

        it("is false when the value mismatches [3]", [&]() {
          AssertThat(assignment(8, assignment::TRUE), Is().Not().EqualTo(assignment(8, assignment::NONE)));
          AssertThat(assignment(13, assignment::TRUE), Is().Not().EqualTo(assignment(13, assignment::FALSE)));
        });
      });

      describe("negation '~", []() {
        it("turns 'FALSE' into 'TRUE' [1]", [&]() {
          AssertThat(~assignment(1, assignment::FALSE), Is().EqualTo(assignment(1, assignment::TRUE)));
        });

        it("turns 'FALSE' into 'TRUE' [2]", [&]() {
          AssertThat(~assignment(2, assignment::FALSE), Is().EqualTo(assignment(2, assignment::TRUE)));
        });

        it("turns 'TRUE' into 'FALSE' [1]", [&]() {
          AssertThat(~assignment(3, assignment::FALSE), Is().EqualTo(assignment(3, assignment::TRUE)));
        });

        it("turns 'TRUE' into 'FALSE' [2]", [&]() {
          AssertThat(~assignment(4, assignment::FALSE), Is().EqualTo(assignment(4, assignment::TRUE)));
        });

        it("keeps 'NONE' as-is [1]", [&]() {
          AssertThat(~assignment(5, assignment::NONE), Is().EqualTo(assignment(5, assignment::NONE)));
        });

        it("keeps 'NONE' as-is [2]", [&]() {
          AssertThat(~assignment(6, assignment::NONE), Is().EqualTo(assignment(6, assignment::NONE)));
        });
      });
    });
  });
 });
