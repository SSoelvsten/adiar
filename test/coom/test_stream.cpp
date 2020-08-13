#include <tpie/tpie.h>

#include <coom/data.cpp>
#include <coom/stream.cpp>

using namespace coom;

// Create a negation for integers to keep the test cases easy to read
template<> class coom::negator<int>
{
protected:
  int negate(const int& x) {
    return -x;
  }
};


go_bandit([]() {
    describe("COOM: Stream Decorators", [&]() {

        describe("out_stream<T>", []() {
            it("can write to stream given a path", [&]() {
                out_stream<int> os = out_stream<int>("ostream_test.coom");

                os.push(1);
                os.push(2);
              });

            it("can write to anonymous stream", [&]() {
                out_stream<int> os;

                os.push(1);
                os.push(2);
              });

            it("can be converted into an in_stream<T, forward>", [&]() {
                out_stream<int> os;

                os << 1;
                os << 2;

                in_stream<int, forward<int>> is = in_stream(os);

                is.reset();
                AssertThat(is.pull(), Is().EqualTo(1));
                AssertThat(is.pull(), Is().EqualTo(2));
              });

            it("can be converted into an in_stream<T, reverse>", [&]() {
                out_stream<int> os;

                os << 1;
                os << 2;
                os << 3;

                in_stream is = in_stream<int, reverse<int>>(os);

                is.reset();
                AssertThat(is.pull(), Is().EqualTo(3));
                AssertThat(is.pull(), Is().EqualTo(2));
                AssertThat(is.pull(), Is().EqualTo(1));
              });
          });

        // Create a _tpie_stream to wrap
        auto _tpie_stream_ptr = std::make_shared<tpie::file_stream<int>>();
        _tpie_stream_ptr -> open();

        _tpie_stream_ptr -> write(1);
        _tpie_stream_ptr -> write(2);
        _tpie_stream_ptr -> write(3);
        _tpie_stream_ptr -> write(4);

        describe("in_stream<T, forward<T>>", [&]() {
            it("can wrap a directly given tpie::file_stream", [&]() {
                in_stream is = in_stream<int, forward<int>, negator<int>>(_tpie_stream_ptr);

                is.reset();
                AssertThat(is.pull(), Is().EqualTo(1));
                AssertThat(is.pull(), Is().EqualTo(2));
                AssertThat(is.pull(), Is().EqualTo(3));
                AssertThat(is.pull(), Is().EqualTo(4));
              });

            it("can peek next element", [&]() {
                in_stream is = in_stream<int, forward<int>, negator<int>>(_tpie_stream_ptr);

                is.reset();
                AssertThat(is.peek(), Is().EqualTo(1));
                is.pull();

                AssertThat(is.peek(), Is().EqualTo(2));
                is.pull();

                AssertThat(is.peek(), Is().EqualTo(3));
                is.pull();

                AssertThat(is.peek(), Is().EqualTo(4));
                is.pull();
              });

            it("can peek next element multiple times", [&]() {
                in_stream is = in_stream<int, forward<int>, negator<int>>(_tpie_stream_ptr);

                is.reset();
                is.pull();

                AssertThat(is.peek(), Is().EqualTo(2));
                AssertThat(is.peek(), Is().EqualTo(2));
              });

            it("can reset back to the beginning", [&]() {
                in_stream is = in_stream<int, forward<int>, negator<int>>(_tpie_stream_ptr);

                is.reset();
                AssertThat(is.pull(), Is().EqualTo(1));
                AssertThat(is.pull(), Is().EqualTo(2));

                is.reset();

                AssertThat(is.pull(), Is().EqualTo(1));
              });

            it("can tell whether there are more elements", [&]() {
                in_stream is = in_stream<int, forward<int>, negator<int>>(_tpie_stream_ptr);

                is.reset();
                AssertThat(is.can_pull(), Is().True());
                is.pull();

                AssertThat(is.can_pull(), Is().True());
                is.pull();

                AssertThat(is.can_pull(), Is().True());
                is.pull();

                AssertThat(is.can_pull(), Is().True());
                is.pull();

                AssertThat(is.can_pull(), Is().False());
              });

            it("can reopen a prior persisted stream", [&]() {
                in_stream is = in_stream<int, forward<int>, negator<int>>("ostream_test.coom");

                is.reset();
                AssertThat(is.can_pull(), Is().True());
                AssertThat(is.pull(), Is().EqualTo(1));

                AssertThat(is.can_pull(), Is().True());
                AssertThat(is.pull(), Is().EqualTo(2));

                AssertThat(is.can_pull(), Is().False());
              });
          });

        describe("in_stream<T, reverse<T>>", [&]() {
            it("can wrap a directly given tpie::file_stream", [&]() {
                in_stream is = in_stream<int, reverse<int>, negator<int>>(_tpie_stream_ptr);

                is.reset();
                AssertThat(is.pull(), Is().EqualTo(4));
                AssertThat(is.pull(), Is().EqualTo(3));
                AssertThat(is.pull(), Is().EqualTo(2));
                AssertThat(is.pull(), Is().EqualTo(1));
              });

            it("can peek next element", [&]() {
                in_stream is = in_stream<int, reverse<int>, negator<int>>(_tpie_stream_ptr);

                is.reset();
                AssertThat(is.peek(), Is().EqualTo(4));
                is.pull();

                AssertThat(is.peek(), Is().EqualTo(3));
                is.pull();

                AssertThat(is.peek(), Is().EqualTo(2));
                is.pull();

                AssertThat(is.peek(), Is().EqualTo(1));
                is.pull();
              });

            it("can peek next element multiple times", [&]() {
                in_stream is = in_stream<int, reverse<int>, negator<int>>(_tpie_stream_ptr);

                is.reset();
                is.pull();

                AssertThat(is.peek(), Is().EqualTo(3));
                AssertThat(is.peek(), Is().EqualTo(3));
              });

            it("can tell whether there are more elements", [&]() {
                in_stream is = in_stream<int, reverse<int>, negator<int>>(_tpie_stream_ptr);

                is.reset();
                AssertThat(is.can_pull(), Is().True());
                is.pull();

                AssertThat(is.can_pull(), Is().True());
                is.pull();

                AssertThat(is.can_pull(), Is().True());
                is.pull();

                AssertThat(is.can_pull(), Is().True());
                is.pull();

                AssertThat(is.can_pull(), Is().False());
              });

            it("can reset back to the end", [&]() {
                in_stream is = in_stream<int, reverse<int>, negator<int>>(_tpie_stream_ptr);

                is.reset();
                AssertThat(is.pull(), Is().EqualTo(4));
                AssertThat(is.pull(), Is().EqualTo(3));

                is.reset();

                AssertThat(is.pull(), Is().EqualTo(4));
              });

            it("does not pull wrong element after a peek", [&]() {
                in_stream is = in_stream<int, reverse<int>, negator<int>>(_tpie_stream_ptr);

                is.reset();
                AssertThat(is.peek(), Is().EqualTo(4));
                AssertThat(is.pull(), Is().EqualTo(4));
              });

            it("does not persist peek after reset", [&]() {
                in_stream is = in_stream<int, reverse<int>, negator<int>>(_tpie_stream_ptr);

                is.reset();
                is.pull(); // pulls a 4
                is.peek(); // peeks a 3

                is.reset();
                AssertThat(is.peek(), Is().EqualTo(4));
              });

            it("has next element even after peeking the last", [&]() {
                in_stream is = in_stream<int, reverse<int>, negator<int>>(_tpie_stream_ptr);

                is.reset();
                is.pull(); // pulls a 4
                is.pull(); // pulls a 3
                is.pull(); // pulls a 2

                AssertThat(is.peek(), Is().EqualTo(1));
                AssertThat(is.can_pull(), Is().True());
                AssertThat(is.pull(), Is().EqualTo(1));
              });

            it("can reopen a prior persisted stream", [&]() {
                in_stream is = in_stream<int, reverse<int>, negator<int>>("ostream_test.coom");

                is.reset();
                AssertThat(is.can_pull(), Is().True());
                AssertThat(is.pull(), Is().EqualTo(2));

                AssertThat(is.can_pull(), Is().True());
                AssertThat(is.pull(), Is().EqualTo(1));

                AssertThat(is.can_pull(), Is().False());
              });
          });

        describe("in_stream<T, _, negator<T>>", [&]() {
            it("can toggle negation on and pull", [&]() {
                in_stream is = in_stream<int, reverse<int>, negator<int>>(_tpie_stream_ptr);

                is.reset();
                is.negate();

                AssertThat(is.pull(), Is().EqualTo(-4));
                AssertThat(is.pull(), Is().EqualTo(-3));
                AssertThat(is.pull(), Is().EqualTo(-2));
                AssertThat(is.pull(), Is().EqualTo(-1));
              });

            it("can toggle negation on and peek", [&]() {
                in_stream is = in_stream<int, reverse<int>, negator<int>>(_tpie_stream_ptr);

                is.reset();
                is.negate();

                is.pull();

                AssertThat(is.peek(), Is().EqualTo(-3));
                AssertThat(is.pull(), Is().EqualTo(-3));

                AssertThat(is.peek(), Is().EqualTo(-2));
                AssertThat(is.pull(), Is().EqualTo(-2));

                AssertThat(is.pull(), Is().EqualTo(-1));
              });

            it("can toggle negation off again", [&]() {
                in_stream is = in_stream<int, forward<int>, negator<int>>(_tpie_stream_ptr);

                is.reset();
                is.negate();

                AssertThat(is.pull(), Is().EqualTo(-1));
                AssertThat(is.pull(), Is().EqualTo(-2));

                is.negate();

                AssertThat(is.pull(), Is().EqualTo(3));
                AssertThat(is.pull(), Is().EqualTo(4));
              });
          });

        describe("node_stream", [&]() {
            auto sink_T = create_sink(true);
            auto sink_F = create_sink(false);

            out_stream out = out_stream<node>();

            // The OBDD for Evaluate

            auto n5 = create_node(3,0, sink_F, sink_T);
            out << n5;

            auto n4 = create_node(2,1, sink_T, n5.node_ptr);
            out << n4;

            auto n3 = create_node(2,0, sink_F, sink_T);
            out << n3;

            auto n2 = create_node(1,0, n3.node_ptr, n4.node_ptr);
            out << n2;

            auto n1 = create_node(0,0, n3.node_ptr, n2.node_ptr);
            out << n1;

            node_stream obdd = node_stream(out);

            it("reads in reverse of output", [&]() {
                obdd.reset();

                AssertThat(obdd.pull(), Is().EqualTo(n1));
                AssertThat(obdd.pull(), Is().EqualTo(n2));
                AssertThat(obdd.pull(), Is().EqualTo(n3));
                AssertThat(obdd.pull(), Is().EqualTo(n4));
                AssertThat(obdd.pull(), Is().EqualTo(n5));
              });

            it("can negate the obdd", [&]() {
                obdd.reset();
                obdd.negate();

                AssertThat(obdd.pull(), Is().EqualTo(n1));
                AssertThat(obdd.pull(), Is().EqualTo(n2));
                AssertThat(obdd.pull(), Is().EqualTo(create_node(2,0, sink_T, sink_F)));
                AssertThat(obdd.pull(), Is().EqualTo(create_node(2,1, sink_F, n5.node_ptr)));
                AssertThat(obdd.pull(), Is().EqualTo(create_node(3,0, sink_T, sink_F)));
              });
          });
      });
  });
