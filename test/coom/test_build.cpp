#include <tpie/tpie.h>
#include <coom/build.cpp>

using namespace coom;

go_bandit([]() {
    describe("COOM: Build", [&]() {
        ptr_t sink_T = create_sink_ptr(true);
        ptr_t sink_F = create_sink_ptr(false);

        describe("build_sink", [&]() {
            it("can create true sink only", [&]() {
                tpie::file_stream<node_t> out_nodes;
                out_nodes.open();

                tpie::file_stream<meta_t> out_meta;
                out_meta.open();

                build_sink(true, out_nodes, out_meta);

                out_nodes.seek(0);

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_sink(true)));
                AssertThat(out_nodes.can_read(), Is().False());

                AssertThat(out_meta.size(), Is().EqualTo(0u));
              });

            it("can create false sink only", [&]() {
                tpie::file_stream<node_t> out_nodes;
                out_nodes.open();

                tpie::file_stream<meta_t> out_meta;
                out_meta.open();

                build_sink(false, out_nodes, out_meta);

                out_nodes.seek(0);

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_sink(false)));
                AssertThat(out_nodes.can_read(), Is().False());

                AssertThat(out_meta.size(), Is().EqualTo(0u));
              });
          });

        describe("build_x", [&]() {
            it("can create x0", [&]() {
                tpie::file_stream<node_t> out_nodes;
                out_nodes.open();

                tpie::file_stream<meta_t> out_meta;
                out_meta.open();

                build_x(0, out_nodes, out_meta);

                out_nodes.seek(0);

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(0, 0, sink_F, sink_T)));
                AssertThat(out_nodes.can_read(), Is().False());

                out_meta.seek(0);

                AssertThat(out_meta.can_read(), Is().True());
                AssertThat(out_meta.read(), Is().EqualTo(meta_t { 0 }));
                AssertThat(out_meta.can_read(), Is().False());
              });

            it("can create x42", [&]() {
                tpie::file_stream<node_t> out_nodes;
                out_nodes.open();

                tpie::file_stream<meta_t> out_meta;
                out_meta.open();

                build_x(42, out_nodes, out_meta);

                out_nodes.seek(0);

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(42, 0, sink_F, sink_T)));
                AssertThat(out_nodes.can_read(), Is().False());

                out_meta.seek(0);

                AssertThat(out_meta.can_read(), Is().True());
                AssertThat(out_meta.read(), Is().EqualTo(meta_t { 42 }));
                AssertThat(out_meta.can_read(), Is().False());
              });
          });

        describe("build_not_x", [&]() {
            it("can create !x1", [&]() {
                tpie::file_stream<node_t> out_nodes;
                out_nodes.open();

                tpie::file_stream<meta_t> out_meta;
                out_meta.open();

                build_not_x(1, out_nodes, out_meta);

                out_nodes.seek(0);

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(1, 0, sink_T, sink_F)));
                AssertThat(out_nodes.can_read(), Is().False());

                out_meta.seek(0);

                AssertThat(out_meta.can_read(), Is().True());
                AssertThat(out_meta.read(), Is().EqualTo(meta_t { 1 }));
                AssertThat(out_meta.can_read(), Is().False());
              });

            it("can create !x3", [&]() {
                tpie::file_stream<node_t> out_nodes;
                out_nodes.open();

                tpie::file_stream<meta_t> out_meta;
                out_meta.open();

                build_not_x(3, out_nodes, out_meta);

                out_nodes.seek(0);

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(3, 0, sink_T, sink_F)));
                AssertThat(out_nodes.can_read(), Is().False());

                out_meta.seek(0);

                AssertThat(out_meta.can_read(), Is().True());
                AssertThat(out_meta.read(), Is().EqualTo(meta_t { 3 }));
                AssertThat(out_meta.can_read(), Is().False());
              });
          });

        describe("build_and", [&]() {
            it("can create {x1, x2, x5}", [&]() {
                tpie::file_stream<uint64_t> in_labels;
                in_labels.open();

                in_labels.write(1);
                in_labels.write(2);
                in_labels.write(5);

                tpie::file_stream<node_t> out_nodes;
                out_nodes.open();

                tpie::file_stream<meta_t> out_meta;
                out_meta.open();

                build_and(in_labels, out_nodes, out_meta);

                out_nodes.seek(0);

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(5, 0,
                                                                      sink_F,
                                                                      sink_T)));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(2, 0,
                                                                      sink_F,
                                                                      create_node_ptr(5,0))));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(1, 0,
                                                                      sink_F,
                                                                      create_node_ptr(2,0))));

                AssertThat(out_nodes.can_read(), Is().False());

                out_meta.seek(0);

                AssertThat(out_meta.can_read(), Is().True());
                AssertThat(out_meta.read(), Is().EqualTo(meta_t { 5 }));

                AssertThat(out_meta.can_read(), Is().True());
                AssertThat(out_meta.read(), Is().EqualTo(meta_t { 2 }));

                AssertThat(out_meta.can_read(), Is().True());
                AssertThat(out_meta.read(), Is().EqualTo(meta_t { 1 }));

                AssertThat(out_meta.can_read(), Is().False());
              });

            it("can create {} as trivially true", [&]() {
                tpie::file_stream<uint64_t> in_labels;
                in_labels.open();

                tpie::file_stream<node_t> out_nodes;
                out_nodes.open();

                tpie::file_stream<meta_t> out_meta;
                out_meta.open();

                build_and(in_labels, out_nodes, out_meta);

                out_nodes.seek(0);

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_sink(true)));
                AssertThat(out_nodes.can_read(), Is().False());

                AssertThat(out_meta.size(), Is().EqualTo(0u));
              });
          });

        describe("build_or", [&]() {
            it("can create {x1, x2, x5}", [&]() {
                tpie::file_stream<uint64_t> in_labels;
                in_labels.open();

                in_labels.write(1);
                in_labels.write(2);
                in_labels.write(5);

                tpie::file_stream<node_t> out_nodes;
                out_nodes.open();

                tpie::file_stream<meta_t> out_meta;
                out_meta.open();

                build_or(in_labels, out_nodes, out_meta);

                out_nodes.seek(0);

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(5, 0,
                                                                      sink_F,
                                                                      sink_T)));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(2, 0,
                                                                      create_node_ptr(5,0),
                                                                      sink_T)));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(1, 0,
                                                                      create_node_ptr(2,0),
                                                                      sink_T)));

                AssertThat(out_nodes.can_read(), Is().False());

                out_meta.seek(0);

                AssertThat(out_meta.can_read(), Is().True());
                AssertThat(out_meta.read(), Is().EqualTo(meta_t { 5 }));

                AssertThat(out_meta.can_read(), Is().True());
                AssertThat(out_meta.read(), Is().EqualTo(meta_t { 2 }));

                AssertThat(out_meta.can_read(), Is().True());
                AssertThat(out_meta.read(), Is().EqualTo(meta_t { 1 }));

                AssertThat(out_meta.can_read(), Is().False());
              });

            it("can create {} as trivially false", [&]() {
                tpie::file_stream<uint64_t> in_labels;
                in_labels.open();

                tpie::file_stream<node_t> out_nodes;
                out_nodes.open();

                tpie::file_stream<meta_t> out_meta;
                out_meta.open();

                build_or(in_labels, out_nodes, out_meta);

                out_nodes.seek(0);

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_sink(false)));
                AssertThat(out_nodes.can_read(), Is().False());

                AssertThat(out_meta.size(), Is().EqualTo(0u));
              });
          });

        describe("build_counter", [&]() {
            it("creates counting to 3 in [0,8]", [&]() {
                tpie::file_stream<node_t> out_nodes;
                out_nodes.open();

                tpie::file_stream<meta_t> out_meta;
                out_meta.open();

                build_counter(0, 8, 3, out_nodes, out_meta);

                out_nodes.seek(0);

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(8, 3,
                                                                      sink_T,
                                                                      sink_F)));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(8, 2,
                                                                      sink_F,
                                                                      sink_T)));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(7, 3,
                                                                      create_node_ptr(8,3),
                                                                      sink_F)));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(7, 2,
                                                                      create_node_ptr(8,2),
                                                                      create_node_ptr(8,3))));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(7, 1,
                                                                      sink_F,
                                                                      create_node_ptr(8,2))));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(6, 3,
                                                                      create_node_ptr(7,3),
                                                                      sink_F)));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(6, 2,
                                                                      create_node_ptr(7,2),
                                                                      create_node_ptr(7,3))));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(6, 1,
                                                                      create_node_ptr(7,1),
                                                                      create_node_ptr(7,2))));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(6, 0,
                                                                      sink_F,
                                                                      create_node_ptr(7,1))));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(5, 3,
                                                                      create_node_ptr(6,3),
                                                                      sink_F)));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(5, 2,
                                                                      create_node_ptr(6,2),
                                                                      create_node_ptr(6,3))));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(5, 1,
                                                                      create_node_ptr(6,1),
                                                                      create_node_ptr(6,2))));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(5, 0,
                                                                      create_node_ptr(6,0),
                                                                      create_node_ptr(6,1))));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(4, 3,
                                                                      create_node_ptr(5,3),
                                                                      sink_F)));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(4, 2,
                                                                      create_node_ptr(5,2),
                                                                      create_node_ptr(5,3))));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(4, 1,
                                                                      create_node_ptr(5,1),
                                                                      create_node_ptr(5,2))));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(4, 0,
                                                                      create_node_ptr(5,0),
                                                                      create_node_ptr(5,1))));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(3, 3,
                                                                      create_node_ptr(4,3),
                                                                      sink_F)));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(3, 2,
                                                                      create_node_ptr(4,2),
                                                                      create_node_ptr(4,3))));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(3, 1,
                                                                      create_node_ptr(4,1),
                                                                      create_node_ptr(4,2))));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(3, 0,
                                                                      create_node_ptr(4,0),
                                                                      create_node_ptr(4,1))));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(2, 2,
                                                                      create_node_ptr(3,2),
                                                                      create_node_ptr(3,3))));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(2, 1,
                                                                      create_node_ptr(3,1),
                                                                      create_node_ptr(3,2))));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(2, 0,
                                                                      create_node_ptr(3,0),
                                                                      create_node_ptr(3,1))));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(1, 1,
                                                                      create_node_ptr(2,1),
                                                                      create_node_ptr(2,2))));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(1, 0,
                                                                      create_node_ptr(2,0),
                                                                      create_node_ptr(2,1))));

                AssertThat(out_nodes.can_read(), Is().True());
                AssertThat(out_nodes.read(), Is().EqualTo(create_node(0, 0,
                                                                      create_node_ptr(1,0),
                                                                      create_node_ptr(1,1))));

                AssertThat(out_nodes.can_read(), Is().False());

                out_meta.seek(0);

                AssertThat(out_meta.can_read(), Is().True());
                AssertThat(out_meta.read(), Is().EqualTo(meta_t { 8 }));

                AssertThat(out_meta.can_read(), Is().True());
                AssertThat(out_meta.read(), Is().EqualTo(meta_t { 7 }));

                AssertThat(out_meta.can_read(), Is().True());
                AssertThat(out_meta.read(), Is().EqualTo(meta_t { 6 }));

                AssertThat(out_meta.can_read(), Is().True());
                AssertThat(out_meta.read(), Is().EqualTo(meta_t { 5 }));

                AssertThat(out_meta.can_read(), Is().True());
                AssertThat(out_meta.read(), Is().EqualTo(meta_t { 4 }));

                AssertThat(out_meta.can_read(), Is().True());
                AssertThat(out_meta.read(), Is().EqualTo(meta_t { 3 }));

                AssertThat(out_meta.can_read(), Is().True());
                AssertThat(out_meta.read(), Is().EqualTo(meta_t { 2 }));

                AssertThat(out_meta.can_read(), Is().True());
                AssertThat(out_meta.read(), Is().EqualTo(meta_t { 1 }));

                AssertThat(out_meta.can_read(), Is().True());
                AssertThat(out_meta.read(), Is().EqualTo(meta_t { 0 }));

                AssertThat(out_meta.can_read(), Is().False());
              });
          });
      });
  });
