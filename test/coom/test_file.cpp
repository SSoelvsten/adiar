go_bandit([]() {
    describe("COOM: File", [&]() {
        file<int,1> test_file_1;
        file<int,2> test_file_2;

        node_file node_test_file;

        node_t n2 = create_node(1,1, create_sink_ptr(false), create_sink_ptr(true));
        node_t n1 = create_node(1,0, create_sink_ptr(true), create_sink_ptr(false));
        node_t n0 = create_node(0,0, create_node_ptr(1,1), create_node_ptr(1,0));

        describe("file_writer", [&]() {
            it("can hook into and write to test_file_1", [&]() {
              file_writer<int,1> fw(test_file_1);

              fw.unsafe_push(meta_t { 0 });

              fw.unsafe_push(21);
              fw.unsafe_push(42);
            });

            it("can hook into and write to test_file_2", [&]() {
              file_writer<int,2> fw(test_file_2);

              fw.unsafe_push(meta_t { 2 });
              fw.unsafe_push(meta_t { 1 });

              fw.unsafe_push(1, 0);
              fw.unsafe_push(2, 0);
              fw.unsafe_push(4, 1);
              fw.unsafe_push(5, 1);
              fw.unsafe_push(3, 0);
            });

            it("can hook into and write to node_test_file", [&]() {
              node_writer nw(node_test_file);
              nw << n2 << n1 << n0;
            });

            it("does not break when source file is destructed early", [&]() {
              file<int,1>* f = new file<int, 1>();
              file_writer<int,1> fw(*f);

              fw.unsafe_push(meta_t { 0 });

              fw.unsafe_push(21);

              delete f;

              fw.unsafe_push(42);
            });
          });

        describe("file", [&]() {
              it("can construct a new temporary file", [&]() {
                file<int,1> file;
              });

              it("can construct a new named file [FILES = 1]", [&]() {
                file<int,1> file("coom_file_test.coom");
              });

              it("can construct a new named file [FILES = 2]", [&]() {
                file<int,2> f("coom_file_test_2.coom");
              });

              it("can construct a copy of a file", [&]() {
                file<int,1> file1;
                file<int,1> file2(file1);

                AssertThat(file2._meta_file, Is().EqualTo(file1._meta_file));
                AssertThat(file2._files[0], Is().EqualTo(file1._files[0]));
              });

              it("can construct a prior named file [FILES = 1]", [&]() {
                file<int,1> file("coom_file_test.coom");
              });

              it("can check for equality of files", [&]() {
                file<int,1> file_1;
                file<int,1> file_2;

                AssertThat(file_1 == file_2, Is().False());
                AssertThat(file_1 != file_2, Is().True());

                file<int,1> file_1_copy(file_1);
                AssertThat(file_1 == file_1_copy, Is().True());
                AssertThat(file_1 != file_1_copy, Is().False());
              });

              it("can compute sizes of test_file_1", [&]() {
                AssertThat(test_file_1.size(), Is().EqualTo(2u));
                AssertThat(test_file_1.meta_size(), Is().EqualTo(1u));
                AssertThat(test_file_1.file_size(), Is().EqualTo(2u * 4u + 1u * 8u));
              });

              it("can compute size of test_file_2", [&]() {
                AssertThat(test_file_2.size(), Is().EqualTo(5u));
                AssertThat(test_file_2.meta_size(), Is().EqualTo(2u));
                AssertThat(test_file_2.file_size(), Is().EqualTo(5u * 4u + 2u * 8u));
              });


            describe("node_file", [&]() {
                it("can compute size of node_test_file", [&]() {
                    AssertThat(node_test_file.size(), Is().EqualTo(3u));
                    AssertThat(node_test_file.meta_size(), Is().EqualTo(2u));
                    AssertThat(node_test_file.file_size(), Is().EqualTo(3u * 24u + 2u * 8u));
                });

                node_file x0;

                node_writer nw_0(x0);
                nw_0 << create_node(0,MAX_ID,
                                    create_sink_ptr(false),
                                    create_sink_ptr(true));

                node_file x0_and_x1;

                node_writer nw_01(x0_and_x1);

                nw_01 << create_node(1, MAX_ID,
                                     create_sink_ptr(false),
                                     create_sink_ptr(true));

                nw_01 << create_node(0, MAX_ID,
                                     create_sink_ptr(false),
                                     create_node_ptr(1, MAX_ID));

                node_file sink_T;
                node_writer nw_T(sink_T);
                nw_T << create_sink(true);

                node_file sink_F;
                node_writer nw_F(sink_F);
                nw_F << create_sink(false);

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

                    it("should not recognise sink file as the other sink [1]", [&]() {
                        AssertThat(is_sink(sink_T, is_false), Is().False());
                        AssertThat(is_sink(sink_F, is_true), Is().False());
                      });

                    it("should not recognise sink file as the other sink [2]", [&]() {

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

                    it("should extract labels from x0", [&]() {
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

        describe("file_stream", [&]() {
            it("can read test_file_1 [forwards]", [&]() {
              file_stream<int, false> fs(test_file_1._files[0]);

              AssertThat(fs.can_pull(), Is().True());
              AssertThat(fs.pull(), Is().EqualTo(21));
              AssertThat(fs.can_pull(), Is().True());
              AssertThat(fs.pull(), Is().EqualTo(42));
              AssertThat(fs.can_pull(), Is().False());
            });

            it("can peek test_file_1 [forwards]", [&]() {
              file_stream<int, false> fs(test_file_1._files[0]);

              AssertThat(fs.can_pull(), Is().True());
              AssertThat(fs.peek(), Is().EqualTo(21));
              AssertThat(fs.pull(), Is().EqualTo(21));
              AssertThat(fs.can_pull(), Is().True());
              AssertThat(fs.peek(), Is().EqualTo(42));
              AssertThat(fs.pull(), Is().EqualTo(42));
              AssertThat(fs.can_pull(), Is().False());
            });

            it("can read test_file_1 [reverse]", [&]() {
              file_stream<int, true> fs(test_file_1._files[0]);

              AssertThat(fs.can_pull(), Is().True());
              AssertThat(fs.pull(), Is().EqualTo(42));
              AssertThat(fs.can_pull(), Is().True());
              AssertThat(fs.pull(), Is().EqualTo(21));
              AssertThat(fs.can_pull(), Is().False());
            });

            it("can peek test_file_1 [reverse]", [&]() {
              file_stream<int, true> fs(test_file_1._files[0]);

              AssertThat(fs.can_pull(), Is().True());
              AssertThat(fs.peek(), Is().EqualTo(42));
              AssertThat(fs.pull(), Is().EqualTo(42));
              AssertThat(fs.can_pull(), Is().True());
              AssertThat(fs.peek(), Is().EqualTo(21));
              AssertThat(fs.pull(), Is().EqualTo(21));
              AssertThat(fs.can_pull(), Is().False());
            });

            it("can read test_file_2 [forwards]", [&]() {
              file_stream<int, false> fs1(test_file_2._files[0]);

              AssertThat(fs1.can_pull(), Is().True());
              AssertThat(fs1.pull(), Is().EqualTo(1));
              AssertThat(fs1.can_pull(), Is().True());
              AssertThat(fs1.pull(), Is().EqualTo(2));
              AssertThat(fs1.can_pull(), Is().True());
              AssertThat(fs1.pull(), Is().EqualTo(3));
              AssertThat(fs1.can_pull(), Is().False());

              file_stream<int, false> fs2(test_file_2._files[1]);

              AssertThat(fs2.can_pull(), Is().True());
              AssertThat(fs2.pull(), Is().EqualTo(4));
              AssertThat(fs2.can_pull(), Is().True());
              AssertThat(fs2.pull(), Is().EqualTo(5));
              AssertThat(fs2.can_pull(), Is().False());
            });

            it("can peek test_file_2 [forwards]", [&]() {
              file_stream<int, false> fs1(test_file_2._files[0]);

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

              file_stream<int, false> fs2(test_file_2._files[1]);

              AssertThat(fs2.can_pull(), Is().True());
              AssertThat(fs2.peek(), Is().EqualTo(4));
              AssertThat(fs2.pull(), Is().EqualTo(4));
              AssertThat(fs2.can_pull(), Is().True());
              AssertThat(fs2.peek(), Is().EqualTo(5));
              AssertThat(fs2.pull(), Is().EqualTo(5));
              AssertThat(fs2.can_pull(), Is().False());
            });

            it("can read test_file_1 forwards via fresh proxy object", [&]() {
              file<int, 1> proxy(test_file_1);
              file_stream<int, false> fs(proxy._files[0]);

              AssertThat(fs.can_pull(), Is().True());
              AssertThat(fs.pull(), Is().EqualTo(21));
              AssertThat(fs.can_pull(), Is().True());
              AssertThat(fs.pull(), Is().EqualTo(42));
              AssertThat(fs.can_pull(), Is().False());
            });

            it("can read meta stream of test_file_1", [&]() {
              meta_stream ms(test_file_1);

              AssertThat(ms.can_pull(), Is().True());
              AssertThat(ms.pull(), Is().EqualTo(meta_t {0}));
              AssertThat(ms.can_pull(), Is().False());
            });

            it("can read meta stream of node_test_file", [&]() {
              meta_stream ms(node_test_file);

              AssertThat(ms.can_pull(), Is().True());
              AssertThat(ms.pull(), Is().EqualTo(meta_t {0}));
              AssertThat(ms.can_pull(), Is().True());
              AssertThat(ms.pull(), Is().EqualTo(meta_t {1}));
              AssertThat(ms.can_pull(), Is().False());
            });

            it("can read node stream of node_test_file", [&]() {
              node_stream ns(node_test_file);

              AssertThat(ns.can_pull(), Is().True());
              AssertThat(ns.pull(), Is().EqualTo(n0));
              AssertThat(ns.can_pull(), Is().True());
              AssertThat(ns.pull(), Is().EqualTo(n1));
              AssertThat(ns.can_pull(), Is().True());
              AssertThat(ns.pull(), Is().EqualTo(n2));
              AssertThat(ns.can_pull(), Is().False());
            });

            it("can read negated node stream of node_test_file", [&]() {
              node_stream ns(node_test_file, true);

              AssertThat(ns.can_pull(), Is().True());
              AssertThat(ns.pull(), Is().EqualTo(!n0));
              AssertThat(ns.can_pull(), Is().True());
              AssertThat(ns.pull(), Is().EqualTo(!n1));
              AssertThat(ns.can_pull(), Is().True());
              AssertThat(ns.pull(), Is().EqualTo(!n2));
              AssertThat(ns.can_pull(), Is().False());
            });

            it("does not break when source file is destructed early", [&]() {
              file<int,1>* f = new file<int, 1>();

              file_writer<int,1> fw(*f);
              fw.unsafe_push(21, 0);

              file_stream<int, false> fs(f -> _files[0]);

              delete f;

              AssertThat(fs.can_pull(), Is().True());
              AssertThat(fs.pull(), Is().EqualTo(21));
            });
         });

        it("can return a written to file, that then can be read", [&]() {
          auto t = []() {
            file<int,1> f;

            file_writer<int,1> fw(f);
            fw.unsafe_push(42,0);
            fw.unsafe_push(7,0);

            return f;
          };

          file<int, 1> f = t();
          file_stream<int, false> fs(f._files[0]);

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(42));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(7));
          AssertThat(fs.can_pull(), Is().False());
        });
     });
  });
