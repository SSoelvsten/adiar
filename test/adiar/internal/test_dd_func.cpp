#include "../../test.h"

go_bandit([]() {
  describe("adiar/internal/util.h", []() {
    describe("dd_varprofile(f, cb)", []() {
      // TODO
    });

    describe("dd_varprofile(f, begin, end)", []() {
      shared_levelized_file<node> terminal_F;
      {
        node_writer writer(terminal_F);
        writer << node(false);
      }

      shared_levelized_file<node> terminal_T;
      {
        node_writer writer(terminal_T);
        writer << node(true);
      }

      const ptr_uint64 false_ptr = ptr_uint64(false);
      const ptr_uint64 true_ptr = ptr_uint64(true);

      shared_levelized_file<node> x42;
      {
        node_writer writer(x42);
        writer << node(42, node::MAX_ID, false_ptr, true_ptr);
      }

      shared_levelized_file<bdd::node_t> bdd_file;
      {
        node n3 = node(4, node::MAX_ID, true_ptr, false_ptr);
        node n2 = node(3, node::MAX_ID, n3.uid(), false_ptr);
        node n1 = node(1, node::MAX_ID, n3.uid(), n2.uid());
        node_writer writer(bdd_file);
        writer << n3 << n2 << n1;
      }

      shared_levelized_file<zdd::node_t> zdd_file;
      {
        node n4 = node(2, node::MAX_ID, true_ptr, true_ptr);
        node n3 = node(2, node::MAX_ID-1, false_ptr, true_ptr);
        node n2 = node(1, node::MAX_ID, n4.uid(), n4.uid());
        node n1 = node(0, node::MAX_ID, n3.uid(), n4.uid());
        node_writer writer(zdd_file);
        writer << n4 << n3 << n2 << n1;
      }

      it("does not write anything for BDD false terminal", [&]() {
        std::vector<int> out(42, -1);

        auto it_b   = out.begin();
        auto it_e   = out.end();

        const auto out_it = bdd_varprofile(terminal_F, it_b, it_e);

        AssertThat(out_it, Is().EqualTo(it_b));
        AssertThat(*it_b, Is().EqualTo(-1));
      });

      it("returns empty file for a ZDD true terminal", [&]() {
        std::vector<int> out(42, -1);

        auto it_b   = out.begin();
        auto it_e   = out.end();

        const auto out_it = zdd_varprofile(terminal_T, it_b, it_e);

        AssertThat(out_it, Is().EqualTo(it_b));
        AssertThat(*it_b, Is().EqualTo(-1));
      });

      it("returns [42] for a ZDD with one node (label 42)", [&]() {
        std::vector<int> out(42, -1);

        auto it_b   = out.begin();
        auto it_e   = out.end();

        const auto out_it = zdd_varprofile(x42, it_b, it_e);

        AssertThat(out_it, Is().EqualTo(it_b+1));
        AssertThat(*it_b, Is().EqualTo(42));
        AssertThat(*out_it, Is().EqualTo(-1));
      });

      it("returns [1,3,4] for a BDD with multiple nodes", [&]() {
        std::vector<int> out(42, -1);

        auto it_b   = out.begin();
        auto it_e   = out.end();

        const auto out_it = bdd_varprofile(bdd_file, it_b, it_e);

        AssertThat(*(it_b+0), Is().EqualTo(1));
        AssertThat(*(it_b+1), Is().EqualTo(3));
        AssertThat(*(it_b+2), Is().EqualTo(4));

        AssertThat(out_it, Is().EqualTo(it_b+3));
        AssertThat(*out_it, Is().EqualTo(-1));
      });

      it("returns [0,1,2] for a ZDD with multiple nodes", [&]() {
        std::vector<int> out(42, -1);

        auto it_b   = out.begin();
        auto it_e   = out.end();

        const auto out_it = zdd_varprofile(zdd_file, it_b, it_e);

        AssertThat(*(it_b+0), Is().EqualTo(0));
        AssertThat(*(it_b+1), Is().EqualTo(1));
        AssertThat(*(it_b+2), Is().EqualTo(2));

        AssertThat(out_it, Is().EqualTo(it_b+3));
        AssertThat(*out_it, Is().EqualTo(-1));
      });

      it("throws if range is too small for BDD", [&]() {
        std::vector<int> out(1,-1);
        AssertThrows(out_of_range, bdd_varprofile(bdd_file, out.begin(), out.end()));
      });

      it("throws if range is too small for ZDD", [&]() {
        std::vector<int> out(1,-1);
        AssertThrows(out_of_range, zdd_varprofile(zdd_file, out.begin(), out.end()));
      });
    });
  });
});
