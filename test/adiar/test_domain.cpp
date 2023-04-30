#include "../test.h"

go_bandit([]() {
  adiar_unset_domain();

  describe("adiar/domain.h", []() {
    describe("adiar_has_domain(), adiar_get_domain()", []() {
      it("throws exception when getting missing domain", [&]() {
        AssertThat(adiar_has_domain(), Is().False());
        AssertThrows(std::domain_error, adiar_get_domain());
      });
    });

    describe("adiar_has_domain(), adiar_set_domain(...), adiar_unset_domain()", []() {
      it("has domain after 'adiar_set_domain(file)'", [&]() {
        adiar::shared_file<node::label_t> dom;

        { // Garbage collect writer to free write-lock
          label_writer lw(dom);
          lw << 1 << 2 << 3;
        }

        AssertThat(adiar_has_domain(), Is().False());
        adiar_set_domain(dom);
        AssertThat(adiar_has_domain(), Is().True());
      });

      it("can remove prior domain", [&]() {
        AssertThat(adiar_has_domain(), Is().True());
        adiar_unset_domain();
        AssertThat(adiar_has_domain(), Is().False());
      });

      it("can remove with no prior domain", [&]() {
        AssertThat(adiar_has_domain(), Is().False());
        adiar_unset_domain();
        AssertThat(adiar_has_domain(), Is().False());
      });

      it("has domain after 'adiar_set_domain(varcount)'", [&]() {
        AssertThat(adiar_has_domain(), Is().False());
        adiar_set_domain(42);
        AssertThat(adiar_has_domain(), Is().True());
      });
    });

    describe("adiar_has_domain(), adiar_get_domain(), adiar_set_domain(...)", []() {
      it("gives back domain with varcount [0]", [&]() {
        adiar_unset_domain();
        adiar_set_domain(0);

        AssertThat(adiar_has_domain(), Is().True());

        adiar::file_stream<node::label_t> ls(adiar_get_domain());

        AssertThat(ls.can_pull(), Is().False());
      });

      it("gives back domain with varcount [1]", [&]() {
        adiar_unset_domain();
        adiar_set_domain(1);

        AssertThat(adiar_has_domain(), Is().True());

        adiar::file_stream<node::label_t> ls(adiar_get_domain());

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(0u));

        AssertThat(ls.can_pull(), Is().False());
      });

      it("gives back domain with varcount [7]", [&]() {
        adiar_unset_domain();
        adiar_set_domain(7);

        AssertThat(adiar_has_domain(), Is().True());

        adiar::file_stream<node::label_t> ls(adiar_get_domain());

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(0u));

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(1u));

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(2u));

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(3u));

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(4u));

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(5u));

        AssertThat(ls.can_pull(), Is().True());
        AssertThat(ls.pull(), Is().EqualTo(6u));

        AssertThat(ls.can_pull(), Is().False());
      });

      it("can overwrite with new varcount [2,3]", [&]() {
        adiar_unset_domain();

        adiar_set_domain(3);
        {
          AssertThat(adiar_has_domain(), Is().True());

          adiar::file_stream<node::label_t> ls(adiar_get_domain());

          AssertThat(ls.can_pull(), Is().True());
          AssertThat(ls.pull(), Is().EqualTo(0u));

          AssertThat(ls.can_pull(), Is().True());
          AssertThat(ls.pull(), Is().EqualTo(1u));

          AssertThat(ls.can_pull(), Is().True());
          AssertThat(ls.pull(), Is().EqualTo(2u));

          AssertThat(ls.can_pull(), Is().False());
        }

        adiar_set_domain(2);
        {
          AssertThat(adiar_has_domain(), Is().True());

          adiar::file_stream<node::label_t> ls(adiar_get_domain());

          AssertThat(ls.can_pull(), Is().True());
          AssertThat(ls.pull(), Is().EqualTo(0u));

          AssertThat(ls.can_pull(), Is().True());
          AssertThat(ls.pull(), Is().EqualTo(1u));

          AssertThat(ls.can_pull(), Is().False());
        }
      });

      it("gives back the given domain file", [&]() {
        adiar_unset_domain();

        adiar::shared_file<node::label_t> dom;

        { // Garbage collect writer to free write-lock
          label_writer lw(dom);
          lw << 1 << 2 << 3;
        }

        adiar_set_domain(dom);

        AssertThat(adiar_has_domain(), Is().True());
        AssertThat(adiar_get_domain(), Is().EqualTo(dom));
      });

      it("can overwrite with another domain file", [&]() {
        adiar_unset_domain();

        adiar::shared_file<node::label_t> dom1;

        { // Garbage collect writer to free write-lock
          label_writer lw(dom1);
          lw << 1 << 2 << 3;
        }

        adiar_set_domain(dom1);

        AssertThat(adiar_has_domain(), Is().True());
        AssertThat(adiar_get_domain(), Is().EqualTo(dom1));

        adiar::shared_file<node::label_t> dom2;

        { // Garbage collect writer to free write-lock
          label_writer lw(dom2);
          lw << 4 << 9 << 35;
        }

        adiar_set_domain(dom2);

        AssertThat(adiar_has_domain(), Is().True());
        AssertThat(adiar_get_domain(), Is().EqualTo(dom2));
      });
    });

    // TODO: more tests when 'https://github.com/thomasmoelhave/tpie/issues/265'
    //       is resolved.
  });
});
