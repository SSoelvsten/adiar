#include "../test.h"

go_bandit([]() {
  describe("adiar/domain.h", []() {

    it("should return false on adiar_has_domain() initially", [&]() {
      AssertThat(adiar_has_domain(), Is().False());
    });

    it("should return true on adiar_has_domain() after domain is added", [&]() {
      label_file dom;

      { // Garbage collect writer to free write-lock
        label_writer lw(dom);
        lw << 1 << 2 << 3;
      }

      adiar_set_domain(dom);

      AssertThat(adiar_has_domain(), Is().True());
    });

    it("should return the correct domain on adiar_get_domain()", [&]() {
      label_file dom;

      { // Garbage collect writer to free write-lock
        label_writer lw(dom);
        lw << 1 << 2 << 3;
      }

      adiar_set_domain(dom);

      AssertThat(adiar_get_domain(), Is().EqualTo(dom));
    });

    it("should return the correct domain on adiar_get_domain() after domain change", [&]() {
      label_file dom1;

      { // Garbage collect writer to free write-lock
        label_writer lw(dom1);
        lw << 1 << 2 << 3;
      }

      label_file dom2;

      { // Garbage collect writer to free write-lock
        label_writer lw(dom2);
        lw << 4 << 9 << 35;
      }

      adiar_set_domain(dom2);

      AssertThat(adiar_get_domain(), Is().EqualTo(dom2));
    });

  });
});
