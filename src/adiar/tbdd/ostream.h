#ifndef ADIAR_TBDD_OSTREAM_H
#define ADIAR_TBDD_OSTREAM_H

#include <fstream>
#include <ostream>
#include <string>
#include <type_traits>

#include <adiar/exception.h>
#include <adiar/internal/block_size.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Output writer for an *Extended Resolution Proof*, taking care of
  ///        formatting the output.
  ///
  /// \tparam Integer Type (signed or unsigned) for all indexing structures.
  //////////////////////////////////////////////////////////////////////////////
  template<typename Integer = int>
  class tbdd_ostream
  {
    // TODO: There are some input validity checks inside of 'adiar_assert(...)'.
    //       Should these be made into proper if-throw statements?

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Signed integer type, used for variable and clause identifiers.
    ////////////////////////////////////////////////////////////////////////////
    using int_type  = typename std::make_signed<Integer>::type;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Unsigned integer type
    ////////////////////////////////////////////////////////////////////////////
    using uint_type = typename std::make_unsigned<Integer>::type;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Supported proof types of the proof.
    ///
    /// \internal This essentially can be thought of as a boolean value; if true
    ///           then it is an LRAT proof and hence additional steps needs to
    ///           be run.
    ////////////////////////////////////////////////////////////////////////////
    enum type : bool
    {
      /** List of proven clauses. Use a tool like 'drat-trim' to expand this
          into the LRAT format which can then be checked. */
      DRAT = false,
      /** DRAT with additional 'antecedent hints' which immediately can be
          checked. Note, this file is considerably larger than the DRAT
          version. */
      LRAT = true
    };

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Formatting of proof.
    ///
    /// \internal Similar to `adiar::tbdd_ostream::type`, this can be treated as
    ///           a Boolean. If, it is non-zero then it is some binary format
    ///           and hence ' ' and '\n' should be omitted.
    ////////////////////////////////////////////////////////////////////////////
    enum mode : char
    {
      /** Output each clause in decimal numbers and include line endings. This
          is recommended for human-readable outputs. */
      ASCII           = 0,
      /** Output all numbers using a fixed number of bytes per character. */
      Binary_Fixed    = 1,
      /** Output all numbers in with a variable byte encoding. */
      Binary_Variadic = 2,
    };

  private:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief End marker for a clause or antecedent hints in RAT proofs.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr int_type end = 0;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Return value for a non-existent variable or clause.
    ///
    /// \details When dealing with a *DRAT* proof there are no antecedent hints
    ///          of the clauses needed to proof a new inferred clause. Hence,
    ///          this dummy value is returned.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr int_type none = end;

  private:
    /*
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Class to hold onto an file output stream and its buffer.
    ////////////////////////////////////////////////////////////////////////////
    class __ofstream_raii
    {
    private:
      //////////////////////////////////////////////////////////////////////////
      /// \brief Elements in output buffer.
      //////////////////////////////////////////////////////////////////////////
      using buffer_elem = char;

      //////////////////////////////////////////////////////////////////////////
      /// \brief   Type of TPIE array for output buffer.
      ///
      /// \details The default has (possibly) a much smaller buffer than what we
      ///          are using for I/O-efficiency. Furthermore, a default *std*
      ///          buffer would not be accounted for by TPIE's memory manager.
      //////////////////////////////////////////////////////////////////////////
      // TODO: wrap `tpie::array` inside of <internal/data_structures/array.h>
      using buffer_type = tpie::array<buffer_elem>;

      //////////////////////////////////////////////////////////////////////////
      /// \brief Buffer to replace the default from `fs` below
      //////////////////////////////////////////////////////////////////////////
      buffer_type _buffer;

      //////////////////////////////////////////////////////////////////////////
      /// \brief Output file stream.
      //////////////////////////////////////////////////////////////////////////
      std::ofstream _os;

    private:
      //////////////////////////////////////////////////////////////////////////
      /// \brief Derives the `std::ios` to open the `std::ofstream` with.
      //////////////////////////////////////////////////////////////////////////
      static constexpr std::ostream::openmode open_mode(const mode &md)
      {
        constexpr std::ios ascii_mode  = std::ios::out | std::ios::trunc;
        constexpr std::ios binary_mode = ascii_mode | std::ios::binary;

        return mode//::ASCII ? ascii_mode : binary_mode;
      }

    public:
      //////////////////////////////////////////////////////////////////////////
      /// \brief The default-constructor is deleted: this class cannot be
      ///        instantiated without any arguments
      //////////////////////////////////////////////////////////////////////////
      __ofstream_raii()                      = delete;

      //////////////////////////////////////////////////////////////////////////
      /// \brief The copy-constructor is deleted: this class is not copyable.
      //////////////////////////////////////////////////////////////////////////
      __ofstream_raii(const __ostream_raii&) = delete;

      //////////////////////////////////////////////////////////////////////////
      /// \brief Move construction
      //////////////////////////////////////////////////////////////////////////
      __ofstream_raii(__ostream_raii&&)      = default;

      //////////////////////////////////////////////////////////////////////////
      /// \brief Construct from a file path.
      ///
      /// \tparam FsPath Anything `std::ofstream` can use as a path.
      //////////////////////////////////////////////////////////////////////////
      template <typename FsPath>
      __ofstream_raii(const FsPath path, const mode &md)
        : _buffer(buffer_type::memory_fits(internal::get_block_size()))
        , _os(path, open_mode(md))
      {
        // Use the larger buffer instead of the default.
        _os.rdbuf()->pubsetbuf(_buffer.get(), _buffer.size())
      }

      //////////////////////////////////////////////////////////////////////////
      /// \brief Destructor
      //////////////////////////////////////////////////////////////////////////
      ~__ofstream_raii()
      {
        // Flush the output stream before closing it.
        _os << std::flush;
        _os.close();
      }

    public:
      //////////////////////////////////////////////////////////////////////////
      /// \brief Obtain a reference to the output stream.
      //////////////////////////////////////////////////////////////////////////
      std::ostream& get() const
      {
        return _os;
      }
    };
    */

  private:
    // \brief Type-erased pointer to object with streams, buffers etc. such that
    //        the destructor is resolved at run-time.
    //
    // unique_ptr<void> _os_raii = nullptr;

    /// \brief Output Stream for the proof.
    std::ostream &_os;

    /// \brief Starting position (for `size()`)
    const std::ostream::pos_type _start_pos = _os.tellp();

    /// \brief Proof Type.
    const type _type;

    /// \brief Proof Mode.
    const mode _mode;

    /// \brief Number of input variables.
    const int_type _cnf_vars;

    /// \brief Number of input clauses.
    const int_type _cnf_clauses;

    /// \brief Number of additional extension variables in the proof.
    int_type _proof_vars    = 0;

    /// \brief Number of clauses in proof.
    int_type _proof_clauses = 0;

    /// \brief Number of non-whitespace characters pushed to the output.
    uint_type _size         = 0u;

  private:
    //////////////////////////////////////////////////////////////////////////////
    /// \brief Limit on `_proof_vars` without overflowing.
    //////////////////////////////////////////////////////////////////////////////
    int_type max_var() const
    {
      const int_type max_value = std::numeric_limits<int_type>::max();
      return (max_value - _cnf_vars) - 1;
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Limit on `_proof_clauses` without overflowing.
    //////////////////////////////////////////////////////////////////////////////
    int_type max_clause() const
    {
      const int_type max_value = std::numeric_limits<int_type>::max();
      return (max_value - _cnf_clauses) - 1;
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Low-level constructor, leaving you to parse the input CNF and to
    ///        open, set-up, and close the output stream.
    ///
    /// \param vars    The number of input variables in the CNF formula.
    ///
    /// \param clauses The number of input clauses in the CNF formula.
    ///
    /// \param os      Reference to an output stream whereto print the proof.
    ///                This object must outlive the `tbdd_ostream` object.
    ///
    /// \param tp      If `type::LRAT` (i.e. if `true`), include clause indices
    ///                and antecedent hints in the output.
    ///
    /// \param md      If `mode::ASCII` (i.e. if `true`), add spaces and
    ///                newlines within the proof.
    ///
    /// \throws invalid_argument If `vars` or `clauses` is a negative number.
    ////////////////////////////////////////////////////////////////////////////
    tbdd_ostream(int_type vars,
                 int_type clauses,
                 std::ostream &os = std::cout,
                 type tp          = type::LRAT,
                 mode md          = mode::ASCII)
      : _os(os), _type(tp), _mode(md), _cnf_vars(vars), _cnf_clauses(clauses)
    {
      if (vars < 0) {
        throw invalid_argument("Number of variables cannot be negative");
      }
      if (clauses < 0) {
        throw invalid_argument("Number of clauses cannot be negative");
      }
    }

    // TODO: constructor with 'vars' and 'clauses' replaced by a std::istream
    //       from which to read the header.

    // TODO: constructor overloads with *FILE, std::tring and other path-like
    //       objects. Use them to open the output stream ourselves with
    //       `__ofstream_raii`.

    ////////////////////////////////////////////////////////////////////////////
    /// Disallow default construction.
    ////////////////////////////////////////////////////////////////////////////
    tbdd_ostream()                                 = delete;

    ////////////////////////////////////////////////////////////////////////////
    /// Disallow copy-construction.
    ////////////////////////////////////////////////////////////////////////////
    tbdd_ostream(const tbdd_ostream &)             = delete;

    ////////////////////////////////////////////////////////////////////////////
    /// Disallow copy-assignment.
    ////////////////////////////////////////////////////////////////////////////
    tbdd_ostream& operator =(const tbdd_ostream &) = delete;

    ////////////////////////////////////////////////////////////////////////////
    /// Move-construction.
    ////////////////////////////////////////////////////////////////////////////
    tbdd_ostream(tbdd_ostream &&pf)                = default;

    ////////////////////////////////////////////////////////////////////////////
    /// Move-assignment.
    ////////////////////////////////////////////////////////////////////////////
    tbdd_ostream& operator =(tbdd_ostream &&)      = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Destructor.
    ////////////////////////////////////////////////////////////////////////////
    ~tbdd_ostream()                                = default;

  private:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the current clause id.
    ////////////////////////////////////////////////////////////////////////////
    inline int current_clause() const
    {
      return _cnf_clauses + (_proof_clauses);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain a fresh clause id.
    ////////////////////////////////////////////////////////////////////////////
    inline int next_clause()
    {
      if (this->max_clause() <= _proof_clauses) {
        throw out_of_range("Overflow of Clause Index");
      }
      _proof_clauses += 1;
      return current_clause();
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of input variables in CNF.
    ////////////////////////////////////////////////////////////////////////////
    int vars_cnf() const
    { return _cnf_vars; }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of clauses in input CNF.
    ////////////////////////////////////////////////////////////////////////////
    int clauses_cnf() const
    { return _cnf_clauses; }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of extension variables in outputted proof.
    ////////////////////////////////////////////////////////////////////////////
    int vars_proof() const
    { return _proof_vars; }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of clauses in outputted proof.
    ////////////////////////////////////////////////////////////////////////////
    int clauses_proof() const
    { return _proof_clauses; }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Total number of variables in CNF and extension variables in
    ///        outputted proof.
    ////////////////////////////////////////////////////////////////////////////
    int vars() const
    { return vars_cnf() + vars_proof(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Total number of clauses in input CNF and outputted proof.
    ////////////////////////////////////////////////////////////////////////////
    int clauses() const
    { return clauses_cnf() + clauses_proof(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain a fresh extension variable.
    ////////////////////////////////////////////////////////////////////////////
    inline int next_var()
    {
      if (this->max_var() <= _proof_vars) {
        throw out_of_range("Overflow of Proof Extension Variables");
      }
      return _cnf_vars + (++_proof_vars);
    }

  private:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Value for 'end-of-clause'.
    ////////////////////////////////////////////////////////////////////////////
    inline char eoc() const
    {
      return this->_mode == mode::ASCII ? '0' : 0;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Add 'end-of-clause'
    ///
    /// \tpama eol Whether this also marks the end of the line.
    ////////////////////////////////////////////////////////////////////////////
    template<bool eol>
    inline void write_eoc()
    {
      _os << this->eoc();
      if (this->_mode == mode::ASCII) {
        constexpr char c = eol ? '\n' : ' ';
        _os << c;
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Add 'delete' clause identifier
    ////////////////////////////////////////////////////////////////////////////
    inline void write_del()
    {
      _os << 'd';
      if (this->_mode == mode::ASCII) { _os << ' '; }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Add a single signed integral, i.e. literal or clause id to the
    ///        output.
    ////////////////////////////////////////////////////////////////////////////
    template<bool inc_size = true>
    inline void write_int(const int_type id)
    {
      adiar_assert(id != 0 && id != none);

      if constexpr (inc_size) { _size += 1; }

      switch (this->_mode) {
      case mode::ASCII: {
        // In ASCII mode, every literal/clause index is succeeded by a space.
        // This does not add trailing whitespaces on each line, since there is a
        // '0\n' at the end (see `write_eoc<true>()`).
        _os << id << ' ';
        break;
      }
      case mode::Binary_Fixed: {
        // Relies on `_os << ...` always uses the same number of bytes for every
        // value pushed into it.
        _os << id;
        break;
      }
      case mode::Binary_Variadic: {
        // The `static_assert(...)` statements below double-checks that
        // `std::ostream::char_type` is a `char` (8 bits). As such, this
        // (probably) works for `wchar` too.
        using ostream_char = typename std::make_unsigned<std::ostream::char_type>::type;

        constexpr uint_type ostream_bits = 8*sizeof(ostream_char);
        static_assert(ostream_bits == 8/*bits*/);

        // Use the most significant bit to denote the number continues
        constexpr ostream_char top_bit = 1 << (ostream_bits-1);
        static_assert(top_bit == 128);

        // Encode negative numbers as Two's Complement independent of hardware
        uint_type unsigned_id = id < 0 ? 2*(-id)+1 : 2*id;

        // Take the next least-significant bits of 'unsigned_id'
        while (top_bit <= unsigned_id) {
          constexpr uint_type mask = top_bit-1;
          static_assert(mask == 127);

          _os << (top_bit | static_cast<ostream_char>(mask & unsigned_id));
          unsigned_id >>= ostream_bits-1;
        }
        _os << unsigned_id;
        break;
      }
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Output an entire clause.
    ////////////////////////////////////////////////////////////////////////////
    template<typename Iterator>
    void write_clause(Iterator begin, Iterator end)
    {
      for (; begin != end; ++begin) {
        const int_type lit = *begin;
        adiar_assert(-vars() <= lit && lit <= vars());
        this->write_int(lit);
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Output an entire list of antecedent hints.
    ////////////////////////////////////////////////////////////////////////////
    template<typename Iterator>
    void write_hints(Iterator begin, Iterator end)
    {
      for (; begin != end; ++begin) {
        const int_type idx = *begin;
        adiar_assert(0 < idx && idx <= clauses());
        this->write_int(idx);
      }
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Size of proof (total length of all clauses and hints).
    ////////////////////////////////////////////////////////////////////////////
    size_t size() const
    {
      // TODO: Number of tokens?
      return _size;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief  Size of proof (in bytes).
    ///
    /// \remark This is only accurate, if the same `std::ostream` has not been
    ///         used concurrently by other processes.
    ////////////////////////////////////////////////////////////////////////////
    size_t file_size() const
    {
      // TODO: Update a local `_file_size` member variable in `this->write_...()`
      //       helper method functions.
      return _os.tellp() - _start_pos;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Output a RUP-inferred clause together with its hints.
    ///
    /// \returns Clause identifier of the newly added clause (if it exists).
    ////////////////////////////////////////////////////////////////////////////
    template<typename ClauseIterator, typename HintsIterator>
    int_type write_rup(ClauseIterator clause_begin, ClauseIterator clause_end,
                       HintsIterator  hints_begin,  HintsIterator  hints_end)
    {
      // Format
      // - DRAT: '<clause> 0\n'
      // - LRAT: '<id> <clause> 0 <hints> 0', <hints> = <antecedents>

      const int_type clause_id = this->next_clause();

      // '<id>'
      if (this->_type == type::LRAT) {
        this->write_int<false>(clause_id);
      }

      // '<clause>'
      this->write_clause(clause_begin, clause_end);

      if (this->_type == type::LRAT) {
        // '0 <hints>'
        this->write_eoc<false>();
        this->write_hints(hints_begin, hints_end);
      }

      // '0\n'
      this->write_eoc<true>();

      // Return clause id (if it exists in the output)
      return this->_type == type::DRAT ? none : clause_id;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Output a RUP-inferred clause (without any hints).
    ///
    /// \details This overload only works while outputting *DRAT* proofs, since
    ///          they do not require any hints.
    ///
    /// \returns The clause identifier 'none', since a DRAT proof does not
    ///          support index hints.
    ////////////////////////////////////////////////////////////////////////////
    template<typename ClauseIterator>
    int_type write_rup(ClauseIterator begin, ClauseIterator end)
    {
      if (this->_type != type::DRAT) {
        throw invalid_argument("RUP without hints is only supported in DRAT mode");
      }
      return write_rup(begin, end, end, end);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Skip over a RUP clause.
    ///
    /// \details Use this to skip over some clause identifiers.
    ///
    /// \returns Skipped clause identifier (if it could have existed).
    ////////////////////////////////////////////////////////////////////////////
    int_type skip_rup()
    {
      const int_type clause_id = this->next_clause();
      return this->_type == type::DRAT ? none : clause_id;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Output a RAT-inferred clause with its hints.
    ///
    /// \details The clause literals are given by `clause_begin` and
    ///          `clause_end`. The first of these is expected to be the pivot.
    ///          The `hints_begin` and `hints_end` are iterators over iterators.
    ///
    /// \returns Clause identifier of the newly added clause (if it exists).
    ////////////////////////////////////////////////////////////////////////////
    template<typename ClauseIterator, typename PivotsIterator>
    int_type write_rat(ClauseIterator clause_begin, ClauseIterator clause_end,
                       PivotsIterator pivots_begin, PivotsIterator pivots_end)
    {
      // Format
      // - DRAT: '<clause> 0\n'
      // - LRAT: '<id> <clause> 0 <hints> 0\n', <hints> = (<pivot> <antecedents>*)*

      const int_type clause_id = this->next_clause();

      // '<id>'
      if (this->_type == type::LRAT) {
        this->write_int<false>(clause_id);
      }

      // '<clause>'
      adiar_assert(clause_begin != clause_end);
      this->write_clause(clause_begin, clause_end);

      if (this->_type == type::LRAT) {
        // '0 (-<pivot clause_id> <hints>)*'
        this->write_eoc<false>();

        for (; pivots_begin != pivots_end; ++pivots_begin) {
          // '-<pivot clause_id>'
          const int_type pivot_clause = pivots_begin->pivot_clause;
          adiar_assert(0 < pivot_clause && pivot_clause < clause_id);

          this->write_int(-pivot_clause);

          // '<hints>'
          this->write_hints(pivots_begin->hints.begin(), pivots_begin->hints.end());
        }
      }

      // '0\n'
      this->write_eoc<true>();

      // Return clause id (if it exists in the output)
      return this->_type == type::DRAT ? none : clause_id;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Output a RAT-inferred clause.
    ///
    /// \returns The clause identifier 'none', since a DRAT proof does not
    ///          support index hints.
    ////////////////////////////////////////////////////////////////////////////
    template<typename ClauseIterator>
    int_type write_rat(ClauseIterator begin, ClauseIterator end)
    {
      // Without hints, this is formatted similarly to a RUP step. Hence, we may
      // as well just reuse that piece of code.
      return write_rup(begin, end);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Skip over a RUP clause.
    ///
    /// \details Use this to skip over some clause identifiers.
    ///
    /// \returns Skipped clause identifier (if it could have existed).
    ////////////////////////////////////////////////////////////////////////////
    int_type skip_rat()
    {
      // This is equivalent to `skip_rup()`, so reuse its implementation.
      return skip_rup();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Output a clause(s) delete statement.
    ///
    /// \details When in *DRAT* mode, the iterators are expected to provide
    ///          literals to write a previous clause anew. For *LRAT* proofs,
    ///          the iterators are expected to provide the to-be removed clause
    ///          identifiers.
    ////////////////////////////////////////////////////////////////////////////
    template<typename Iterator>
    void write_del(Iterator begin, Iterator end)
    {
      // <id>
      if (this->_type == type::LRAT) {
        this->write_int<false>(this->current_clause());
      }

      // 'd'
      this->write_del();

      // '<literals>/<clause ids>'
      for (; begin != end; ++begin) {
        const int_type x = *begin;
        adiar_assert(this->_type != type::DRAT || (-vars() <= x && x <= vars()));
        adiar_assert(this->_type != type::LRAT || (x <= clauses() && x != none));
        this->write_int(x);
      }

      // '0\n'
      this->write_eoc<true>();
    }
  };
}

#endif // ADIAR_TBDD_OSTREAM_H
