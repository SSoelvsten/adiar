#include "../../../test.h"

#include <filesystem>

go_bandit([]() {
  describe("adiar/internal/io/file.h , file_stream.h , file_writer.h", []() {
    // The default folder for temporary files is '/tmp/' on Ubuntu and '/var/tmp/'
    // on Fedora. Both of these are to the OS not on the same drive and so you get
    // a 'cross-device link' error when using std::filesystem::rename(...) to move
    // it to './'.
    //
    // To ensure our tests properly cover BOTH cases, we have to move it inside of
    // the '/tmp/' folder and also to './'.
    //
    // HACK: get the temporary folder itself directly from TPIE.
    const std::string tmp_path = tpie::tempname::get_actual_path() + "/";
    const std::string curr_path = "./";

    describe("file() [empty]", [&tmp_path, &curr_path]() {
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

      it("can be 'moved' when not existing [./]", [&curr_path]() {
        std::string new_path = curr_path + "after-move-path.adiar";
        if (std::filesystem::exists(new_path)) {
          // Clean up after prior test run
          std::filesystem::remove(new_path);
        }

        file<int> f;
        std::string old_path = f.path();

        AssertThat(std::filesystem::exists(old_path), Is().False());
        AssertThat(std::filesystem::exists(new_path), Is().False());

        AssertThat(f.can_move(), Is().True());
        f.move(new_path);

        AssertThat(std::filesystem::exists(old_path), Is().False());
        AssertThat(std::filesystem::exists(new_path), Is().False());

        f.touch();

        AssertThat(std::filesystem::exists(old_path), Is().False());
        AssertThat(std::filesystem::exists(new_path), Is().True());
      });

      it("cannot be 'moved' on-top of an existing file", []() {
        file<int> f1;
        f1.touch();

        file<int> f2;
        f2.touch();

        AssertThat(f1.can_move(), Is().True());
        AssertThrows(std::runtime_error, f1.move(f2.path()));
      });

      it("can be 'moved' when existing [/tmp/]", [&tmp_path]() {
        std::string new_path = tmp_path + "after-move-path.adiar";
        if (std::filesystem::exists(new_path)) {
          // Clean up after prior test run
          std::filesystem::remove(new_path);
        }

        file<int> f;
        f.touch();
        std::string old_path = f.path();

        AssertThat(std::filesystem::exists(old_path), Is().True());
        AssertThat(std::filesystem::exists(new_path), Is().False());

        AssertThat(f.can_move(), Is().True());
        f.move(new_path);

        AssertThat(std::filesystem::exists(old_path), Is().False());
        AssertThat(std::filesystem::exists(new_path), Is().True());
      });

      it("can be 'moved' when existing [./]", [&curr_path]() {
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

        AssertThat(f.can_move(), Is().True());
        f.move(new_path);

        AssertThat(std::filesystem::exists(old_path), Is().False());
        AssertThat(std::filesystem::exists(new_path), Is().True());
      });

      it("is still temporary after move [/tmp/]", [&tmp_path]() {
        std::string new_path = tmp_path + "after-move-path.adiar";
        if (std::filesystem::exists(new_path)) {
          // Clean up after prior test run
          std::filesystem::remove(new_path);
        }

        {
          file<int> f;
          f.touch();
          std::string old_path = f.path();

          AssertThat(f.can_move(), Is().True());
          f.move(new_path);

          AssertThat(std::filesystem::exists(new_path), Is().True());

          AssertThat(f.is_persistent(), Is().False());
          AssertThat(f.is_temp(), Is().True());
        }
        AssertThat(std::filesystem::exists(new_path), Is().False());
      });

      it("is still temporary after move [./]", [&curr_path]() {
        std::string new_path = curr_path + "after-move-path.adiar";
        if (std::filesystem::exists(new_path)) {
          // Clean up after prior test run
          std::filesystem::remove(new_path);
        }

        {
          file<int> f;
          f.touch();
          std::string old_path = f.path();

          AssertThat(f.can_move(), Is().True());
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

    describe("file() + file_stream", []() {
      it("can attach to and detach from an empty file [con-/destructor]", []() {
        file<int> f;
        adiar::internal::file_stream<int> fs(f);
      });

      it("can attach to and detach from an empty file [member functions]", []() {
        file<int> f;
        adiar::internal::file_stream<int> fs;
        fs.attach(f);
      });

      it("remembers it was attached", []() {
        file<int> f;
        adiar::internal::file_stream<int> fs(f);
        AssertThat(fs.attached(), Is().True());
        fs.detach();
        AssertThat(fs.attached(), Is().False());
      });

      it("cannot be pulled from", []() {
        file<int> f;
        adiar::internal::file_stream<int> fs(f);

        AssertThat(fs.can_pull(), Is().False());
      });

      it("can be reset", []() {
        file<int> f;
        adiar::internal::file_stream<int> fs(f);

        fs.reset();
        AssertThat(fs.attached(), Is().True());
        AssertThat(fs.can_pull(), Is().False());
      });
    });

    describe("file() + file_writer", [&tmp_path, &curr_path]() {
      it("can attach to and detach from an empty file [con-/destructor]", []() {
        file<int> f;
        adiar::file_writer<int> fw(f);
      });

      it("can attach to and detach from an empty file [member functions]", []() {
        file<int> f;
        adiar::file_writer<int> fw;
        fw.detach();
      });

      it("remembers it was attached", []() {
        file<int> f;
        adiar::file_writer<int> fw(f);
        AssertThat(fw.attached(), Is().True());
        fw.detach();
        AssertThat(fw.attached(), Is().False());
      });

      it("exists after writer attach", []() {
        file<int> f;
        AssertThat(f.exists(), Is().False());

        adiar::file_writer<int> fw(f);
        fw.detach();

        AssertThat(f.exists(), Is().True());
      });

      it("reports whether elements were pushed", []() {
        file<int> f;
        adiar::file_writer<int> fw(f);

        AssertThat(fw.has_pushed(), Is().False());
        AssertThat(fw.empty(), Is().True());
        fw << 1;

        AssertThat(fw.has_pushed(), Is().True());
        AssertThat(fw.empty(), Is().False());

        fw << 2 << 3;

        AssertThat(fw.has_pushed(), Is().True());
        AssertThat(fw.empty(), Is().False());
      });

      it("changes size when writing content to file [1]", []() {
        file<int> f;

        adiar::file_writer<int> fw(f);

        AssertThat(fw.size(), Is().EqualTo(0u));
        fw << 1 << 2;
        AssertThat(fw.size(), Is().EqualTo(2u));
        fw << 3;
        AssertThat(fw.size(), Is().EqualTo(3u));
        fw.detach();

        AssertThat(f.size(), Is().EqualTo(3u));
      });

      it("changes size when writing content to file [2]", []() {
        file<int> f;

        adiar::file_writer<int> fw(f);
        AssertThat(fw.size(), Is().EqualTo(0u));
        fw << 42;
        AssertThat(fw.size(), Is().EqualTo(1u));
        fw << 21;
        AssertThat(fw.size(), Is().EqualTo(2u));
        fw.detach();

        AssertThat(f.size(), Is().EqualTo(2u));
      });

      it("can be 'moved' after write [/tmp/]", [&tmp_path]() {
        std::string new_path = tmp_path + "after-move-path.adiar";
        if (std::filesystem::exists(new_path)) {
          // Clean up after prior test run
          std::filesystem::remove(new_path);
        }

        file<int> f;

        adiar::file_writer<int> fw(f);
        fw << 42 << 21;
        fw.detach();

        std::string old_path = f.path();

        AssertThat(std::filesystem::exists(old_path), Is().True());
        AssertThat(std::filesystem::exists(new_path), Is().False());

        AssertThat(f.can_move(), Is().True());
        f.move(new_path);

        AssertThat(std::filesystem::exists(old_path), Is().False());
        AssertThat(std::filesystem::exists(new_path), Is().True());
      });

      it("can be 'moved' after write [./]", [&curr_path]() {
        std::string new_path = curr_path + "after-move-path.adiar";
        if (std::filesystem::exists(new_path)) {
          // Clean up after prior test run
          std::filesystem::remove(new_path);
        }

        file<int> f;

        adiar::file_writer<int> fw(f);
        fw << 42 << 21;
        fw.detach();

        std::string old_path = f.path();

        AssertThat(std::filesystem::exists(old_path), Is().True());
        AssertThat(std::filesystem::exists(new_path), Is().False());

        AssertThat(f.can_move(), Is().True());
        f.move(new_path);

        AssertThat(std::filesystem::exists(old_path), Is().False());
        AssertThat(std::filesystem::exists(new_path), Is().True());
      });
    });

    describe("file() + file_stream + file_writer", [&tmp_path, &curr_path]() {
      it("can read written content [1]", []() {
        file<int> f;

        adiar::file_writer<int> fw(f);
        fw << 1 << 2 << 3;
        fw.detach();
        AssertThat(fw.attached(), Is().False());

        adiar::internal::file_stream<int, false> fs(f);
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

        adiar::file_writer<int> fw(f);
        fw << 42 << 21;
        fw.detach();
        AssertThat(fw.attached(), Is().False());

        adiar::internal::file_stream<int, false> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(42));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(21));
        AssertThat(fs.can_pull(), Is().False());
        fs.detach();
      });

      it("can read written content in reverse [1]", []() {
        file<int> f;

        adiar::file_writer<int> fw(f);
        fw << 1 << 2 << 3;
        fw.detach();
        AssertThat(fw.attached(), Is().False());

        adiar::internal::file_stream<int, true> fs(f);
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

        adiar::file_writer<int> fw(f);
        fw << 42 << 21;
        fw.detach();
        AssertThat(fw.attached(), Is().False());

        adiar::internal::file_stream<int, true> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(21));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(42));
        AssertThat(fs.can_pull(), Is().False());
        fs.detach();
      });

      it("can seek existing elements [forward]", []() {
        file<int> f;

        adiar::file_writer<int> fw(f);
        fw << 1 << 2 << 3 << 4 << 5 << 6;
        fw.detach();
        AssertThat(fw.attached(), Is().False());

        adiar::internal::file_stream<int> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(2), Is().EqualTo(2));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(2), Is().EqualTo(2));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(3), Is().EqualTo(3));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(6), Is().EqualTo(6));
        AssertThat(fs.can_pull(), Is().True());
      });

      it("can seek existing elements [reverse]", []() {
        file<int> f;

        adiar::file_writer<int> fw(f);
        fw << 6 << 5 << 4 << 3 << 2 << 1;
        fw.detach();
        AssertThat(fw.attached(), Is().False());

        adiar::internal::file_stream<int, true> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(2), Is().EqualTo(2));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(2), Is().EqualTo(2));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(3), Is().EqualTo(3));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(6), Is().EqualTo(6));
        AssertThat(fs.can_pull(), Is().True());
      });

      it("can seek non-existing element [forward]", []() {
        file<int> f;

        adiar::file_writer<int> fw(f);
        fw << 1 << 2 << 3 << 5 << 6;
        fw.detach();
        AssertThat(fw.attached(), Is().False());

        adiar::internal::file_stream<int> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(2), Is().EqualTo(2));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(4), Is().EqualTo(5));
        AssertThat(fs.can_pull(), Is().True());
      });

      it("can seek non-existing element [reverse]", []() {
        file<int> f;

        adiar::file_writer<int> fw(f);
        fw << 6 << 5 << 3 << 2 << 1;
        fw.detach();
        AssertThat(fw.attached(), Is().False());

        adiar::internal::file_stream<int, true> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(2), Is().EqualTo(2));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(4), Is().EqualTo(5));
        AssertThat(fs.can_pull(), Is().True());
      });

      it("can seek past end [forward]", []() {
        file<int> f;

        adiar::file_writer<int> fw(f);
        fw << 1 << 2 << 3 << 5 << 6;
        fw.detach();
        AssertThat(fw.attached(), Is().False());

        adiar::internal::file_stream<int> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(3), Is().EqualTo(3));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(8), Is().EqualTo(6));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(7), Is().EqualTo(6));
        AssertThat(fs.can_pull(), Is().True());
      });

      it("can seek past end [reverse]", []() {
        file<int> f;

        adiar::file_writer<int> fw(f);
        fw << 6 << 5 << 3 << 2 << 1;
        fw.detach();
        AssertThat(fw.attached(), Is().False());

        adiar::internal::file_stream<int, true> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(3), Is().EqualTo(3));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(8), Is().EqualTo(6));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(7), Is().EqualTo(6));
        AssertThat(fs.can_pull(), Is().True());
      });

      it("can sort written content", []() {
        file<int> f;

        adiar::file_writer<int> fw(f);
        fw << 42 << 2 << 32 << 21;
        fw.sort<std::less<>>();
        fw.detach();
        AssertThat(fw.attached(), Is().False());

        adiar::internal::file_stream<int> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(2));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(21));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(32));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(42));
        AssertThat(fs.can_pull(), Is().False());
        fs.detach();
      });

      it("can read from 'moved' file [/tmp/]", [&tmp_path]() {
        std::string new_path = tmp_path + "after-move-path.adiar";
        if (std::filesystem::exists(new_path)) {
          // Clean up after prior test run
          std::filesystem::remove(new_path);
        }

        file<int> f;

        adiar::file_writer<int> fw(f);
        fw << 12 << 9 << 1;
        fw.detach();

        AssertThat(f.can_move(), Is().True());
        f.move(new_path);

        adiar::internal::file_stream<int, false> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(12));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(9));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(1));
        AssertThat(fs.can_pull(), Is().False());
        fs.detach();
      });

      it("can read from 'moved' file [./]", [&curr_path]() {
        std::string new_path = curr_path + "after-move-path.adiar";
        if (std::filesystem::exists(new_path)) {
          // Clean up after prior test run
          std::filesystem::remove(new_path);
        }

        file<int> f;

        adiar::file_writer<int> fw(f);
        fw << 12 << 9 << 1;
        fw.detach();

        AssertThat(f.can_move(), Is().True());
        f.move(new_path);

        adiar::internal::file_stream<int, false> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(12));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(9));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(1));
        AssertThat(fs.can_pull(), Is().False());
        fs.detach();
      });

      it("can read from 'moved' file in reverse [/tmp/]", [&tmp_path]() {
        std::string new_path = tmp_path + "after-move-path.adiar";
        if (std::filesystem::exists(new_path)) {
          // Clean up after prior test run
          std::filesystem::remove(new_path);
        }

        file<int> f;

        adiar::file_writer<int> fw(f);
        fw << 8 << 9 << 4 << 2;
        fw.detach();

        AssertThat(f.can_move(), Is().True());
        f.move(new_path);

        adiar::internal::file_stream<int, true> fs(f);
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

      it("can read from 'moved' file in reverse [./]", [&curr_path]() {
        std::string new_path = curr_path + "after-move-path.adiar";
        if (std::filesystem::exists(new_path)) {
          // Clean up after prior test run
          std::filesystem::remove(new_path);
        }

        file<int> f;

        adiar::file_writer<int> fw(f);
        fw << 8 << 9 << 4 << 2;
        fw.detach();

        AssertThat(f.can_move(), Is().True());
        f.move(new_path);

        adiar::internal::file_stream<int, true> fs(f);
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

    describe("file(path)", []() {
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

        AssertThat(f.can_move(), Is().False());
        AssertThrows(std::runtime_error, f.move(new_path));
      });

      // Clean up for above tests
      if (std::filesystem::exists(path)) {
        std::filesystem::remove(path);
      }

      {
        file<int> f;

        adiar::file_writer<int> fw(f);
        fw << 0 << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8 << 9;
        fw.detach();

        f.move(path);
        f.make_persistent();
      }

      it("can reopen a non-empty persisted file", [&path]() {
        file<int> f(path);
      });

      it("has expected size after reopening a persisted non-empty file", [&path]() {
        file<int> f(path);

        AssertThat(f.empty(), Is().False());
        AssertThat(f.size(), Is().EqualTo(10u));
      });

      it("can read content with a stream", [&path]() {
        file<int> f(path);
        adiar::internal::file_stream<int> fs(f);

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
      });

      it("can read content in reverse with a stream", [&path]() {
        file<int> f(path);
        adiar::internal::file_stream<int, true> fs(f);

        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(9));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(8));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(7));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(6));
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
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(0));
        AssertThat(fs.can_pull(), Is().False());
      });

      it("cannot reattach a writer to a persisted file", [&path]() {
        file<int> f(path);
        adiar::file_writer<int> fw;
        AssertThrows(std::runtime_error, fw.attach(f));
      });

      // Clean up for above tests
      if (std::filesystem::exists(path)) {
        std::filesystem::remove(path);
      }
    });

    describe("file.sort(const pred_t&)", []() {
      it("can sort non-existing file", []() {
        file<int> f;
        AssertThat(f.exists(), Is().False());

        f.sort<std::less<int>>();
        AssertThat(f.exists(), Is().False());

        adiar::internal::file_stream<int> fs(f);
        AssertThat(fs.can_pull(), Is().False());
        fs.detach();
      });

      it("can sort existing empty file", []() {
        file<int> f;
        f.touch();
        AssertThat(f.exists(), Is().True());

        f.sort<std::less<int>>();
        AssertThat(f.exists(), Is().True());

        adiar::internal::file_stream<int> fs(f);
        AssertThat(fs.can_pull(), Is().False());
        fs.detach();
      });

      it("can sort non-empty file [1]", []() {
        file<int> f;

        adiar::file_writer<int> fw(f);
        fw << 8 << 9 << 4 << 2;
        fw.detach();

        f.sort<std::less<int>>();

        adiar::internal::file_stream<int> fs(f);
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

        adiar::file_writer<int> fw(f);
        fw << 42 << -1 << 8 << 21 << 8 << 3;
        fw.detach();

        f.sort<std::less<int>>();

        adiar::internal::file_stream<int> fs(f);
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

      it("cannot sort a persisted empty file", []() {
        std::string path;
        { // Scope to destruct 'f' early
          file<int> f;
          f.touch();

          path = f.path();
          f.make_persistent();
          AssertThat(std::filesystem::exists(path), Is().True());

          AssertThrows(std::runtime_error, f.sort<std::less<int>>());
        }
        AssertThat(std::filesystem::exists(path), Is().True());

        // Clean up for this test
        std::filesystem::remove(path);
      });

      it("cannot sort a persisted non-empty file", []() {
        std::string path;
        { // Scope to destruct 'f' early
          file<int> f;
          f.touch();

          adiar::file_writer<int> fw(f);
          fw << -1 << 8 << 3;
          fw.detach();

          path = f.path();
          f.make_persistent();
          AssertThat(std::filesystem::exists(path), Is().True());

          AssertThrows(std::runtime_error, f.sort<std::less<int>>());
        }
        AssertThat(std::filesystem::exists(path), Is().True());

        { // Check is not sorted
          file<int> f(path);
          adiar::internal::file_stream<int> fs(f);
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(-1));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(8));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(3));
          AssertThat(fs.can_pull(), Is().False());
          fs.detach();
        }

        // Clean up for this test
        std::filesystem::remove(path);
      });
    });

    describe("file::copy(const file&)", []() {
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
        adiar::file_writer<int> fw(f1);

        fw << 21 << 42 << 21;
        fw.detach();
        AssertThat(f1.exists(), Is().True());
        AssertThat(f1.size(), Is().EqualTo(3u));

        file<int> f2 = file<int>::copy(f1);

        AssertThat(f2.exists(), Is().True());
        AssertThat(f2.size(), Is().EqualTo(3u));
        AssertThat(f2.path(), Is().Not().EqualTo(f1.path()));

        adiar::internal::file_stream<int> fs(f2);
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
        adiar::file_writer<int> fw(f1);

        fw << 0 << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8 << 9;
        fw.detach();
        AssertThat(f1.exists(), Is().True());
        AssertThat(f1.size(), Is().EqualTo(10u));

        file<int> f2 = file<int>::copy(f1);

        AssertThat(f2.exists(), Is().True());
        AssertThat(f2.size(), Is().EqualTo(10u));
        AssertThat(f2.path(), Is().Not().EqualTo(f1.path()));

        adiar::internal::file_stream<int> fs(f2);
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
        adiar::file_writer<int> fw(f1);
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
          adiar::file_writer<int> fw(f1);
          fw << 0 << 1 << 2 << 3 << 4;
          fw.detach();
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
    });
  });
 });
