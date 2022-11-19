#include "../../test.h"

go_bandit([]() {
  describe("adiar/internal/util.h", []() {
    describe("varprofile", []() {
      node_file terminal_F;
      {
        node_writer writer(terminal_F);
        writer << node(false);
      }

      node_file terminal_T;
      {
        node_writer writer(terminal_T);
        writer << node(true);
      }

      ptr_uint64 false_ptr = ptr_uint64(false);
      ptr_uint64 true_ptr = ptr_uint64(true);

      node_file x42;
      {
        node_writer writer(x42);
        writer << node(42, node::MAX_ID, false_ptr, true_ptr);
      }

      node_file bdd_file;
      {
        node n3 = node(4, node::MAX_ID, true_ptr, false_ptr);
        node n2 = node(3, node::MAX_ID, n3.uid(), false_ptr);
        node n1 = node(1, node::MAX_ID, n3.uid(), n2.uid());
        node_writer writer(bdd_file);
        writer << n3 << n2 << n1;
      }

      node_file zdd_file;
      {
        node n4 = node(2, node::MAX_ID, true_ptr, true_ptr);
        node n3 = node(2, node::MAX_ID-1, false_ptr, true_ptr);
        node n2 = node(1, node::MAX_ID, n4.uid(), n4.uid());
        node n1 = node(0, node::MAX_ID, n3.uid(), n4.uid());
        node_writer writer(zdd_file);
        writer << n4 << n3 << n2 << n1;
      }

      it("returns empty file for a BDD false terminal", [&]() {
        label_file label_file_out = bdd_varprofile(terminal_F);

        label_stream<> out_labels(label_file_out);

        AssertThat(out_labels.can_pull(), Is().False());
      });

      it("returns empty file for a ZDD true terminal", [&]() {
        label_file label_file_out = zdd_varprofile(terminal_T);

        label_stream<> out_labels(label_file_out);

        AssertThat(out_labels.can_pull(), Is().False());
      });

      it("returns [42] for a ZDD with one node (label 42)", [&]() {
        label_file label_file_out = zdd_varprofile(x42);

        label_stream<> out_labels(label_file_out);

        AssertThat(out_labels.can_pull(), Is().True());
        AssertThat(out_labels.pull(), Is().EqualTo(42u));
        AssertThat(out_labels.can_pull(), Is().False());
      });
      
      it("returns [1,3,4] for a BDD with multiple nodes", [&]() {
        label_file label_file_out = bdd_varprofile(bdd_file);

        label_stream<> out_labels(label_file_out);

        AssertThat(out_labels.can_pull(), Is().True());
        AssertThat(out_labels.pull(), Is().EqualTo(1u));

        AssertThat(out_labels.can_pull(), Is().True());
        AssertThat(out_labels.pull(), Is().EqualTo(3u));

        AssertThat(out_labels.can_pull(), Is().True());
        AssertThat(out_labels.pull(), Is().EqualTo(4u));

        AssertThat(out_labels.can_pull(), Is().False());
      });

      it("returns [0,1,2] for a ZDD with multiple nodes", [&]() {
        label_file label_file_out = zdd_varprofile(zdd_file);

        label_stream<> out_labels(label_file_out);

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
