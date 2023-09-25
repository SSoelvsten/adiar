#include "../../test.h"

go_bandit([]() {
  describe("adiar/internal/dd_func.h", []() {
    const ptr_uint64 false_ptr = ptr_uint64(false);
    const ptr_uint64 true_ptr = ptr_uint64(true);

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

    shared_levelized_file<node> x0;
    {
      node_writer writer(x0);
      writer << node(0, node::MAX_ID, false_ptr, true_ptr);
    }

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

    describe("dd_varprofile(f, cb)", [&]() {
      // TODO
    });

    describe("dd_varprofile(f, begin, end)", [&]() {
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

      it("returns [0] for a ZDD with one node (label 0)", [&]() {
        std::vector<int> out(42, -1);

        auto it_b   = out.begin();
        auto it_e   = out.end();

        const auto out_it = zdd_varprofile(x0, it_b, it_e);

        AssertThat(out_it, Is().EqualTo(it_b+1));
        AssertThat(*it_b, Is().EqualTo(0));
        AssertThat(*out_it, Is().EqualTo(-1));
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

    describe("terminal predicates", [&]() {
      describe("is_terminal", [&]() {
        it("rejects x0 as a BDD terminal file", [&]() {
          AssertThat(is_terminal(bdd(x0)), Is().False());
        });

        it("rejects x0 as a ZDD terminal file", [&]() {
          AssertThat(is_terminal(zdd(x0)), Is().False());
        });

        it("rejects larger BDD as being a terminal", [&]() {
          AssertThat(is_terminal(bdd(bdd_file)), Is().False());
        });

        it("rejects larger ZDD as being a terminal", [&]() {
          AssertThat(is_terminal(zdd(zdd_file)), Is().False());
        });

        it("accepts a BDD true terminal", [&]() {
          AssertThat(is_terminal(bdd(terminal_T)), Is().True());
        });

        it("accepts a ZDD true terminal", [&]() {
          AssertThat(is_terminal(zdd(terminal_T)), Is().True());
        });

        it("accepts a BDD false terminal", [&]() {
          AssertThat(is_terminal(bdd(terminal_F)), Is().True());
        });

        it("accepts a ZDD false terminal", [&]() {
          AssertThat(is_terminal(zdd(terminal_F)), Is().True());
        });
      });

      describe("value_of", [&]() {
        it("extracts from a true BDD terminal", [&]() {
          AssertThat(value_of(bdd(terminal_T)), Is().True());
        });

        it("extracts from a negation of a true BDD terminal", [&]() {
          AssertThat(value_of(~ bdd(terminal_T)), Is().False());
        });

        it("extracts from a true ZDD terminal", [&]() {
          AssertThat(value_of(zdd(terminal_T)), Is().True());
        });

        it("extracts from a false BDD terminal", [&]() {
          AssertThat(value_of(bdd(terminal_F)), Is().False());
        });

        it("extracts from a negation of a false BDD terminal", [&]() {
          AssertThat(value_of(~ bdd(terminal_F)), Is().True());
        });

        it("extracts from a false ZDD terminal", [&]() {
          AssertThat(value_of(zdd(terminal_F)), Is().False());
        });
      });

      describe("is_false / is_empty", [&]() {
        it("rejects x0 [BDD]", [&]() {
          AssertThat(is_false(bdd(x0)), Is().False());
        });

        it("rejects x0 [ZDD]", [&]() {
          AssertThat(is_false(zdd(x0)), Is().False());
          AssertThat(is_empty(zdd(x0)), Is().False());
        });

        it("rejects x42 [BDD]", [&]() {
          AssertThat(is_false(bdd(x42)), Is().False());
        });

        it("rejects x42 [ZDD]", [&]() {
          AssertThat(is_false(zdd(x42)), Is().False());
          AssertThat(is_empty(zdd(x42)), Is().False());
        });

        it("rejects larger decision diagram [BDD]", [&]() {
          AssertThat(is_false(bdd(bdd_file)), Is().False());
        });

        it("rejects larger decision diagram [ZDD]", [&]() {
          AssertThat(is_false(zdd(zdd_file)), Is().False());
          AssertThat(is_empty(zdd(zdd_file)), Is().False());
        });

        it("rejects true terminal [BDD]", [&]() {
          AssertThat(is_false(bdd(terminal_T)), Is().False());
        });

        it("rejects true terminal [ZDD]", [&]() {
          AssertThat(is_false(zdd(terminal_T)), Is().False());
          AssertThat(is_empty(zdd(terminal_T)), Is().False());
        });

        it("accepts false terminal [BDD]", [&]() {
          AssertThat(is_false(bdd(terminal_F)), Is().True());
        });

        it("accepts false terminal [ZDD]", [&]() {
          AssertThat(is_false(zdd(terminal_F)), Is().True());
          AssertThat(is_empty(zdd(terminal_F)), Is().True());
        });
      });

      describe("is_true / is_null", [&]() {
        it("rejects x0 [BDD]", [&]() {
          AssertThat(is_true(bdd(x0)), Is().False());
        });

        it("rejects x0 [ZDD]", [&]() {
          AssertThat(is_true(zdd(x0)), Is().False());
          AssertThat(is_null(zdd(x0)), Is().False());
        });

        it("rejects x42 [BDD]", [&]() {
          AssertThat(is_true(bdd(x42)), Is().False());
        });

        it("rejects x42 [ZDD]", [&]() {
          AssertThat(is_true(zdd(x42)), Is().False());
          AssertThat(is_null(zdd(x42)), Is().False());
        });

        it("rejects larger decision diagram [BDD]", [&]() {
          AssertThat(is_true(bdd(bdd_file)), Is().False());
        });

        it("rejects larger decision diagram [ZDD]", [&]() {
          AssertThat(is_true(zdd(zdd_file)), Is().False());
          AssertThat(is_null(zdd(zdd_file)), Is().False());
        });

        it("rejects false terminal [BDD]", [&]() {
          AssertThat(is_true(bdd(terminal_F)), Is().False());
        });

        it("rejects false terminal [ZDD]", [&]() {
          AssertThat(is_true(zdd(terminal_F)), Is().False());
          AssertThat(is_null(zdd(terminal_F)), Is().False());
        });

        it("accepts a true terminal [BDD]", [&]() {
          AssertThat(is_true(bdd(terminal_T)), Is().True());
        });

        it("accepts a true terminal [ZDD]", [&]() {
          AssertThat(is_true(zdd(terminal_T)), Is().True());
          AssertThat(is_null(zdd(terminal_T)), Is().True());
        });
      });
    });
  });
});
