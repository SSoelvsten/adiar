#ifndef ADIAR_EXEC_POLICY_H
#define ADIAR_EXEC_POLICY_H

#include <algorithm>
#include <limits>
#include <type_traits>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \addtogroup module__adiar
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Settings to dictate the execution of Adiar's algorithms.
  ///
  /// Adiar's algorithms work very differently from other BDD implementations.
  /// Hence, it makes use of multiple novel techniques to make it competitive
  /// across the entire spectrum of BDD shapes and sizes. These can be turned
  /// on/off or tweaked by changing these settings.
  ///
  /// Most likely, you would want to apply all techniques (in a safe way) and so
  /// you do not need to think about changing any of these.
  ///
  /// For example, you can fix the `bdd_exists` to only use \em internal memory
  /// and the \em Nested \em Sweeping framework as follows:
  /// ```cpp
  /// bdd_exists(exec_policy::memory::Internal & exec_policy::quantify::Nested,
  ///            f,
  ///            vars.rbegin(), vars.rend());
  /// ```
  //////////////////////////////////////////////////////////////////////////////
  class exec_policy
  {
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Whether Adiar should exclusively use random access or priority
    ///          queues or automatically pick either way based on the width of
    ///          inputs.
    ///
    /// \details Adiar’s Algorithms delay recursion through use of auxiliary
    ///          data structures. These auxiliary data structures are redundant
    ///          when the width of one or more inputs fits in the internal
    ///          memory. In these cases it is much faster to load in the entire
    ///          level of a diagram and then use random access.
    ///
    /// \warning Using `Random_Access` may lead to crashes if all inputs are too
    ///          wide or input to random access is not canonical!
    ///
    /// \see bdd_apply zdd_binop
    ////////////////////////////////////////////////////////////////////////////
    enum class access : char
    {
      /** Pick \em Random Access when an input is narrow enough. */
      Auto,
      /** Always use Random Access */
      Random_Access,
      /** Always use Priority Queues */
      Priority_Queue
    };

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Whether Adiar should exclusively use internal or external
    ///          memory or automatically pick either type based on size of
    ///          input.
    ///
    /// \details Adiar’s Algorithms delay recursion through use of auxiliary
    ///          data structures. These auxiliary data structures can be
    ///          optimised for internal memory, and so have a high performance
    ///          on very small instances, or they can be designed for external
    ///          memory such that they can handle decision diagrams much larger
    ///          than the available memory.
    ///
    /// \note    For more details, please read "Predicting Memory Demands of BDD
    ///          Operations using Maximum Graph Cuts" ATVA 2023.
    ///
    /// \warning Using `Internal` may lead to crashes if an input or output is
    ///          too large!
    ////////////////////////////////////////////////////////////////////////////
    enum class memory : char
    {
      /** Pick \em internal memory as long as it is safe to do so. */
      Auto,
      /** Always use \em internal memory */
      Internal,
      /** Always use \em external memory */
      External
    };

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Strategies and settings for Adiar to use in quantify/project
    ///          algorithms.
    ///
    /// \details Adiar’s supports multiple approaches to compute the
    ///          quantification of multiple variables. While `Auto`
    ///          heuristically uses one or more of the three options, one can
    ///          choose to run the approach of particular interest
    ///
    /// \see bdd_exists bdd_forall zdd_project
    ////////////////////////////////////////////////////////////////////////////
    class quantify
    {
    public:
      //////////////////////////////////////////////////////////////////////////
      /// \brief
      //////////////////////////////////////////////////////////////////////////
      enum algorithm : char
      {
        /** Use the \em nested \em sweeping framework (recommended). */
        Nested,
        /** Quantify each variable independently. */
        Singleton
      };

      //////////////////////////////////////////////////////////////////////////
      /// \brief Multiplicative value for the maximal growth of a BDD may during
      ///        repeated transpostion before switching to Nested Sweeping.
      ///
      /// \details A value of `1.0` is equivalent to the original input size.
      ///          Larger Values reflect an acceptable increase in size. A value
      ///          between `0.0` and `1.0` requires the BDD to shrink to
      ///          continue repeating transposition.
      ///
      /// \details A negative value will result in undefined behaviour.
      //////////////////////////////////////////////////////////////////////////
      class transposition_growth
      {
      private:
        static constexpr float min_val = 0.0f;

      public:
        /// \brief Minimal value
        static constexpr transposition_growth
        min()
        {
          return min_val;
        }

        /// \brief Maximal value
        static constexpr transposition_growth
        max()
        {
          return std::numeric_limits<float>::max();
        }

      private:
        float _value;

        /// \brief Convert into a `signed char`
        static constexpr float
        from_float(float f)
        {
          // Truncate `f` to be in the interval [0.0f, ...)
          return f < min_val ? min_val : f;
        }

      public:
        constexpr transposition_growth()
          : _value(1.5f)
        {}

        /// \brief Wrap a `float`.
        constexpr transposition_growth(float value)
          : _value(from_float(value))
        {}

        /// \brief Reobtain the wrapped `float`
        operator float() const
        {
          return this->_value;
        }
      };

      //////////////////////////////////////////////////////////////////////////
      /// \brief Maximum number of repeated transpositions before switching to
      ///        nested sweeping.
      //////////////////////////////////////////////////////////////////////////
      class transposition_max
      {
      public:
        /// \brief Minimal value (equivalent to disabling repeated transpositions).
        static constexpr transposition_max
        min()
        {
          return std::numeric_limits<unsigned char>::min();
        }

        /// \brief Maximal value (equivalent to using the built-in heuristics
        ///        based on the graph's meta information).
        static constexpr transposition_max
        max()
        {
          return std::numeric_limits<unsigned char>::max();
        }

      private:
        unsigned char _value;

      public:
        /// \brief Default value construction.
        constexpr transposition_max()
          : _value(std::numeric_limits<unsigned char>::max())
        {}

        /// \brief Wrap an `unsigned char`
        constexpr transposition_max(unsigned char value)
          : _value(value)
        {}

        /// \brief Reobtain the wrapped `unsigned char` value
        operator unsigned char() const
        {
          return this->_value;
        }
      };
    };

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Settings for Nested Sweeping framework
    ////////////////////////////////////////////////////////////////////////////
    class nested
    {
    public:
      //////////////////////////////////////////////////////////////////////////
      /// \brief Chosen epsilon value for Nested Sweeping to start skipping the
      ///        expensive node merging computations (if applicable).
      ///
      /// \details During Nested Sweeping, if a later sweep will touch a level
      ///          again, one can postpone merging duplicate nodes. This does
      ///          no apply to the last Reduce sweep up. So, the final diagram
      ///          is still canonical.
      ///
      /// \details Setting this value to *1.0* is equivalent to always using the
      ///          fast reduce. On the other hand, setting it to a negative
      ///          value turns this optimisation of.
      ///
      /// \details To safe on space, the value is stored in a char. Hence, this
      ///          option can only be set in increments of ~1%.
      //////////////////////////////////////////////////////////////////////////
      class fast_reduce
      {
      public:
        /// \brief Minimal value
        static constexpr fast_reduce
        min()
        {
          return -1.0f;
        }

        /// \brief Maximal value
        static constexpr fast_reduce
        max()
        {
          return 1.0f;
        }

      private:
        signed char _value;

        /// \brief Number of ticks in the interval [0,1] and [-1,0]
        static constexpr float ticks = std::numeric_limits<signed char>::max();

        /// \brief Convert into a `signed char`
        static constexpr signed char
        from_float(float f)
        {
          // Truncate `f` to be in the interval [-1,1]
          if (1.0f < f) {
            f = 1.0f;
          } else if (f < -1.0f) {
            f = -1.0f;
          }

          // Multiply by 'ticks' to extend the interval *[-1,1]* into
          // *[-ticks,ticks]*. Then, convert to an integral value. This
          // introduces a loss of precision.
          return static_cast<signed char>(f * ticks);
        }

      public:
        /// \brief Default value construction.
        constexpr fast_reduce()
          : _value(0.05f)
        {}

        /// \brief Conversion constructor from `float`.
        constexpr fast_reduce(float f)
          : _value{ from_float(f) }
        {}

        /// \brief Reobtain this value as a `float`.
        operator float() const
        {
          return static_cast<double>(this->_value) / ticks;
        }

        bool
        operator==(const fast_reduce& other) const
        {
          return this->_value == other._value;
        }
      };
    };

  private:
    // TODO: Merge all enums into a single 64 bit integer to safe on space?

    ////////////////////////////////////////////////////////////////////////////
    /// \brief `access` (default `Auto`).
    ////////////////////////////////////////////////////////////////////////////
    access _access = access::Auto;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief `memory` (default `Auto`).
    ////////////////////////////////////////////////////////////////////////////
    memory _memory = memory::Auto;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief `quantify` algorithm (default `Nested`).
    ////////////////////////////////////////////////////////////////////////////
    quantify::algorithm _quantify__algorithm = quantify::Nested;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Maximal growth during repeated transposition of `quantify`.
    ////////////////////////////////////////////////////////////////////////////
    quantify::transposition_growth _quantify__transposition_growth /*default*/;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Maximal number of repeated transpositions in `quantify`.
    ////////////////////////////////////////////////////////////////////////////
    quantify::transposition_max _quantify__transposition_max /*default*/;
    ;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Chosen epsilon value for Nested Sweeping to start skipping the
    ///        expensive node merging computations (if applicable).
    ////////////////////////////////////////////////////////////////////////////
    nested::fast_reduce _nested__fast_reduce /*default*/;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Default constructor with all options set to their default value.
    ////////////////////////////////////////////////////////////////////////////
    exec_policy() = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Copy constructor.
    ////////////////////////////////////////////////////////////////////////////
    exec_policy(const exec_policy&) = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Move constructor.
    ////////////////////////////////////////////////////////////////////////////
    exec_policy(exec_policy&&) = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Conversion construction from `access` enum.
    ////////////////////////////////////////////////////////////////////////////
    exec_policy(const access& am)
      : _access(am)
    {}

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Conversion construction from `memory` enum.
    ////////////////////////////////////////////////////////////////////////////
    exec_policy(const memory& mm)
      : _memory(mm)
    {}

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Conversion construction from `quantify` enum.
    ////////////////////////////////////////////////////////////////////////////
    exec_policy(const quantify::algorithm& qa)
      : _quantify__algorithm(qa)
    {}

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Conversion construction from `quantify` enum.
    ////////////////////////////////////////////////////////////////////////////
    exec_policy(const quantify::transposition_growth& tg)
      : _quantify__transposition_growth(tg)
    {}

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Conversion construction from `quantify` enum.
    ////////////////////////////////////////////////////////////////////////////
    exec_policy(const quantify::transposition_max& tm)
      : _quantify__transposition_max(tm)
    {}

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Conversion construction from `quantify` enum.
    ////////////////////////////////////////////////////////////////////////////
    exec_policy(const nested::fast_reduce& fr)
      : _nested__fast_reduce(fr)
    {}

    // TODO: constructor with defaults for a specific 'version number'?

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Copy assignment
    ////////////////////////////////////////////////////////////////////////////
    exec_policy&
    operator=(const exec_policy&) = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ////////////////////////////////////////////////////////////////////////////
    exec_policy&
    operator=(exec_policy&&) = default;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain a value
    ////////////////////////////////////////////////////////////////////////////
    template <typename T>
    const T&
    get() const;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Check for equality of settings.
    ////////////////////////////////////////////////////////////////////////////
    bool
    operator==(const exec_policy& ep) const
    {
      // Order based from the most generic to the most specific setting.
      return this->_memory == ep._memory && this->_access == ep._access
        && this->_quantify__algorithm == ep._quantify__algorithm
        && this->_quantify__transposition_growth == ep._quantify__transposition_growth
        && this->_quantify__transposition_max == ep._quantify__transposition_max
        && this->_nested__fast_reduce == ep._nested__fast_reduce;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Check for at least one mismatch in the settings.
    ////////////////////////////////////////////////////////////////////////////
    bool
    operator!=(const exec_policy& ep) const
    {
      return !(*this == ep);
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set the access mode.
    ////////////////////////////////////////////////////////////////////////////
    exec_policy&
    set(const access& am)
    {
      this->_access = am;
      return *this;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create a copy with the access mode changed.
    ////////////////////////////////////////////////////////////////////////////
    exec_policy
    operator&(const access& am) const
    {
      exec_policy ep = *this;
      return ep.set(am);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set the memory mode.
    ////////////////////////////////////////////////////////////////////////////
    exec_policy&
    set(const memory& mm)
    {
      this->_memory = mm;
      return *this;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create a copy with the memory mode changed.
    ////////////////////////////////////////////////////////////////////////////
    exec_policy
    operator&(const memory& mm) const
    {
      exec_policy ep = *this;
      return ep.set(mm);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set the quantify algorithm.
    ////////////////////////////////////////////////////////////////////////////
    exec_policy&
    set(const quantify::algorithm& qa)
    {
      this->_quantify__algorithm = qa;
      return *this;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create a copy with the quantify algorithm changed.
    ////////////////////////////////////////////////////////////////////////////
    exec_policy
    operator&(const quantify::algorithm& qa) const
    {
      exec_policy ep = *this;
      return ep.set(qa);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set the quantify strategy.
    ////////////////////////////////////////////////////////////////////////////
    exec_policy&
    set(const quantify::transposition_growth& tg)
    {
      this->_quantify__transposition_growth = tg;
      return *this;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create a copy with the quantify algorithm changed.
    ////////////////////////////////////////////////////////////////////////////
    exec_policy
    operator&(const quantify::transposition_growth& tg) const
    {
      exec_policy ep = *this;
      return ep.set(tg);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set the quantify strategy.
    ////////////////////////////////////////////////////////////////////////////
    exec_policy&
    set(const quantify::transposition_max& tm)
    {
      this->_quantify__transposition_max = tm;
      return *this;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create a copy with the quantify algorithm changed.
    ////////////////////////////////////////////////////////////////////////////
    exec_policy
    operator&(const quantify::transposition_max& tm) const
    {
      exec_policy ep = *this;
      return ep.set(tm);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set the quantify strategy.
    ////////////////////////////////////////////////////////////////////////////
    exec_policy&
    set(const nested::fast_reduce& fr)
    {
      this->_nested__fast_reduce = fr;
      return *this;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create a copy with the quantify strategy changed.
    ////////////////////////////////////////////////////////////////////////////
    exec_policy
    operator&(const nested::fast_reduce& fr) const
    {
      exec_policy ep = *this;
      return ep.set(fr);
    }
  };

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Chosen access mode.
  ////////////////////////////////////////////////////////////////////////////
  template <>
  inline const exec_policy::access&
  exec_policy::get<exec_policy::access>() const
  {
    return this->_access;
  }

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Chosen memory type.
  ////////////////////////////////////////////////////////////////////////////
  template <>
  inline const exec_policy::memory&
  exec_policy::get<exec_policy::memory>() const
  {
    return this->_memory;
  }

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Chosen quantification strategy.
  ////////////////////////////////////////////////////////////////////////////
  template <>
  inline const exec_policy::quantify::algorithm&
  exec_policy::get<exec_policy::quantify::algorithm>() const
  {
    return this->_quantify__algorithm;
  }

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Chosen quantification strategy.
  ////////////////////////////////////////////////////////////////////////////
  template <>
  inline const exec_policy::quantify::transposition_growth&
  exec_policy::get<exec_policy::quantify::transposition_growth>() const
  {
    return this->_quantify__transposition_growth;
  }

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Chosen quantification strategy.
  ////////////////////////////////////////////////////////////////////////////
  template <>
  inline const exec_policy::quantify::transposition_max&
  exec_policy::get<exec_policy::quantify::transposition_max>() const
  {
    return this->_quantify__transposition_max;
  }

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Chosen Nested Sweeping fast reduce epsilon value.
  ////////////////////////////////////////////////////////////////////////////
  template <>
  inline const exec_policy::nested::fast_reduce&
  exec_policy::get<exec_policy::nested::fast_reduce>() const
  {
    return this->_nested__fast_reduce;
  }

  /// \}
  //////////////////////////////////////////////////////////////////////////////

  /// \cond

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Lift enum values to `exec_policy`.
  //////////////////////////////////////////////////////////////////////////////
  template <typename A, typename B>
  typename std::enable_if<
    std::is_convertible<A, exec_policy>::value && !std::is_same<A, exec_policy>::value
      && std::is_convertible<A, exec_policy>::value && !std::is_same<A, exec_policy>::value,
    exec_policy>::type
  operator&(const A& a, const B& b)
  {
    return exec_policy(a) & b;
  }

  /// \endcond
}

#endif // ADIAR_EXEC_POLICY_H
