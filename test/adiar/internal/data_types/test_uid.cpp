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

    describe("out-index", []() {
      it("should strip away out-index from 'internal node' pointer [1]", [&]() {
        const ptr_uint64 p1(53, 4, false);
        const uid_uint64 u1 = p1;

        const ptr_uint64 p2(53, 4, true);
        const uid_uint64 u2 = p2;

        AssertThat(u1, Is().EqualTo(u2));
      });

      it("should strip away out-index 'internal node' pointer [2]", [&]() {
        const ptr_uint64 p1(42, 0, false);
        const uid_uint64 u1 = p1;

        const ptr_uint64 p2(42, 0, true);
        const uid_uint64 u2 = p2;

        AssertThat(u1, Is().EqualTo(u2));
      });

      it("can provide a pointer with out-index 0", [&]() {
        const uid_uint64 u(42, 0);
        AssertThat(u.with(false), Is().EqualTo(ptr_uint64(42, 0, false)));
      });

      it("can provide a pointer with out-index 1", [&]() {
        const uid_uint64 u(42, 0);
        AssertThat(u.with(true), Is().EqualTo(ptr_uint64(42, 0, true)));
      });
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
