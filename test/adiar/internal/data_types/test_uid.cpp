#include "../../../test.h"

go_bandit([]() {
  describe("adiar/internal/data_types/uid.h", []() {
    describe("flag", []() {
      it("should strip away flag when copying from 'false' pointer", [&]() {
        const ptr_uint64 p(false);
        const uid_uint64 u = flag(p);

        AssertThat(p.is_flagged(), Is().False());
        AssertThat(u, Is().EqualTo(p));
      });

      it("should strip away flag when copying from 'true' pointer", [&]() {
        const ptr_uint64 p(true);
        const uid_uint64 u = flag(p);

        AssertThat(p.is_flagged(), Is().False());
        AssertThat(u, Is().EqualTo(p));
      });

      it("should strip away flag when copying from 'internal node' pointer", [&]() {
        const ptr_uint64 p(53, 4);
        const uid_uint64 u = flag(p);

        AssertThat(p.is_flagged(), Is().False());
        AssertThat(u, Is().EqualTo(p));
      });
    });

    describe("NIL", [&](){
#ifndef NDEBUG
      it("throws 'std::illegal_argument' when given NIL [unflagged]", [&]() {
        AssertThrows(std::invalid_argument, uid_uint64(ptr_uint64::NIL()));
      });

      it("throws 'std::illegal_argument' when given NIL [flagged]", [&]() {
        AssertThrows(std::invalid_argument, uid_uint64(flag(ptr_uint64::NIL())));
      });
#endif
    });

    describe("terminals", [&](){
      it("should take up 8 bytes of memory", [&]() {
        AssertThat(sizeof(uid_uint64(false)), Is().EqualTo(8u));
        AssertThat(sizeof(uid_uint64(true)),  Is().EqualTo(8u));
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
        AssertThat(sizeof(uid_uint64(0,0)),               Is().EqualTo(8u));
        AssertThat(sizeof(uid_uint64(42,21)),             Is().EqualTo(8u));
        AssertThat(sizeof(uid_uint64(uid_uint64::MAX_LABEL, uid_uint64::MAX_ID)), Is().EqualTo(8u));
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
