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
      writer << node(0, node::max_id, false_ptr, true_ptr);
    }

    shared_levelized_file<node> x42;
    {
      node_writer writer(x42);
      writer << node(42, node::max_id, false_ptr, true_ptr);
    }

    shared_levelized_file<bdd::node_type> bdd_file;
    {
      node n3 = node(4, node::max_id, true_ptr, false_ptr);
      node n2 = node(3, node::max_id, n3.uid(), false_ptr);
      node n1 = node(1, node::max_id, n3.uid(), n2.uid());
      node_writer writer(bdd_file);
      writer << n3 << n2 << n1;
    }

    shared_levelized_file<zdd::node_type> zdd_file;
    {
      node n4 = node(2, node::max_id, true_ptr, true_ptr);
      node n3 = node(2, node::max_id-1, false_ptr, true_ptr);
      node n2 = node(1, node::max_id, n4.uid(), n4.uid());
      node n1 = node(0, node::max_id, n3.uid(), n4.uid());
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

    describe("canonicity, i.e. sorted & indexable", [&]() {
      // TODO
    });

    describe("terminal predicates [dd_...]", [&]() {
      describe("dd::is_terminal()", [&]() {
        // TODO
      });

      describe("dd_isterminal(...)", [&]() {
        it("rejects x0 as a BDD terminal file", [&]() {
          AssertThat(bdd_isterminal(x0), Is().False());
        });

        it("rejects x0 as a ZDD terminal file", [&]() {
          AssertThat(zdd_isterminal(x0), Is().False());
        });

        it("rejects larger BDD as being a terminal", [&]() {
          AssertThat(bdd_isterminal(bdd_file), Is().False());
        });

        it("rejects larger ZDD as being a terminal", [&]() {
          AssertThat(zdd_isterminal(zdd_file), Is().False());
        });

        it("accepts a BDD true terminal", [&]() {
          AssertThat(bdd_isterminal(terminal_T), Is().True());
        });

        it("accepts a ZDD true terminal", [&]() {
          AssertThat(zdd_isterminal(terminal_T), Is().True());
        });

        it("accepts a BDD false terminal", [&]() {
          AssertThat(bdd_isterminal(terminal_F), Is().True());
        });

        it("accepts a ZDD false terminal", [&]() {
          AssertThat(zdd_isterminal(terminal_F), Is().True());
        });
      });

      describe("dd::value()", [&]() {
        // TODO
      });

      describe("dd_valueof(...)", [&]() {
        it("extracts from a true BDD terminal", [&]() {
          AssertThat(dd_valueof(bdd(terminal_T)), Is().True());
        });

        it("extracts from a negation of a true BDD terminal", [&]() {
          AssertThat(dd_valueof(~ bdd(terminal_T)), Is().False());
        });

        it("extracts from a true ZDD terminal", [&]() {
          AssertThat(dd_valueof(zdd(terminal_T)), Is().True());
        });

        it("extracts from a false BDD terminal", [&]() {
          AssertThat(dd_valueof(bdd(terminal_F)), Is().False());
        });

        it("extracts from a negation of a false BDD terminal", [&]() {
          AssertThat(dd_valueof(~ bdd(terminal_F)), Is().True());
        });

        it("extracts from a false ZDD terminal", [&]() {
          AssertThat(dd_valueof(zdd(terminal_F)), Is().False());
        });
      });

      describe("dd::is_false()", [&]() {
        // TODO
      });

      describe("bdd_isfalse / zdd_isfalse / zdd_isempty", [&]() {
        it("rejects x0 [BDD]", [&]() {
          AssertThat(bdd_isfalse(x0), Is().False());
        });

        it("rejects x0 [ZDD]", [&]() {
          AssertThat(zdd_isfalse(x0), Is().False());
          AssertThat(zdd_isempty(x0), Is().False());
        });

        it("rejects x42 [BDD]", [&]() {
          AssertThat(bdd_isfalse(x42), Is().False());
        });

        it("rejects x42 [ZDD]", [&]() {
          AssertThat(zdd_isfalse(x42), Is().False());
          AssertThat(zdd_isempty(x42), Is().False());
        });

        it("rejects larger decision diagram [BDD]", [&]() {
          AssertThat(bdd_isfalse(bdd_file), Is().False());
        });

        it("rejects larger decision diagram [ZDD]", [&]() {
          AssertThat(zdd_isfalse(zdd_file), Is().False());
          AssertThat(zdd_isempty(zdd_file), Is().False());
        });

        it("rejects true terminal [BDD]", [&]() {
          AssertThat(bdd_isfalse(terminal_T), Is().False());
        });

        it("rejects true terminal [ZDD]", [&]() {
          AssertThat(zdd_isfalse(terminal_T), Is().False());
          AssertThat(zdd_isempty(terminal_T), Is().False());
        });

        it("accepts false terminal [BDD]", [&]() {
          AssertThat(bdd_isfalse(terminal_F), Is().True());
        });

        it("accepts false terminal [ZDD]", [&]() {
          AssertThat(zdd_isfalse(terminal_F), Is().True());
          AssertThat(zdd_isempty(terminal_F), Is().True());
        });
      });

      describe("dd::is_true()", [&]() {
        // TODO
      });

      describe("bdd_istrue / zdd_istrue / zdd_isnull", [&]() {
        it("rejects x0 [BDD]", [&]() {
          AssertThat(bdd_istrue(x0), Is().False());
        });

        it("rejects x0 [ZDD]", [&]() {
          AssertThat(zdd_istrue(x0), Is().False());
          AssertThat(zdd_isnull(x0), Is().False());
        });

        it("rejects x42 [BDD]", [&]() {
          AssertThat(bdd_istrue(x42), Is().False());
        });

        it("rejects x42 [ZDD]", [&]() {
          AssertThat(zdd_istrue(x42), Is().False());
          AssertThat(zdd_isnull(x42), Is().False());
        });

        it("rejects larger decision diagram [BDD]", [&]() {
          AssertThat(bdd_istrue(bdd_file), Is().False());
        });

        it("rejects larger decision diagram [ZDD]", [&]() {
          AssertThat(zdd_istrue(zdd_file), Is().False());
          AssertThat(zdd_isnull(zdd_file), Is().False());
        });

        it("rejects false terminal [BDD]", [&]() {
          AssertThat(bdd_istrue(terminal_F), Is().False());
        });

        it("rejects false terminal [ZDD]", [&]() {
          AssertThat(zdd_istrue(terminal_F), Is().False());
          AssertThat(zdd_isnull(terminal_F), Is().False());
        });

        it("accepts a true terminal [BDD]", [&]() {
          AssertThat(bdd_istrue(terminal_T), Is().True());
        });

        it("accepts a true terminal [ZDD]", [&]() {
          AssertThat(zdd_istrue(terminal_T), Is().True());
          AssertThat(zdd_isnull(terminal_T), Is().True());
        });
      });
    });

    describe("top and bottom-most variables", [&]() {
      // TODO
    });
  });
});
