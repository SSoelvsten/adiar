#include "../../../test.h"

#include <filesystem>

go_bandit([]() {
  describe("adiar/internal/io/file.h , file_stream.h , file_writer.h", []() {
    describe("file() [empty]", []() {
      it("creates a file marked as 'temporary'", []() {
        file<int> f;
        AssertThat(f.is_persistent(), Is().False());
        AssertThat(f.is_temp(), Is().True());
      });

      it("has no content", []() {
        file<int> f;
        AssertThat(f.size(), Is().EqualTo(0u));
        AssertThat(f.empty(), Is().True());
      });

      it("does not (yet) 'exist' on disk", []() {
        file<int> f;

        AssertThat(f.exists(), Is().False());
        AssertThat(std::filesystem::exists(f.path()), Is().False());
      });

      it("'exists' after a 'touch'", []() {
        file<int> f;
        f.touch();

        AssertThat(f.exists(), Is().True());
        AssertThat(std::filesystem::exists(f.path()), Is().True());
      });

      it("is a fresh file", []() {
        file<int> f1;
        file<int> f2;
        AssertThat(f1.path(), Is().Not().EqualTo(f2.path()));
      });

      it("is temporary (removed from disk after destruct)", []() {
        std::string path;
        { // Scope to destruct 'f' early
          file<int> f;
          f.touch();

          path = f.path();
          AssertThat(std::filesystem::exists(path), Is().True());
        }
        AssertThat(std::filesystem::exists(path), Is().False());
      });

      it("can be 'moved' when not existing", []() {
        std::string new_path = "./after-move-path.adiar";
        if (std::filesystem::exists(new_path)) {
          // Clean up after prior test run
          std::filesystem::remove(new_path);
        }

        file<int> f;
        std::string old_path = f.path();

        AssertThat(std::filesystem::exists(old_path), Is().False());
        AssertThat(std::filesystem::exists(new_path), Is().False());

        f.move(new_path);

        AssertThat(std::filesystem::exists(old_path), Is().False());
        AssertThat(std::filesystem::exists(new_path), Is().False());

        f.touch();

        AssertThat(std::filesystem::exists(old_path), Is().False());
        AssertThat(std::filesystem::exists(new_path), Is().True());
      });

      it("can be 'moved' when existing", []() {
        std::string new_path = "./after-move-path.adiar";
        if (std::filesystem::exists(new_path)) {
          // Clean up after prior test run
          std::filesystem::remove(new_path);
        }

        file<int> f;
        f.touch();
        std::string old_path = f.path();

        AssertThat(std::filesystem::exists(old_path), Is().True());
        AssertThat(std::filesystem::exists(new_path), Is().False());

        f.move(new_path);

        AssertThat(std::filesystem::exists(old_path), Is().False());
        AssertThat(std::filesystem::exists(new_path), Is().True());
      });

      it("cannot be 'moved' on-top of another (non-empty) file", []() {
        file<int> f1;
        f1.touch();

        file<int> f2;
        f2.touch();

        AssertThrows(std::runtime_error, f1.move(f2.path()));
      });

      it("is still temporary after move", []() {
        std::string new_path = "./after-move-path.adiar";
        if (std::filesystem::exists(new_path)) {
          // Clean up after prior test run
          std::filesystem::remove(new_path);
        }

        {
          file<int> f;
          f.touch();
          std::string old_path = f.path();

          f.move(new_path);

          AssertThat(std::filesystem::exists(new_path), Is().True());

          AssertThat(f.is_persistent(), Is().False());
          AssertThat(f.is_temp(), Is().True());
        }
        AssertThat(std::filesystem::exists(new_path), Is().False());
      });

      it("can be made persistent (not removed from disk)", []() {
        std::string path;
        { // Scope to destruct 'f' early
          file<int> f;
          f.touch();

          path = f.path();
          AssertThat(std::filesystem::exists(path), Is().True());

          f.make_persistent();
          AssertThat(f.is_persistent(), Is().True());
          AssertThat(f.is_temp(), Is().False());
        }
        AssertThat(std::filesystem::exists(path), Is().True());

        // Clean up for this test
        std::filesystem::remove(path);
      });

      it("exists on disk after being made persistent", []() {
        std::string path;
        { // Scope to destruct 'f' early
          file<int> f;
          path = f.path();

          AssertThat(f.exists(), Is().False());
          AssertThat(std::filesystem::exists(path), Is().False());
          f.make_persistent();
          AssertThat(f.exists(), Is().True());
          AssertThat(std::filesystem::exists(path), Is().True());
        }
        // Clean up for this test
        if (std::filesystem::exists(path)) {
          std::filesystem::remove(path);
        }
      });
    });

    describe("file(path) [empty]", []() {
      it("throws exception on path to non-existing file", []() {
        AssertThrows(std::runtime_error,
                     file<int>("./non-existing-file.adiar"));
      });

      std::string path;
      {
        file<int> f;
        f.touch();

        path = f.path();
        f.make_persistent();
      }

      it("can reopen existing file", [&path]() {
        file<int> f(path);
        AssertThat(f.path(), Is().EqualTo(path));

        AssertThat(f.exists(), Is().True());
        AssertThat(f.size(), Is().EqualTo(0u));
      });

      it("is marked persistent", [&path]() {
        file<int> f(path);
        AssertThat(f.is_persistent(), Is().True());
        AssertThat(f.is_temp(), Is().False());
      });

      it("is unchanged after marking it persistent once more", [&path]() {
        {
          file<int> f(path);
          f.make_persistent();

          AssertThat(f.is_persistent(), Is().True());
          AssertThat(f.is_temp(), Is().False());
        }
        AssertThat(std::filesystem::exists(path), Is().True());
      });

      it("cannot be 'moved'", [&path]() {
        std::string new_path = "./after-move-path.adiar";
        if (std::filesystem::exists(new_path)) {
          // Clean up after prior test run
          std::filesystem::remove(new_path);
        }

        file<int> f(path);
        std::string old_path = f.path();

        AssertThrows(std::runtime_error, f.move(new_path));
      });

      // Clean up for this test
      if (std::filesystem::exists(path)) {
        std::filesystem::remove(path);
      }
    });

    describe("file() + file_stream", []() {
      it("can attach to an empty file [constructor]", []() {
        file<int> f;
        file_stream<int> fs(f);
      });

      it("can attach to an empty file [member function]", []() {
        file<int> f;
        file_stream<int> fs;
        fs.attach(f);
      });

      it("remembers it was attached", []() {
        file<int> f;
        file_stream<int> fs(f);
        AssertThat(fs.attached(), Is().True());
      });

      it("cannot be pulled from", []() {
        file<int> f;
        file_stream<int> fs(f);

        AssertThat(fs.can_pull(), Is().False());
      });

      it("can be reset", []() {
        file<int> f;
        file_stream<int> fs(f);

        fs.reset();
        AssertThat(fs.attached(), Is().True());
        AssertThat(fs.can_pull(), Is().False());
      });

      it("can be detached again", []() {
        file<int> f;
        file_stream<int> fs(f);
        AssertThat(fs.attached(), Is().True());
        fs.detach();
        AssertThat(fs.attached(), Is().False());
      });
    });

    describe("file() + file_writer", []() {
      it("can attach to an empty file [constructor]", []() {
        file<int> f;
        file_writer<int> fw(f);
      });

      it("can attach to an empty file [member function]", []() {
        file<int> f;
        file_writer<int> fw;
        fw.attach(f);
      });

      it("remembers it was attached", []() {
        file<int> f;
        file_writer<int> fw(f);
        AssertThat(fw.attached(), Is().True());
      });

      it("can be detached again", []() {
        file<int> f;
        file_writer<int> fw(f);
        AssertThat(fw.attached(), Is().True());
        fw.detach();
        AssertThat(fw.attached(), Is().False());
      });

      it("exists on writer attach", []() {
        file<int> f;
        AssertThat(f.exists(), Is().False());

        file_writer<int> fw(f);
        fw.detach();

        AssertThat(f.exists(), Is().True());
      });

      it("changes size when writing content to file [1]", []() {
        file<int> f;

        file_writer<int> fw(f);
        fw << 1 << 2 << 3;
        fw.detach();

        AssertThat(f.size(), Is().EqualTo(3u));
      });

      it("changes size when writing content to file [2]", []() {
        file<int> f;

        file_writer<int> fw(f);
        fw << 42 << 21;
        fw.detach();

        AssertThat(f.size(), Is().EqualTo(2u));
      });

      it("can be 'moved' after write", []() {
        std::string new_path = "./after-move-path.adiar";
        if (std::filesystem::exists(new_path)) {
          // Clean up after prior test run
          std::filesystem::remove(new_path);
        }

        file<int> f;

        file_writer<int> fw(f);
        fw << 42 << 21;
        fw.detach();

        std::string old_path = f.path();

        AssertThat(std::filesystem::exists(old_path), Is().True());
        AssertThat(std::filesystem::exists(new_path), Is().False());

        f.move(new_path);

        AssertThat(std::filesystem::exists(old_path), Is().False());
        AssertThat(std::filesystem::exists(new_path), Is().True());
      });
    });

    describe("file() + file_stream + file_writer", []() {
      it("can read written content [1]", []() {
        file<int> f;

        file_writer<int> fw(f);
        fw << 1 << 2 << 3;
        fw.detach();
        AssertThat(fw.attached(), Is().False());

        file_stream<int, false> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(1));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(2));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(3));
        AssertThat(fs.can_pull(), Is().False());
        fs.detach();
      });

      it("can read written content [2]", []() {
        file<int> f;

        file_writer<int> fw(f);
        fw << 42 << 21;
        fw.detach();
        AssertThat(fw.attached(), Is().False());

        file_stream<int, false> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(42));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(21));
        AssertThat(fs.can_pull(), Is().False());
        fs.detach();
      });

      it("can read written content in reverse [1]", []() {
        file<int> f;

        file_writer<int> fw(f);
        fw << 1 << 2 << 3;
        fw.detach();
        AssertThat(fw.attached(), Is().False());

        file_stream<int, true> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(3));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(2));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(1));
        AssertThat(fs.can_pull(), Is().False());
        fs.detach();
      });

      it("can read written content in reverse [2]", []() {
        file<int> f;

        file_writer<int> fw(f);
        fw << 42 << 21;
        fw.detach();
        AssertThat(fw.attached(), Is().False());

        file_stream<int, true> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(21));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(42));
        AssertThat(fs.can_pull(), Is().False());
        fs.detach();
      });

      it("can read from 'moved' file", []() {
        std::string new_path = "./after-move-path.adiar";
        if (std::filesystem::exists(new_path)) {
          // Clean up after prior test run
          std::filesystem::remove(new_path);
        }

        file<int> f;

        file_writer<int> fw(f);
        fw << 12 << 9 << 1;
        fw.detach();

        f.move(new_path);

        file_stream<int, false> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(12));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(9));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(1));
        AssertThat(fs.can_pull(), Is().False());
        fs.detach();
      });

      it("can read from 'moved' file in reverse", []() {
        std::string new_path = "./after-move-path.adiar";
        if (std::filesystem::exists(new_path)) {
          // Clean up after prior test run
          std::filesystem::remove(new_path);
        }

        file<int> f;

        file_writer<int> fw(f);
        fw << 8 << 9 << 4 << 2;
        fw.detach();

        f.move(new_path);

        file_stream<int, true> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(2));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(4));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(9));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(8));
        AssertThat(fs.can_pull(), Is().False());
        fs.detach();
      });
    });

    describe("file.sort()", []() {
      it("can sort non-existing file", []() {
        file<int> f;
        AssertThat(f.exists(), Is().False());

        f.sort<std::less<int>>();
        AssertThat(f.exists(), Is().False());

        file_stream<int> fs(f);
        AssertThat(fs.can_pull(), Is().False());
        fs.detach();
      });

      it("can sort existing empty file", []() {
        file<int> f;
        f.touch();
        AssertThat(f.exists(), Is().True());

        f.sort<std::less<int>>();
        AssertThat(f.exists(), Is().True());

        file_stream<int> fs(f);
        AssertThat(fs.can_pull(), Is().False());
        fs.detach();
      });

      it("can sort non-empty file [1]", []() {
        file<int> f;

        file_writer<int> fw(f);
        fw << 8 << 9 << 4 << 2;
        fw.detach();

        f.sort<std::less<int>>();

        file_stream<int> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(2));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(4));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(8));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(9));
        AssertThat(fs.can_pull(), Is().False());
        fs.detach();
      });

      it("can sort non-empty file [2]", []() {
        file<int> f;

        file_writer<int> fw(f);
        fw << 42 << -1 << 8 << 21 << 8 << 3;
        fw.detach();

        f.sort<std::less<int>>();

        file_stream<int> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(-1));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(3));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(8));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(8));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(21));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(42));
        AssertThat(fs.can_pull(), Is().False());
        fs.detach();
      });
    });

    describe("::copy(const file&)", []() {
      it("can copy over non-existing file", []() {
        file<int> f1;
        AssertThat(f1.exists(), Is().False());

        file<int> f2 = file<int>::copy(f1);
        AssertThat(f2.exists(), Is().False());

        AssertThat(f2.path(), Is().Not().EqualTo(f1.path()));
      });

      it("is unaffected by original file being 'touched'", []() {
        file<int> f1;
        file<int> f2 = file<int>::copy(f1);

        f1.touch();
        AssertThat(f2.exists(), Is().False());
      });

      it("can copy over an existing file [empty]", []() {
        file<int> f1;
        f1.touch();
        AssertThat(f1.exists(), Is().True());

        file<int> f2 = file<int>::copy(f1);
        AssertThat(f2.exists(), Is().True());
        AssertThat(f2.path(), Is().Not().EqualTo(f1.path()));
      });

      it("can copy over an existing file [non-empty, 1]", []() {
        file<int> f1;
        file_writer<int> fw(f1);

        fw << 21 << 42 << 21;
        fw.detach();
        AssertThat(f1.exists(), Is().True());
        AssertThat(f1.size(), Is().EqualTo(3u));

        file<int> f2 = file<int>::copy(f1);

        AssertThat(f2.exists(), Is().True());
        AssertThat(f2.size(), Is().EqualTo(3u));
        AssertThat(f2.path(), Is().Not().EqualTo(f1.path()));

        file_stream<int> fs(f2);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(21));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(42));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(21));
        AssertThat(fs.can_pull(), Is().False());
        fs.detach();
      });

      it("can copy over an existing file [non-empty, 2]", []() {
        file<int> f1;
        file_writer<int> fw(f1);

        fw << 0 << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8 << 9;
        fw.detach();
        AssertThat(f1.exists(), Is().True());
        AssertThat(f1.size(), Is().EqualTo(10u));

        file<int> f2 = file<int>::copy(f1);

        AssertThat(f2.exists(), Is().True());
        AssertThat(f2.size(), Is().EqualTo(10u));
        AssertThat(f2.path(), Is().Not().EqualTo(f1.path()));

        file_stream<int> fs(f2);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(0));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(1));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(2));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(3));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(4));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(5));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(6));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(7));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(8));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(9));
        AssertThat(fs.can_pull(), Is().False());
        fs.detach();
      });

      it("is temporary if original file is temporary [empty]", []() {
        file<int> f1;
        f1.touch();

        file<int> f2 = file<int>::copy(f1);
        AssertThat(f2.exists(), Is().True());
        AssertThat(f2.is_persistent(), Is().False());
        AssertThat(f2.is_temp(), Is().True());
      });

      it("is temporary if original file is temporary [non-empty]", []() {
        file<int> f1;
        file_writer<int> fw(f1);
        fw << 0 << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8 << 9;
        fw.detach();
        AssertThat(f1.exists(), Is().True());
        AssertThat(f1.size(), Is().EqualTo(10u));

        file<int> f2 = file<int>::copy(f1);
        AssertThat(f2.exists(), Is().True());
        AssertThat(f2.is_persistent(), Is().False());
        AssertThat(f2.is_temp(), Is().True());
      });

      it("is temporary if original file is persistent [empty]", []() {
        std::string path;
        {
          file<int> f1;
          f1.touch();
          f1.make_persistent();

          file<int> f2 = file<int>::copy(f1);
          AssertThat(f2.exists(), Is().True());
          AssertThat(f2.is_persistent(), Is().False());
          AssertThat(f2.is_temp(), Is().True());

          // clean up after f1
          path = f1.path();
        }
        std::filesystem::remove(path);
      });

      it("is temporary if original file is persistent [non-empty]", []() {
        std::string path;
        {
          file<int> f1;
          f1.make_persistent();

          file_writer<int> fw(f1);
          fw << 0 << 1 << 2 << 3 << 4;
          fw.detach();

          file<int> f2 = file<int>::copy(f1);
          AssertThat(f2.exists(), Is().True());
          AssertThat(f2.is_persistent(), Is().False());
          AssertThat(f2.is_temp(), Is().True());

          // clean up after f1
          path = f1.path();
        }
        std::filesystem::remove(path);
      });
    });
  });
 });
