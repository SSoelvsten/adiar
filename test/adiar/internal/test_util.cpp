#include "../../test.h"

go_bandit([]() {
  describe("adiar/internal/util.h", []() {
    describe("dd_varprofile", []() {
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

      ptr_uint64 false_ptr = ptr_uint64(false);
      ptr_uint64 true_ptr = ptr_uint64(true);

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

      it("returns empty file for a BDD false terminal", [&]() {
        adiar::shared_file<bdd::label_t> label_file_out = bdd_varprofile(terminal_F);

         adiar::file_stream<bdd::label_t> out_labels(label_file_out);

        AssertThat(out_labels.can_pull(), Is().False());
      });

      it("returns empty file for a ZDD true terminal", [&]() {
        adiar::shared_file<zdd::label_t> label_file_out = zdd_varprofile(terminal_T);

         adiar::file_stream<zdd::label_t> out_labels(label_file_out);

        AssertThat(out_labels.can_pull(), Is().False());
      });

      it("returns [42] for a ZDD with one node (label 42)", [&]() {
        adiar::shared_file<zdd::label_t> label_file_out = zdd_varprofile(x42);

         adiar::file_stream<zdd::label_t> out_labels(label_file_out);

        AssertThat(out_labels.can_pull(), Is().True());
        AssertThat(out_labels.pull(), Is().EqualTo(42u));
        AssertThat(out_labels.can_pull(), Is().False());
      });

      it("returns [1,3,4] for a BDD with multiple nodes", [&]() {
        adiar::shared_file<bdd::label_t> label_file_out = bdd_varprofile(bdd_file);

         adiar::file_stream<bdd::label_t> out_labels(label_file_out);

        AssertThat(out_labels.can_pull(), Is().True());
        AssertThat(out_labels.pull(), Is().EqualTo(1u));

        AssertThat(out_labels.can_pull(), Is().True());
        AssertThat(out_labels.pull(), Is().EqualTo(3u));

        AssertThat(out_labels.can_pull(), Is().True());
        AssertThat(out_labels.pull(), Is().EqualTo(4u));

        AssertThat(out_labels.can_pull(), Is().False());
      });

      it("returns [0,1,2] for a ZDD with multiple nodes", [&]() {
        adiar::shared_file<zdd::label_t> label_file_out = zdd_varprofile(zdd_file);

         adiar::file_stream<zdd::label_t> out_labels(label_file_out);

        AssertThat(out_labels.can_pull(), Is().True());
        AssertThat(out_labels.pull(), Is().EqualTo(0u));

        AssertThat(out_labels.can_pull(), Is().True());
        AssertThat(out_labels.pull(), Is().EqualTo(1u));

        AssertThat(out_labels.can_pull(), Is().True());
        AssertThat(out_labels.pull(), Is().EqualTo(2u));

        AssertThat(out_labels.can_pull(), Is().False());
      });
    });
  });
});
