#include <filesystem>

go_bandit([]() {
  describe("adiar/file.h, adiar/file_stream.h, adiar/file_writer.h", [&]() {
    describe("adiar/file.h", [&]() {
      it("can construct fresh file<T> and make it read-only", [&]() {
        file<int> f;

        AssertThat(f.is_read_only(), Is().False());

        f.make_read_only();

        AssertThat(f.is_read_only(), Is().True());
      });

      it("can construct fresh __meta_file<T> and make it read-only", [&]() {
        __meta_file<int, 1> f;

        AssertThat(f.is_read_only(), Is().False());

        f.make_read_only();

        AssertThat(f.is_read_only(), Is().True());
      });
    });

    simple_file<int> test_file_simple;
    simple_file<int> test_file_simple_sorted;

    meta_file<int,1> test_file_meta_1;
    meta_file<int,2> test_file_meta_2;

    node_file node_test_file;

    node_t n2 = create_node(1,1, create_sink_ptr(false), create_sink_ptr(true));
    node_t n1 = create_node(1,0, create_sink_ptr(true), create_sink_ptr(false));
    node_t n0 = create_node(0,0, create_node_ptr(1,1), create_node_ptr(1,0));

    arc_file arc_test_file;

    arc_t node_arc_1 = { create_node_ptr(0,0), create_node_ptr(1,0) };
    arc_t sink_arc_1 = { flag(create_node_ptr(0,0)), create_sink_ptr(false) };

    arc_t sink_arc_2 = { create_node_ptr(1,0), create_sink_ptr(true) };
    arc_t sink_arc_3 = { flag(create_node_ptr(1,0)), create_sink_ptr(false) };

    describe("adiar/file_writer", [&]() {
      describe("simple_file_writer", [&]() {
        it("can hook into and write to test_file_simple", [&]() {
          AssertThat(test_file_simple.is_read_only(), Is().False());

          simple_file_writer<int> fw(test_file_simple);

          fw.unsafe_push(21);
          fw.unsafe_push(42);

          AssertThat(test_file_simple.is_read_only(), Is().False());
        });

        it("can sort after pushing out-of-order", [&]() {
          AssertThat(test_file_simple_sorted.is_read_only(), Is().False());

          simple_file_writer<int, std::less<int>> fw(test_file_simple_sorted);

          fw.unsafe_push(5);
          fw.unsafe_push(3);
          fw.unsafe_push(4);
          fw.unsafe_push(1);
          fw.unsafe_push(2);

          fw.sort();

          AssertThat(test_file_simple_sorted.is_read_only(), Is().False());
        });

        it("does not break when source file is destructed early [simple_file]", [&]() {
          simple_file<int>* f = new simple_file<int>();
          simple_file_writer<int> fw(*f);

          fw.unsafe_push(21);

          delete f;

          fw.unsafe_push(42);
        });
      });

      describe("meta_file_writer", [&]() {
        it("can hook into and write to test_file_meta_1", [&]() {
          AssertThat(test_file_meta_1.is_read_only(), Is().False());

          meta_file_writer<int,1> fw(test_file_meta_1);

          fw.unsafe_push(21);
          fw.unsafe_push(42);

          fw.unsafe_push(create_level_info(0,2u));

          AssertThat(test_file_meta_1.is_read_only(), Is().False());
        });


        it("can hook into and write to test_file_meta_2", [&]() {
          meta_file_writer<int,2> fw(test_file_meta_2);

          fw.unsafe_push(create_level_info(5,1u));
          fw.unsafe_push(create_level_info(4,1u));

          fw.unsafe_push(1); // Check idx argument is defaulted to 0
          fw.unsafe_push(2, 0);
          fw.unsafe_push(4, 1);
          fw.unsafe_push(5, 1);
          fw.unsafe_push(3, 0);

          AssertThat(test_file_meta_2.is_read_only(), Is().False());
        });

        it("does not break when source file is destructed early [simple_file]", [&]() {
          meta_file<int, 1>* f = new meta_file<int, 1>();
          meta_file_writer fw(*f);

          fw.unsafe_push(21);

          delete f;

          fw.unsafe_push(42);
          fw.unsafe_push(create_level_info(0,1u));
        });

        describe("node_writer", [&]() {
          it("can hook into and write to node_test_file", [&]() {
            AssertThat(node_test_file.is_read_only(), Is().False());

            node_writer nw(node_test_file);
            nw << n2 << n1 << n0;

            AssertThat(node_test_file.is_read_only(), Is().False());
          });

          it("can derive whether it is on canonical form [1]", [&]() {
            node_file nf;
            {
              node_writer nw(nf);
              nw << create_node(42, MAX_ID, create_sink_ptr(false), create_sink_ptr(true));
            }

            AssertThat(is_canonical(nf), Is().True());
          });

          it("can derive whether it is on canonical form [2]", [&]() {
            node_file nf;
            {
              node_writer nw(nf);
              nw << create_node(21, 42, create_sink_ptr(false), create_sink_ptr(true));
            }

            AssertThat(is_canonical(nf), Is().False());
          });

          it("can derive whether it is on canonical form [3]", [&]() {
            node_file nf;
            {
              node_writer nw(nf);
              nw << create_node(42, MAX_ID, create_sink_ptr(false), create_sink_ptr(true))
                 << create_node(42, MAX_ID-1, create_sink_ptr(true), create_sink_ptr(false))
                 << create_node(21, MAX_ID, create_node_ptr(42, MAX_ID), create_node_ptr(42, MAX_ID-1));
            }

            AssertThat(is_canonical(nf), Is().True());
          });

          it("can derive whether it is on canonical form [4]", [&]() {
            node_file nf;
            {
              node_writer nw(nf);
              nw << create_node(42, MAX_ID, create_sink_ptr(true), create_sink_ptr(false))
                 << create_node(42, MAX_ID-1, create_sink_ptr(false), create_sink_ptr(true))
                 << create_node(21, MAX_ID, create_node_ptr(42, MAX_ID), create_node_ptr(42, MAX_ID-1));
            }

            AssertThat(is_canonical(nf), Is().False());
          });

          it("can derive whether it is on canonical form [5]", [&]() {
            node_file nf;
            {
              node_writer nw(nf);
              nw << create_node(42, MAX_ID, create_sink_ptr(false), create_sink_ptr(true))
                 << create_node(42, MAX_ID-1, create_sink_ptr(true), create_sink_ptr(false))
                 << create_node(21, MAX_ID-1, create_node_ptr(42, MAX_ID), create_node_ptr(42, MAX_ID-1));
            }

            AssertThat(is_canonical(nf), Is().False());
          });

          it("can derive whether it is on canonical form [6]", [&]() {
            node_file nf;
            {
              node_writer nw(nf);
              nw << create_node(42, MAX_ID, create_sink_ptr(false), create_sink_ptr(true))
                 << create_node(42, MAX_ID-2, create_sink_ptr(true), create_sink_ptr(false))
                 << create_node(21, MAX_ID, create_node_ptr(42, MAX_ID), create_node_ptr(42, MAX_ID-2));
            }

            AssertThat(is_canonical(nf), Is().False());
          });
        });

        describe("arc_writer", [&]() {
          arc_writer aw(arc_test_file);

          it("can hook into arc_test_file and write node arcs", [&]() {
            AssertThat(arc_test_file.is_read_only(), Is().False());

            aw.unsafe_push_node(node_arc_1);
          });

          it("can hook into arc_test_file and write level_info", [&]() {
            aw.unsafe_push(create_level_info(0,1u));
            aw.unsafe_push(create_level_info(1,1u));
          });

          it("can hook into arc_test_file and  write sink arcs out of order", [&]() {
            aw.unsafe_push_sink(sink_arc_3);
            aw.unsafe_push_sink(sink_arc_1);
            aw.unsafe_push_sink(sink_arc_2);
          });
        });
      });
    });

    describe("adiar/file_stream.h", [&]() {
      describe("simple_file_stream", [&]() {
        it("locks the file to be read-only on attachment", [&]() {
          AssertThat(test_file_simple.is_read_only(), Is().False());

          file_stream<int, false> fs(test_file_simple);

          AssertThat(test_file_simple.is_read_only(), Is().True());
        });

        it("can read test_file_simple [forwards]", [&]() {
          file_stream<int, false> fs(test_file_simple);

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(21));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(42));
          AssertThat(fs.can_pull(), Is().False());
        });

        it("can peek test_file_simple [forwards]", [&]() {
          file_stream<int, false> fs(test_file_simple);

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.peek(), Is().EqualTo(21));
          AssertThat(fs.pull(), Is().EqualTo(21));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.peek(), Is().EqualTo(42));
          AssertThat(fs.pull(), Is().EqualTo(42));
          AssertThat(fs.can_pull(), Is().False());
        });

        it("can read test_file_simple_sorted [reverse]", [&]() {
          file_stream<int, true> fs(test_file_simple_sorted);

          // This also tests, whether test_file_simple_sorted actually
          // ended up being sorted.
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(5));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(4));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(3));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(2));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(1));
          AssertThat(fs.can_pull(), Is().False());
        });

        it("does not break when source simple_file is destructed early", [&]() {
          simple_file<int>* f = new simple_file<int>();

          { // Garbage collect the writer early, releasing it's reference counter
            simple_file_writer<int> fw(*f);
            fw.unsafe_push(21);
          }

          file_stream<int, false> fs(*f);

          delete f;

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(21));
        });
      });

      describe("meta_file_stream", [&]() {
        it("locks the file to be read-only on attachment", [&]() {
          AssertThat(test_file_meta_1.is_read_only(), Is().False());
          meta_file_stream<int, 1, 0, false> fs1(test_file_meta_1);

          AssertThat(test_file_meta_1.is_read_only(), Is().True());

          AssertThat(test_file_meta_2.is_read_only(), Is().False());
          meta_file_stream<int, 2, 0, false> fs2(test_file_meta_2);

          AssertThat(test_file_meta_2.is_read_only(), Is().True());
        });

        it("can read test_file_meta_1 [forwards]", [&]() {
          meta_file_stream<int, 1, 0, false> fs(test_file_meta_1);

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(21));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(42));
          AssertThat(fs.can_pull(), Is().False());
        });

        it("can peek test_file_meta_1 [forwards]", [&]() {
          meta_file_stream<int, 1, 0, false> fs(test_file_meta_1);

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.peek(), Is().EqualTo(21));
          AssertThat(fs.pull(), Is().EqualTo(21));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.peek(), Is().EqualTo(42));
          AssertThat(fs.pull(), Is().EqualTo(42));
          AssertThat(fs.can_pull(), Is().False());
        });

        it("can read test_file_meta_1 [reverse]", [&]() {
          meta_file_stream<int, 1, 0, true> fs(test_file_meta_1);

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(42));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(21));
          AssertThat(fs.can_pull(), Is().False());
        });

        it("can peek test_file_1 [reverse]", [&]() {
          meta_file_stream<int, 1, 0, true> fs(test_file_meta_1);

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.peek(), Is().EqualTo(42));
          AssertThat(fs.pull(), Is().EqualTo(42));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.peek(), Is().EqualTo(21));
          AssertThat(fs.pull(), Is().EqualTo(21));
          AssertThat(fs.can_pull(), Is().False());
        });

        it("can read test_file_meta_2 [forwards]", [&]() {
          meta_file_stream<int, 2, 0, false> fs1(test_file_meta_2);

          AssertThat(fs1.can_pull(), Is().True());
          AssertThat(fs1.pull(), Is().EqualTo(1));
          AssertThat(fs1.can_pull(), Is().True());
          AssertThat(fs1.pull(), Is().EqualTo(2));
          AssertThat(fs1.can_pull(), Is().True());
          AssertThat(fs1.pull(), Is().EqualTo(3));
          AssertThat(fs1.can_pull(), Is().False());

          meta_file_stream<int, 2, 1, false> fs2(test_file_meta_2);

          AssertThat(fs2.can_pull(), Is().True());
          AssertThat(fs2.pull(), Is().EqualTo(4));
          AssertThat(fs2.can_pull(), Is().True());
          AssertThat(fs2.pull(), Is().EqualTo(5));
          AssertThat(fs2.can_pull(), Is().False());
        });

        it("can peek test_file_meta_2 [forwards]", [&]() {
          meta_file_stream<int, 2, 0, false> fs1(test_file_meta_2);

          AssertThat(fs1.can_pull(), Is().True());
          AssertThat(fs1.peek(), Is().EqualTo(1));
          AssertThat(fs1.pull(), Is().EqualTo(1));
          AssertThat(fs1.can_pull(), Is().True());
          AssertThat(fs1.peek(), Is().EqualTo(2));
          AssertThat(fs1.pull(), Is().EqualTo(2));
          AssertThat(fs1.can_pull(), Is().True());
          AssertThat(fs1.peek(), Is().EqualTo(3));
          AssertThat(fs1.pull(), Is().EqualTo(3));
          AssertThat(fs1.can_pull(), Is().False());

          meta_file_stream<int, 2, 1, false> fs2(test_file_meta_2);

          AssertThat(fs2.can_pull(), Is().True());
          AssertThat(fs2.peek(), Is().EqualTo(4));
          AssertThat(fs2.pull(), Is().EqualTo(4));
          AssertThat(fs2.can_pull(), Is().True());
          AssertThat(fs2.peek(), Is().EqualTo(5));
          AssertThat(fs2.pull(), Is().EqualTo(5));
          AssertThat(fs2.can_pull(), Is().False());
        });

        it("can read test_file_1 forwards via fresh proxy object", [&]() {
          meta_file<int, 1> proxy(test_file_meta_1);
          meta_file_stream<int, 1, 0, false> fs(proxy);

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(21));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(42));
          AssertThat(fs.can_pull(), Is().False());
        });

        it("does not break when source meta_file is destructed early", [&]() {
          meta_file<int,1>* f = new meta_file<int, 1>();

          { // Garbage collect the writer early, releasing it's reference counter
            meta_file_writer<int,1> fw(*f);
            fw.unsafe_push(21, 0);
          }

          meta_file_stream<int, 1, 0, false> fs(*f);

          delete f;

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(21));
        });

        describe("level_info_stream", [&]() {
          it("can read level_info stream of test_file_meta_1", [&]() {
            level_info_stream ms(test_file_meta_1);

            AssertThat(ms.can_pull(), Is().True());
            AssertThat(ms.pull(), Is().EqualTo(create_level_info(0,2u)));
            AssertThat(ms.can_pull(), Is().False());
          });

          it("can read level_info stream of node_test_file", [&]() {
            level_info_stream ms(node_test_file);

            AssertThat(ms.can_pull(), Is().True());
            AssertThat(ms.pull(), Is().EqualTo(create_level_info(0,1u)));
            AssertThat(ms.can_pull(), Is().True());
            AssertThat(ms.pull(), Is().EqualTo(create_level_info(1,2u)));
            AssertThat(ms.can_pull(), Is().False());
          });
        });

        describe("node_stream", [&]() {
          it("can read node stream of node_test_file", [&]() {
            node_stream<> ns(node_test_file);

            AssertThat(ns.can_pull(), Is().True());
            AssertThat(ns.pull(), Is().EqualTo(n0));
            AssertThat(ns.can_pull(), Is().True());
            AssertThat(ns.pull(), Is().EqualTo(n1));
            AssertThat(ns.can_pull(), Is().True());
            AssertThat(ns.pull(), Is().EqualTo(n2));
            AssertThat(ns.can_pull(), Is().False());
          });

          it("can read negated node stream of node_test_file", [&]() {
            node_stream<> ns(node_test_file, true);

            AssertThat(ns.can_pull(), Is().True());
            AssertThat(ns.pull(), Is().EqualTo(!n0));
            AssertThat(ns.can_pull(), Is().True());
            AssertThat(ns.pull(), Is().EqualTo(!n1));
            AssertThat(ns.can_pull(), Is().True());
            AssertThat(ns.pull(), Is().EqualTo(!n2));
            AssertThat(ns.can_pull(), Is().False());
          });
        });

        describe("arc_streams", [&]() {
          it("can read node arcs of arc_test_file", [&]() {
            node_arc_stream<> as(arc_test_file);

            AssertThat(as.can_pull(), Is().True());
            AssertThat(as.pull(), Is().EqualTo(node_arc_1));
            AssertThat(as.can_pull(), Is().False());
          });

          it("can read sink arcs of arc_test_file", [&]() {
            sink_arc_stream<> as(arc_test_file);

            // The stream has been sorted 1, 2, 3 yet the
            // sink_arc_stream is per default in reverse, so we will
            // expect 3, 2, 1.
            AssertThat(as.can_pull(), Is().True());
            AssertThat(as.pull(), Is().EqualTo(sink_arc_3));
            AssertThat(as.can_pull(), Is().True());
            AssertThat(as.pull(), Is().EqualTo(sink_arc_2));
            AssertThat(as.can_pull(), Is().True());
            AssertThat(as.pull(), Is().EqualTo(sink_arc_1));
            AssertThat(as.can_pull(), Is().False());
          });
        });
      });
    });

    describe("shared_file", [&]() {
      describe("simple_file", [&]() {
        it("can return a written to simple_file, that then can be read", [&]() {
          auto t = []() {
            simple_file<int> f;

            simple_file_writer<int> fw(f);
            fw.unsafe_push(42);
            fw.unsafe_push(7);

            return f;
          };

          simple_file<int> f = t();
          file_stream<int> fs(f);

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(42));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(7));
          AssertThat(fs.can_pull(), Is().False());
        });

        it("can construct a new named simple_file", [&]() {
          simple_file<int> file("simple_file_test.adiar");

          AssertThat(std::filesystem::exists("simple_file_test.adiar"), Is().True());

          {
            simple_file_writer<int> fw(file);
            fw.push(1);
            fw.push(2);
            fw.push(3);
          }
          AssertThat(file.is_read_only(), Is().False());

          {
            file_stream<int, false> fs(file);
            AssertThat(file.is_read_only(), Is().True());
            AssertThat(fs.pull(), Is().EqualTo(1));
            AssertThat(fs.pull(), Is().EqualTo(2));
            AssertThat(fs.pull(), Is().EqualTo(3));
            AssertThat(fs.can_pull(), Is().False());
          }
          AssertThat(file.is_read_only(), Is().True());
        });

        it("should have two temporary simple_files be two different files", [&]() {
          simple_file<int> file1;
          simple_file<int> file2;

          AssertThat(file1._file_ptr, Is().Not().EqualTo(file2._file_ptr));
        });

        it("can construct a copy of a simple_file", [&]() {
          simple_file<int> file1;

          { // Garbage collect the writer to detach it before the reader
            simple_file_writer<int> fw(file1);
            fw.push(42);
          }

          simple_file<int> file2(file1);

          file_stream<int, false> fs(file2);
          AssertThat(fs.pull(), Is().EqualTo(42));
          AssertThat(fs.can_pull(), Is().False());
        });

        it("can construct a prior named simple_file (i.e. reopen a stored file)", [&]() {
          AssertThat(std::filesystem::exists("simple_file_test.adiar"), Is().True());
          simple_file<int> file("simple_file_test.adiar");
          AssertThat(file.is_read_only(), Is().True());

          file_stream<int, false> fs(file);
          AssertThat(file.is_read_only(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(1));
          AssertThat(fs.pull(), Is().EqualTo(2));
          AssertThat(fs.pull(), Is().EqualTo(3));
          AssertThat(fs.can_pull(), Is().False());
        });

        it("can compute sizes of test_file_simple", [&]() {
          AssertThat(test_file_simple.size(), Is().EqualTo(2u));
          AssertThat(test_file_simple.file_size(), Is().EqualTo(2u * sizeof(int)));
        });

        it("can sort a yet unread simple_file", [&]() {
          simple_file<int> file;

          { // Garbage collect the writer to detach it before the reader
            simple_file_writer<int> fw(file);
            fw.push(42);
            fw.push(7);
            fw.push(21);
          }

          sort(file, std::less<>());

          file_stream<int, false> fs(file);

          AssertThat(fs.pull(), Is().EqualTo(7));
          AssertThat(fs.pull(), Is().EqualTo(21));
          AssertThat(fs.pull(), Is().EqualTo(42));
          AssertThat(fs.can_pull(), Is().False());
        });
      });

      describe("meta_file", [&]() {
        it("can return a written to meta_file, that then can be read", [&]() {
          auto t = []() {
            meta_file<int, 1> f;

            meta_file_writer<int, 1> fw(f);
            fw.unsafe_push(42);
            fw.unsafe_push(7);

            return f;
          };

          meta_file<int, 1> f = t();
          meta_file_stream<int, 1, 0, false> fs(f);

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(42));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(7));
          AssertThat(fs.can_pull(), Is().False());
        });

        it("should have two temporary meta_files be two different files", [&]() {
          meta_file<int, 1> file1;
          meta_file<int, 1> file2;

          AssertThat(file1._file_ptr, Is().Not().EqualTo(file2._file_ptr));
        });

        it("can construct a copy of a meta_file", [&]() {
          meta_file<int,1> file1;

          { // Garbage collect the writer to detach it before the reader
            meta_file_writer<int, 1> fw(file1);
            fw.unsafe_push(42);
          }

          meta_file<int,1> file2(file1);

          meta_file_stream<int, 1, 0, false> fs(file2);
          AssertThat(fs.pull(), Is().EqualTo(42));
          AssertThat(fs.can_pull(), Is().False());
        });

        it("can compute sizes of test_file_meta_1", [&]() {
          AssertThat(test_file_meta_1.size(), Is().EqualTo(2u));
          AssertThat(test_file_meta_1.file_size(), Is().EqualTo(2u * sizeof(int) + 1u * sizeof(level_info_t)));
        });

        it("can compute size of test_file_meta_2", [&]() {
          test_file_meta_2.make_read_only();
          AssertThat(test_file_meta_2.size(), Is().EqualTo(5u));
          AssertThat(test_file_meta_2.file_size(), Is().EqualTo(5u * sizeof(int) + 2u * sizeof(level_info_t)));
        });

        describe("node_file", [&]() {
          node_file x0;

          {
            node_writer nw_0(x0);
            nw_0 << create_node(0,MAX_ID,
                                create_sink_ptr(false),
                                create_sink_ptr(true));
          }

          node_file x0_and_x1;

          {
            node_writer nw_01(x0_and_x1);

            nw_01 << create_node(1, MAX_ID,
                                 create_sink_ptr(false),
                                 create_sink_ptr(true));

            nw_01 << create_node(0, MAX_ID,
                                 create_sink_ptr(false),
                                 create_node_ptr(1, MAX_ID));
          }

          node_file sink_T;

          {
            node_writer nw_T(sink_T);
            nw_T << create_sink(true);
          }

          node_file sink_F;

          {
            node_writer nw_F(sink_F);
            nw_F << create_sink(false);
          }

          describe("size computation", [&]() {
            it("can compute size of node_test_file", [&]() {
              AssertThat(node_test_file.size(), Is().EqualTo(3u));
              AssertThat(node_test_file.meta_size(), Is().EqualTo(2u));
              AssertThat(node_test_file.file_size(), Is().EqualTo(3u * sizeof(node_t) + 2u * sizeof(level_info_t)));
            });

            it("can compute size of x0", [&]() {
              AssertThat(x0.size(), Is().EqualTo(1u));
              AssertThat(x0.meta_size(), Is().EqualTo(1u));
              AssertThat(x0.file_size(), Is().EqualTo(1u * sizeof(node_t) + 1u * sizeof(level_info_t)));
            });

            it("can compute size of x0 & x1", [&]() {
              AssertThat(x0_and_x1.size(), Is().EqualTo(2u));
              AssertThat(x0_and_x1.meta_size(), Is().EqualTo(2u));
              AssertThat(x0_and_x1.file_size(), Is().EqualTo(2u * sizeof(node_t) + 2u * sizeof(level_info_t)));
            });

            it("can compute size of sink_T", [&]() {
              AssertThat(sink_T.size(), Is().EqualTo(1u));
              AssertThat(sink_T.meta_size(), Is().EqualTo(0u));
              AssertThat(sink_T.file_size(), Is().EqualTo(1u * sizeof(node_t) + 0u * sizeof(level_info_t)));
            });
          });

          describe("is_sink predicate", [&]() {
            it("should reject x0 as a sink file", [&]() {
              AssertThat(is_sink(x0, is_true), Is().False());
              AssertThat(is_sink(x0, is_false), Is().False());
              AssertThat(is_sink(x0), Is().False());
            });

            it("should reject x0 & x1 as a sink file", [&]() {
              AssertThat(is_sink(x0_and_x1, is_true), Is().False());
              AssertThat(is_sink(x0_and_x1, is_false), Is().False());
              AssertThat(is_sink(x0_and_x1), Is().False());
            });

            it("should recognise a true sink", [&]() {
              AssertThat(is_sink(sink_T, is_true), Is().True());
            });

            it("should recognise a false sink", [&]() {
              AssertThat(is_sink(sink_F, is_false), Is().True());
            });

            it("should not recognise sink file as the other sink", [&]() {
              AssertThat(is_sink(sink_T, is_false), Is().False());
              AssertThat(is_sink(sink_F, is_true), Is().False());
            });

            it("should have any sink as default", [&]() {
              AssertThat(is_sink(sink_T), Is().True());
              AssertThat(is_sink(sink_F), Is().True());
            });
          });

          describe("min_label and max_label", [&]() {
            it("should extract labels from x0", [&]() {
              AssertThat(min_label(x0), Is().EqualTo(0u));
              AssertThat(max_label(x0), Is().EqualTo(0u));
            });

            it("should extract labels from x0_and_x1", [&]() {
              AssertThat(min_label(x0_and_x1), Is().EqualTo(0u));
              AssertThat(max_label(x0_and_x1), Is().EqualTo(1u));
            });

            it("should extract labels from node_test_file", [&]() {
              AssertThat(min_label(node_test_file), Is().EqualTo(0u));
              AssertThat(max_label(node_test_file), Is().EqualTo(1u));
            });
          });
        });
      });
    });
  });
 });
