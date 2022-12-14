#include "../test.h"

go_bandit([]() {
  describe("adiar/assignment.h", []() {
    describe("assignment_t", [&]() {
      const assignment_t a1 = create_assignment(2, false);
      const assignment_t a2 = create_assignment(2, true);
      const assignment_t a3 = create_assignment(3, false);

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

      const assignment_t b1 = create_assignment(2, false);
      const assignment_t b2 = create_assignment(2, true);
      const assignment_t b3 = create_assignment(3, false);

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
        AssertThat(~a1, Is().EqualTo(create_assignment(2, true)));
        AssertThat(~a2, Is().EqualTo(create_assignment(2, false)));
        AssertThat(~a3, Is().EqualTo(create_assignment(3, true)));
      });
    });
  });
 });
