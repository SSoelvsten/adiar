go_bandit([]() {
    describe("CORE: union_t", [&]() {
        it("can construct without any elements", [&]() {
          union_t<int, bool> fu;

          AssertThat(fu.has<int>(), Is().False());
          AssertThat(fu.has<bool>(), Is().False());
        });

        it("can construct with element of type T1", [&]() {
          union_t<int, bool> fu(42);

          AssertThat(fu.has<int>(), Is().True());
          AssertThat(fu.get<int>(), Is().EqualTo(42));
          AssertThat(fu.has<bool>(), Is().False());
        });

        it("can construct with element of type T2", [&]() {
          union_t<int, bool> fu(true);

          AssertThat(fu.has<int>(), Is().False());
          AssertThat(fu.has<bool>(), Is().True());
          AssertThat(fu.get<bool>(), Is().EqualTo(true));
        });

        it("can replace element of type T1", [&]() {
          union_t<int, bool> fu(21);

          AssertThat(fu.has<int>(), Is().True());
          AssertThat(fu.get<int>(), Is().EqualTo(21));
          AssertThat(fu.has<bool>(), Is().False());

          fu.set(42);

          AssertThat(fu.has<int>(), Is().True());
          AssertThat(fu.get<int>(), Is().EqualTo(42));
          AssertThat(fu.has<bool>(), Is().False());
        });

        it("can replace element of type T2", [&]() {
          union_t<int, bool> fu(false);

          AssertThat(fu.has<int>(), Is().False());
          AssertThat(fu.has<bool>(), Is().True());
          AssertThat(fu.get<bool>(), Is().EqualTo(false));

          fu.set(true);

          AssertThat(fu.has<int>(), Is().False());
          AssertThat(fu.has<bool>(), Is().True());
          AssertThat(fu.get<bool>(), Is().EqualTo(true));
        });

        it("can replace element of type T2 with element of type T1", [&]() {
          union_t<int, bool> fu(false);

          AssertThat(fu.has<int>(), Is().False());
          AssertThat(fu.has<bool>(), Is().True());
          AssertThat(fu.get<bool>(), Is().EqualTo(false));

          fu.set(7);

          AssertThat(fu.has<int>(), Is().True());
          AssertThat(fu.get<int>(), Is().EqualTo(7));
          AssertThat(fu.has<bool>(), Is().False());
        });

        it("can replace element of type T1 with element of type T2", [&]() {
          union_t<int, bool> fu(42);

          AssertThat(fu.has<int>(), Is().True());
          AssertThat(fu.has<bool>(), Is().False());

          fu.set(true);

          AssertThat(fu.has<int>(), Is().False());
          AssertThat(fu.has<bool>(), Is().True());
          AssertThat(fu.get<bool>(), Is().EqualTo(true));
        });
     });
  });
