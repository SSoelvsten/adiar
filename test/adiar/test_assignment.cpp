#include "../test.h"

go_bandit([]() {
  describe("adiar/assignment.h", []() {
    describe("assignment", [&]() {
      const assignment a1(2, false);
      const assignment a2(2, true);
      const assignment a3(3, false);

      it("is sorted first by label", [&]() {
        // Less than
        AssertThat(a1 < a3, Is().True());
        AssertThat(a2 < a3, Is().True());
        AssertThat(a3 < a1, Is().False());
        AssertThat(a3 < a2, Is().False());

        // Greater than
        AssertThat(a3 > a1, Is().True());
        AssertThat(a3 > a2, Is().True());
        AssertThat(a1 > a3, Is().False());
        AssertThat(a2 > a3, Is().False());
      });

      it("is not sorted by value second", [&]() {
        // Less than
        AssertThat(a1 < a2, Is().False());

        // Greater than
        AssertThat(a2 > a1, Is().False());
      });

      const assignment b1(2, false);
      const assignment b2(2, true);
      const assignment b3(3, false);

      it("should be equal by content", [&]() {
        AssertThat(a1 == b1, Is().True());
        AssertThat(a2 == b2, Is().True());
        AssertThat(a3 == b3, Is().True());
        AssertThat(a2 == b1, Is().False());
        AssertThat(a3 == b1, Is().False());
        AssertThat(a3 == b2, Is().False());
      });

      it("should be equal by content", [&]() {
        AssertThat(a2 != b1, Is().True());
        AssertThat(a3 != b1, Is().True());
        AssertThat(a3 != b2, Is().True());
        AssertThat(a1 != b1, Is().False());
        AssertThat(a2 != b2, Is().False());
        AssertThat(a3 != b3, Is().False());
      });

      it("can be negated on its value", [&]() {
        AssertThat(~a1, Is().EqualTo(assignment(2, true)));
        AssertThat(~a2, Is().EqualTo(assignment(2, false)));
        AssertThat(~a3, Is().EqualTo(assignment(3, true)));
      });
    });
  });
 });
