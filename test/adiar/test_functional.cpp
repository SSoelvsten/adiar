#include "../test.h"

go_bandit([]() {
  describe("adiar/functional.h", []() {
    describe("make_consumer(ForwardIt&, ForwardIt&)", []() {
      it("consumes values 0, 1, 42 into std::vector<int>", []() {
        std::vector<int> xs = { -1, -1, -1 };

        auto begin = xs.begin();
        auto end   = xs.end();

        consumer<int> c = make_consumer(begin, end);

        AssertThat(xs.at(0), Is().EqualTo(-1));
        c(0);
        AssertThat(xs.at(0), Is().EqualTo(0));

        AssertThat(xs.at(1), Is().EqualTo(-1));
        c(1);
        AssertThat(xs.at(1), Is().EqualTo(1));

        AssertThat(xs.at(2), Is().EqualTo(-1));
        c(42);
        AssertThat(xs.at(2), Is().EqualTo(42));
      });

      it("consumes values 4, -2 into std::vector<int>", []() {
        std::vector<int> xs = { -1, -1, -1 };

        auto begin = xs.begin();
        auto end   = xs.end();

        consumer<int> c = make_consumer(begin, end);

        AssertThat(xs.at(0), Is().EqualTo(-1));
        c(4);
        AssertThat(xs.at(0), Is().EqualTo(4));

        AssertThat(xs.at(1), Is().EqualTo(-1));
        c(-2);
        AssertThat(xs.at(1), Is().EqualTo(-2));

        AssertThat(xs.at(2), Is().EqualTo(-1));
      });

      it("has side effect on begin iterators", []() {
        std::vector<int> xs = { -1, -1, -1 };

        auto begin = xs.begin();
        auto end   = xs.end();

        consumer<int> c = make_consumer(begin, end);

        AssertThat(begin, Is().EqualTo(xs.begin()));
        c(0);
        AssertThat(begin, Is().EqualTo(xs.begin()+1));
        c(0);
        AssertThat(begin, Is().EqualTo(xs.begin()+2));
        c(0);
        AssertThat(begin, Is().EqualTo(xs.begin()+3));
        AssertThat(begin, Is().EqualTo(end));
      });

      it("throws exception when overflowing iterator range", []() {
        std::vector<int> xs = { -1, -1, -1 };

        auto begin = xs.begin();
        auto end   = xs.end();

        consumer<int> c = make_consumer(begin, end);

        c(0);
        c(0);
        c(0);

        AssertThrows(out_of_range, c(0));
      });
    });

    describe("make_consumer(ForwardIt&&, ForwardIt&&)", []() {
      it("consumes values 2, 4, 8 into std::vector<int>", []() {
        std::vector<int> xs = { -1, -1, -1 };

        consumer<int> c = make_consumer(xs.begin(), xs.end());

        AssertThat(xs.at(0), Is().EqualTo(-1));
        c(2);
        AssertThat(xs.at(0), Is().EqualTo(2));

        AssertThat(xs.at(1), Is().EqualTo(-1));
        c(4);
        AssertThat(xs.at(1), Is().EqualTo(4));

        AssertThat(xs.at(2), Is().EqualTo(-1));
        c(8);
        AssertThat(xs.at(2), Is().EqualTo(8));
      });

      it("consumes values 0, 1 into std::vector<int>", []() {
        std::vector<int> xs = { -1, -1, -1 };

        consumer<int> c = make_consumer(xs.begin(), xs.end());

        AssertThat(xs.at(0), Is().EqualTo(-1));
        c(0);
        AssertThat(xs.at(0), Is().EqualTo(0));

        AssertThat(xs.at(1), Is().EqualTo(-1));
        c(1);
        AssertThat(xs.at(1), Is().EqualTo(1));

        AssertThat(xs.at(2), Is().EqualTo(-1));
      });

      it("throws exception when overflowing iterator range", []() {
        std::vector<int> xs = { -1, -1, -1 };

        consumer<int> c = make_consumer(xs.begin(), xs.end());

        c(0);
        c(0);
        c(0);

        AssertThrows(out_of_range, c(0));
      });
    });

    describe("make_generator(ForwardIt&, ForwardIt&)", []() {
      it("wraps std::vector<int> = { }", []() {
        const std::vector<int> xs = {};

        auto begin = xs.begin();
        auto end   = xs.end();

        generator<int> g = make_generator(begin, end);

        AssertThat(g(), Is().EqualTo(make_optional<int>()));
      });

      it("wraps std::vector<int> = { 0 }", []() {
        const std::vector<int> xs = {0};

        auto begin = xs.begin();
        auto end   = xs.end();

        generator<int> g = make_generator(begin, end);

        AssertThat(g(), Is().EqualTo(make_optional<int>(0)));
        AssertThat(g(), Is().EqualTo(make_optional<int>()));
      });

      it("wraps std::vector<int> = { 0, 1 }", []() {
        const std::vector<int> xs = {0,1};

        auto begin = xs.begin();
        auto end   = xs.end();

        generator<int> g = make_generator(begin, end);

        AssertThat(g(), Is().EqualTo(make_optional<int>(0)));
        AssertThat(g(), Is().EqualTo(make_optional<int>(1)));
        AssertThat(g(), Is().EqualTo(make_optional<int>()));
      });

      it("wraps std::vector<int> = { -1, 0, 1 }", []() {
        const std::vector<int> xs = {-1, 0, 1};

        auto begin = xs.begin();
        auto end   = xs.end();

        generator<int> g = make_generator(begin, end);

        AssertThat(g(), Is().EqualTo(make_optional<int>(-1)));
        AssertThat(g(), Is().EqualTo(make_optional<int>(0)));
        AssertThat(g(), Is().EqualTo(make_optional<int>(1)));
        AssertThat(g(), Is().EqualTo(make_optional<int>()));
      });

      it("wraps std::vector<int> = { 4, 2, 0 }", []() {
        const std::vector<int> xs = {4,2,0};

        auto begin = xs.begin();
        auto end   = xs.end();

        generator<int> g = make_generator(begin, end);

        AssertThat(g(), Is().EqualTo(make_optional<int>(4)));
        AssertThat(g(), Is().EqualTo(make_optional<int>(2)));
        AssertThat(g(), Is().EqualTo(make_optional<int>(0)));
        AssertThat(g(), Is().EqualTo(make_optional<int>()));
      });

      it("wraps std::vector<int> = { 2, 2 }", []() {
        const std::vector<int> xs = {2,2};

        auto begin = xs.begin();
        auto end   = xs.end();

        generator<int> g = make_generator(begin, end);

        AssertThat(g(), Is().EqualTo(make_optional<int>(2)));
        AssertThat(g(), Is().EqualTo(make_optional<int>(2)));
        AssertThat(g(), Is().EqualTo(make_optional<int>()));
      });

      it("wraps std::vector<int> = { 2, -1, 0, 2 }", []() {
        const std::vector<int> xs = {2,-1,0,2};

        auto begin = xs.begin();
        auto end   = xs.end();

        generator<int> g = make_generator(begin, end);

        AssertThat(g(), Is().EqualTo(make_optional<int>(2)));
        AssertThat(g(), Is().EqualTo(make_optional<int>(-1)));
        AssertThat(g(), Is().EqualTo(make_optional<int>(0)));
        AssertThat(g(), Is().EqualTo(make_optional<int>(2)));
        AssertThat(g(), Is().EqualTo(make_optional<int>()));
      });
    });

    describe("make_generator(ForwardIt&&, ForwardIt&&)", []() {
      it("wraps std::vector<int> = { }", []() {
        const std::vector<int> xs = {};

        generator<int> g = make_generator(xs.begin(), xs.end());

        AssertThat(g(), Is().EqualTo(make_optional<int>()));
      });

      it("wraps std::vector<int> = { 0 }", []() {
        const std::vector<int> xs = {0};

        generator<int> g = make_generator(xs.begin(), xs.end());

        AssertThat(g(), Is().EqualTo(make_optional<int>(0)));
        AssertThat(g(), Is().EqualTo(make_optional<int>()));
      });

      it("wraps std::vector<int> = { 0, 1 }", []() {
        const std::vector<int> xs = {0,1};

        generator<int> g = make_generator(xs.begin(), xs.end());

        AssertThat(g(), Is().EqualTo(make_optional<int>(0)));
        AssertThat(g(), Is().EqualTo(make_optional<int>(1)));
        AssertThat(g(), Is().EqualTo(make_optional<int>()));
      });

      it("wraps std::vector<int> = { -1, 0, 1 }", []() {
        const std::vector<int> xs = {-1, 0, 1};

        generator<int> g = make_generator(xs.begin(), xs.end());

        AssertThat(g(), Is().EqualTo(make_optional<int>(-1)));
        AssertThat(g(), Is().EqualTo(make_optional<int>(0)));
        AssertThat(g(), Is().EqualTo(make_optional<int>(1)));
        AssertThat(g(), Is().EqualTo(make_optional<int>()));
      });

      it("wraps std::vector<int> = { 4, 2, 0 }", []() {
        const std::vector<int> xs = {4,2,0};

        generator<int> g = make_generator(xs.begin(), xs.end());

        AssertThat(g(), Is().EqualTo(make_optional<int>(4)));
        AssertThat(g(), Is().EqualTo(make_optional<int>(2)));
        AssertThat(g(), Is().EqualTo(make_optional<int>(0)));
        AssertThat(g(), Is().EqualTo(make_optional<int>()));
      });

      it("wraps std::vector<int> = { 2, 2 }", []() {
        const std::vector<int> xs = {2,2};

        generator<int> g = make_generator(xs.begin(), xs.end());

        AssertThat(g(), Is().EqualTo(make_optional<int>(2)));
        AssertThat(g(), Is().EqualTo(make_optional<int>(2)));
        AssertThat(g(), Is().EqualTo(make_optional<int>()));
      });

      it("wraps std::vector<int> = { 2, -1, 0, 2 }", []() {
        const std::vector<int> xs = {2,-1,0,2};

        generator<int> g = make_generator(xs.begin(), xs.end());

        AssertThat(g(), Is().EqualTo(make_optional<int>(2)));
        AssertThat(g(), Is().EqualTo(make_optional<int>(-1)));
        AssertThat(g(), Is().EqualTo(make_optional<int>(0)));
        AssertThat(g(), Is().EqualTo(make_optional<int>(2)));
        AssertThat(g(), Is().EqualTo(make_optional<int>()));
      });
    });

    describe("make_generator(const RetType&)", []() {
      it("Can create -1 generator", []() {
        generator<int> g = make_generator(-1);

        AssertThat(g(), Is().EqualTo(make_optional<int>(-1)));
        AssertThat(g(), Is().EqualTo(make_optional<int>()));
      });

      it("Can create 0 generator", []() {
        generator<int> g = make_generator(0);

        AssertThat(g(), Is().EqualTo(make_optional<int>(0)));
        AssertThat(g(), Is().EqualTo(make_optional<int>()));
      });

      it("Can create 1 generator", []() {
        generator<int> g = make_generator(1);

        AssertThat(g(), Is().EqualTo(make_optional<int>(1)));
        AssertThat(g(), Is().EqualTo(make_optional<int>()));
      });

      it("Can create 42 generator", []() {
        generator<int> g = make_generator(42);

        AssertThat(g(), Is().EqualTo(make_optional<int>(42)));
        AssertThat(g(), Is().EqualTo(make_optional<int>()));
      });
    });
  });
 });
