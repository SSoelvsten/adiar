go_bandit([]() {
  describe("adiar/internal/data_types/uid.h", []() {
    describe("flag", []() {
      it("should strip away flag when copying from 'false' pointer", [&]() {
        const ptr_uint64 p(false);
        const uid u = flag(p);

        AssertThat(p.is_flagged(), Is().False());
        AssertThat(u, Is().EqualTo(p));
      });

      it("should strip away flag when copying from 'true' pointer", [&]() {
        const ptr_uint64 p(true);
        const uid u = flag(p);

        AssertThat(p.is_flagged(), Is().False());
        AssertThat(u, Is().EqualTo(p));
      });

      it("should strip away flag when copying from 'internal node' pointer", [&]() {
        const ptr_uint64 p(53, 4);
        const uid u = flag(p);

        AssertThat(p.is_flagged(), Is().False());
        AssertThat(u, Is().EqualTo(p));
      });
    });

    describe("NIL", [&](){
#ifndef NDEBUG
      it("throws 'std::illegal_argument' when given NIL [unflagged]", [&]() {
        AssertThrows(std::invalid_argument, uid(ptr_uint64::NIL()));
      });

      it("throws 'std::illegal_argument' when given NIL [flagged]", [&]() {
        AssertThrows(std::invalid_argument, uid(flag(ptr_uint64::NIL())));
      });
#endif
    });

    describe("terminals", [&](){
      it("should take up 8 bytes of memory", [&]() {
        AssertThat(sizeof(uid(false)), Is().EqualTo(8u));
        AssertThat(sizeof(uid(true)),  Is().EqualTo(8u));
      });

      describe("is_terminal", []() {
      });

      describe("value", []() {
      });

      describe("is_false", []() {
      });

      describe("is_true", []() {
      });
    });

    describe("internal nodes", [&](){
      it("should take up 8 bytes of memory", [&]() {
        AssertThat(sizeof(uid(0,0)),               Is().EqualTo(8u));
        AssertThat(sizeof(uid(42,21)),             Is().EqualTo(8u));
        AssertThat(sizeof(uid(MAX_LABEL, MAX_ID)), Is().EqualTo(8u));
      });

      describe("is_node", []() {
      });

      describe("label", []() {
      });

      describe("id", []() {
      });
    });
  });
 });
