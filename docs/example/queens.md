\page queens Queens

The N-Queens problem is the following combinatorial problem

> Given N, then in how many ways can N queens be placed on an N x N chess board
> without threatening each other?

We will solve this problem using BDDs.

[TOC]

- - -

Computing the set of all solutions
================================================================================

We will compute the BDD containing all the solutions as described in the paper
"Parallel Disk-Based Computation for Large, Monolithic Binary Decision Diagrams"
by Daniel Kunkle, Vlad Slavici, and Gene Cooperman.

We need to first choose some encoding of the problem into a set of variables. We
will stay with the simple row-by-row ordering of variables for now. That is, we
represent whether a queen is placed at position (_i_,_j_) on the _N_×_N_ board
board as the variable with label computed as follows.

```cpp
inline typename adiar::bdd::label_t label_of_position(uint64_t N, uint64_t i, uint64_t j)
{
  return (N * i) + j;
}
```

Explicitly Constructing Base Cases
--------------------------------------------------------------------------------

Let us first restrict our attention to the base case of expressing the state of
a single field (_i_,_j_). We need to express that a single queen is placed here,
and that this queen is in no conflict with any other placed on the board, i.e.
any queens on the same row, column or diagonals. This essentially is the formula
\f[ x_{ij} \land \neg \mathit{is\_threatened}(i,j) \f]
where _is_threatened_(i,j) is true if one or more queens are placed on
conflicting positions.

We could construct the BDD with the builders and algorithms of _Adiar_. But, we
can do even better than that, because the resulting BDD is well structured. So,
we can explicitly construct in one go with a `adiar::bdd_builder`! Remember that
nodes are to be constructed bottom-up. By the ordering of variables in
`label_of_position` we have to deal with (1) queens on the row _i_ and (2)
queens on other rows. For (1) we have to check all variables, whereas for (2) we
only need to check on column _j_ and the diagonals. All nodes but the one for
x<sub>ij</sub> are connected to by their _low_ edge to the node generated before
them (or to the _true_ terminal if said node is first one generated). The
x<sub>ij</sub> variable is, on the other hand, connected to the prior generated
node by its high edge. All other edges go the to _false_ terminal.

```cpp
adiar::bdd n_queens_S(int i, int j)
{
  adiar::bdd_builder builder;

  uint64_t row = N - 1;
  adiar::bdd_ptr next = builder.add_node(true);

  do {
    uint64_t row_diff = std::max(row,i) - std::min(row,i);

    if (row_diff == 0) {
      // On row of the queen in question
      uint64_t column = N - 1;
      do {
        typename adiar::bdd::label_t label = label_of_position(N, row, column);

        // If (row, column) == (i,j), then the chain goes through high
        // such we check the queen actually is placed here.
        next = column == j
          ? builder.add_node(label, false, next)
          : builder.add_node(label, next, false);
      } while (column-- > 0);
    } else {
      // On another row
      if (j + row_diff < N) {
        // Diagonal to the right is within bounds
        next = builder.add_node(label_of_position(N, row, j + row_diff),
                                next,
                                false);
      }

      // Column
      next = builder.add_node(label_of_position(N, row, j), next, false);

      if (row_diff <= j) {
        // Diagonal to the left is within bounds
        next = builder.add_node(label_of_position(N, row, j - row_diff),
                                next,
                                false);
      }
    }
  } while (row-- > 0);

  adiar::bdd res = builder.build();

  largest_nodes = std::max(largest_nodes, bdd_nodecount(res));
  return res;
}
```

Constructing the Entire Board
--------------------------------------------------------------------------------

From the formula in `n_queens_S` we can construct the formula for the entire row
by combining them with an OR. Since the formula is _true_ only when the queen is
placed then this ensures at-least-one queen is placed on the row. Since
`n_queens_S` is also only _true_ when said queen has no conflicts then this also
immediately contains the at-most-one queen constraint for said row.

```cpp
adiar::bdd n_queens_R(int N, int row)
{
  adiar::bdd out = n_queens_S(N, row, 0);

  for (int j = 1; j < N; j++) {
    out |= n_queens_S(N, row, j);
  }
  return out;
}
```

Now that we can represent a single row, then we only need to combine them such
that all rows are satisfied at the same time. That is, we need to combine the
BDDs constructed in `n_queens_R` with an AND.

```cpp
adiar::bdd n_queens_B(int N)
{
  if (N == 1) { return n_queens_S(N, 0, 0); }

  adiar::bdd out = n_queens_R(N, 0);

  for (int i = 1; i < N; i++) {
    out &= n_queens_R(N, i);
  }
  return out;
}
```

Counting the Number of Solutions
================================================================================

When the entire board is constructed as described above, then we merely need to
count the number of satisfying solutions to the generated BDD.

```cpp
int main(int argc, char* argv[])
{
  uint64_t N = std::stoi(argv[1])

  adiar::bdd board = n_queens_B(N);
  std::cout << bdd_satcount(board) << "\n";
}
```

Printing each Solution
================================================================================

The following is based on a [report by Martin
Faartoft](github.com/MartinFaartoft/n-queens-bdd/blob/master/report.tex).

If we want to list all the assignments then we have to do something more than
merely count the number of satisfying assignments. But, given the BDD we just
constructed we can use it to prune our search tree for valid assignments.

Starting at the left-most column we will attempt to place a queen in one of the
rows by _restricting_ the assignment to the variables in question. For each
attempt we will recurse to the next column. Recursion can be stopped early in
two cases:

 - If the given BDD already is trivially false then we have placed a queen, such
   that it conflicts with another.

 - If the number of unique paths in the restricted BDD is exactly one, then we
   are forced to place the remaining queens.

Since we want to backtrack our choices, we may keep BDDs for each column. This
naturally happens by writing it as a recursive procedure. One should notice
though, that this will result in multiple BDDs concurrently being instantiated
in memory and on disk.

We first need to convert a label back into a position on the board before
getting to that . So, we'll also need the following two small functions.

```cpp
inline uint64_t i_of_label(uint64_t N, typename adiar::bdd::label_t label)
{
  return label / N;
}

inline uint64_t j_of_label(uint64_t N, typename adiar::bdd::label_t label)
{
  return label % N;
}
```

Now we are ready to implement the recursive procedure that takes care of a row
and possibly recurses. For a sanity check, we also return the number of
solutions we have listed.

```cpp
uint64_t n_queens_list(uint64_t N, uint64_t column,
                       std::vector<uint64_t>& partial_assignment,
                       const bdd& constraints)
{
  if (is_terminal(constraints, is_false)) {
    return 0;
  }
  uint64_t solutions = 0;

  for (uint64_t row_q = 0; row_q < N; row_q++) {
    partial_assignment.push_back(row_q);

    // Construct the assignment for this entire column
    adiar::assignment_file column_assignment;

    { // The assignment_writer has to be detached, before we call any
      // bdd functions. It is automatically detached upon destruction,
      //  hence we have it in this little scope.
      adiar::file_writer<map_pair<bdd::label_t, bool>> aw(column_assignment);

      for (uint64_t row = 0; row < N; row++) {
        aw << assignment(label_of_position(N, row, column),
                                row == row_q);
      }
    }

    adiar::bdd restricted_constraints = adiar::bdd_restrict(constraints,
                                                            column_assignment);

    if (adiar::bdd_pathcount(restricted_constraints) == 1) {
      solutions += 1;

      // Obtain the lexicographically minimal true assignment. Well,
      // only one exists, so we get the only one left.
      adiar::assignment_file forced = adiar::bdd_satmin(restricted_constraints);

      // Sort the variables back in order of the columns,
      // rather than rows.
      struct sort_by_column
      {
      private:
        uint64_t N;

      public:
        explicit sort_by_column(const uint64_t N) : N(N) { }

        bool operator()(const adiar::assignment &a,
                        const adiar::assignment &b)
        {
          return j_of_label(N, a.var()) < j_of_label(N, b.var());
        }
      };

      adiar::sort(forced, sort_by_column(N));

      // Copy the rest of the assignment into 'partial_assignment'
      adiar::internal::file_stream<adiar::assignment> fas(forced);
      while (fas.can_pull()) {
        adiar::assignment a = fas.pull();
        if (a.value()) {
          partial_assignment.push_back(i_of_label(N, a.var()));
        }
      }

      n_queens_print_solution(partial_assignment);

      for (uint64_t c = N-1; c > column; c--) {
        partial_assignment.pop_back();
      }
    } else if (is_terminal(restricted_constraints, is_true)) {
      n_queens_print_solution(partial_assignment);
      solutions += 1;
    } else {
      solutions += n_queens_list(N,
                                 column+1,
                                 partial_assignment,
                                 restricted_constraints);
    }
    partial_assignment.pop_back();
  }

  return solutions;
}

uint64_t n_queens_list(uint64_t N, const adiar::bdd& board)
{
  if (N == 1) {
    // To make the recursive function work for N = 1 we would have to
    // have the count_paths above check at the beginning. That would
    // in all other cases merely result in an unecessary counting of
    // paths at the very start.
    std::vector<uint64_t> assignment { 0 };
    n_queens_print_solution(assignment);

    return 1;
  }

  std::vector<uint64_t> partial_assignment { };
  partial_assignment.reserve(N);

  return n_queens_list(N, 0, partial_assignment, board);
}
```

where `n_queens_print_solutions` is a function that prints out the assignment file.


```cpp
void n_queens_print_solution(std::vector<uint64_t>& assignment)
{
  for (uint64_t r : assignment) {
    std::cout << r << " ";
  }
  std::cout << "\n";
}
```

Finally we can print all solutions by calling `n_queens_list` with the board
constructed in `n_queens_B`.

```cpp
int main(int argc, char* argv[])
{
  uint64_t N = std::stoi(argv[1])

  adiar::bdd board = n_queens_B(N);
  n_queens_list(board);
}
```

