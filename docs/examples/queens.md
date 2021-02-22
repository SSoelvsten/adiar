---
layout: default
title: Queens
nav_order: 1
parent: Examples
description: "N-Queens example"
permalink: examples/queens
---

# N-Queens
{: .no_toc }

Remember that the N-Queens problem is as follows
> Given N, then in how many ways can N queens be placed on an N x N chess board
> without threatening each other?
We will solve this problem using BDDs. The final program is available in
[example/queens.cpp](//github.com/SSoelvsten/adiar/blob/master/example/queens.cpp).

{: .fs-6 .fw-300 }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

## Computing the set of all solutions

We will compute the BDD containing all the solutions as described in the paper
"Parallel Disk-Based Computation for Large, Monolithic Binary Decision Diagrams"
by Daniel Kunkle, Vlad Slavici, and Gene Cooperman.

We need to first choose some encoding of the problem into a set of variables. We
will stay with the simple row-by-row ordering of variables for now. That is, we
represent whether a queen is placed at position (_i_,_j_) on the _N_×_N_ board
board as the variable with label computed as follows.

```c++
inline label_t label_of_position(uint64_t N,
                                 uint64_t i,
                                 uint64_t j)
{
  return (N * i) + j;
}
```

### Explicitly constructing base cases

Let us first restrict our attention to the base case of expressing the state of
a single field (_i_,_j_). We need to express that a single queen is placed here,
and that this queen is in no conflict with any other placed on the board, i.e.
any queens on the same row, column or diagonals. This essentially is the formula
`x_ij /\ !is_threatened(i,j)`, where `is_threatened(i,j)` is true if one or more
queens are placed on conflicting positions.

We could construct the BDD with the builders and algorithms of _Adiar_. But, we
can do even better than that, because the resulting BDD is well structured. So,
we can explicitly construct in one go with a [node stream](/core.md#files)!
Remember that nodes are to be written bottom-up and in reverse. By the ordering
of variables in `label_of_position` we have to deal with (1) queens on the row
_i_ and (2) queens on other rows. For (1) we have to check all variables,
whereas for (2) we only need to check on column _j_ and the diagonals. All nodes
but the one for x<sub>ij</sub> are connected to by their _low_ edge to the node
generated before them (or to the _true_ sink if said node is first one
generated). The x<sub>ij</sub> variable is, on the other hand, connected to the
prior generated node by its high edge. All other edges go the to _false_ sink.

```c++
bdd n_queens_S(uint64_t N, uint64_t i, uint64_t j)
{
  node_file out;
  node_writer out_writer(out);

  uint64_t row = N - 1;
  ptr_t next = create_sink_ptr(true);

  do {
    uint64_t row_diff = std::max(row,i) - std::min(row,i);

    if (row_diff == 0) {
      // On row of the queen in question
      uint64_t column = N - 1;
      do {
        label_t label = label_of_position(N, row, column);

        // If (row, column) == (i,j), then the chain goes through high
        if (column == j) {
          // Node to check whether the queen actually is placed, and
          // if so whether all remaining possible conflicts have to
          // be checked.
          label_t label = label_of_position(N, i, j);
          node_t queen = create_node(label,
                                     0,
                                     create_sink_ptr(false),
                                     next);

          out_writer << queen;
          next = queen.uid;
          continue;
        }

        node_t out_node = create_node(label,
                                      0,
                                      next,
                                      create_sink_ptr(false));

        out_writer << out_node;
        next = out_node.uid;
      } while (column-- > 0);
    } else {
      // On another row
      if (j + row_diff < N) {
        // Diagonal to the right is within bounds
        label_t label = label_of_position(N, row, j+row_diff);
        node_t out_node = create_node(label,
                                      0,
                                      next,
                                      create_sink_ptr(false));

        out_writer << out_node;
        next = out_node.uid;
      }

      // Column
      label_t label = label_of_position(N, row, j);
      node_t out_node = create_node(label,
                                    0,
                                    next,
                                    create_sink_ptr(false));

      out_writer << out_node;
      next = out_node.uid;

      if (row_diff <= j) {
        // Diagonal to the left is within bounds
        label_t label = label_of_position(N, row, j-row_diff);
        node_t out_node = create_node(label,
                                      0,
                                      next,
                                      create_sink_ptr(false));

        out_writer << out_node;
        next = out_node.uid;
      }
    }
  } while (row-- > 0);

  return out;
}
```

Notice above, that we return a value of type `node_file` but the function
returns `bdd`. The `bdd` object provides a copy-constructor that creates a BDD
given an underlying `node_file`.

### Constructing the entire board

From the formula in `n_queens_S` we can construct the formula for the entire row
by combining them with an OR. Since the formula is _true_ only when the queen is
placed then this ensures at-least-one queen is placed on the row. Since
`n_queens_S` is also only _true_ when said queen has no conflicts then this also
immediately contains the at-most-one queen constraint for said row.

```c++
bdd n_queens_R(uint64_t N, uint64_t row)
{
  bdd out = n_queens_S(N, row, 0);

  for (uint64_t j = 1; j < N; j++) {
    out |= n_queens_S(N, row, j);
    largest_nodes = std::max(largest_nodes, bdd_nodecount(out));
  }
  return out;
}
```

Now that we can represent a single row, then we only need to combine them such
that all rows are satisfied at the same time. That is, we need to combine the
BDDs constructed in `n_queens_R` with an AND.

```c++
bdd n_queens_B(uint64_t N)
{
  if (N == 1) { return n_queens_S(N, 0, 0); }

  bdd out = n_queens_R(N, 0);

  for (uint64_t i = 1; i < N; i++) {
    out &= n_queens_R(N, i);
    largest_nodes = std::max(largest_nodes, bdd_nodecount(out));
  }
  return out;
}
```

### Counting the number of solutions

When the entire board is constructed as described above, then we merely need to
count the number of satisfying solutions to the generated BDD.

```c++
int main(int argc, char* argv[])
{
  bdd board = n_queens_B(N);
  std::cout << bdd_satcount(board) << "\n";
}
```

## Printing each solution

The following is based on a [report by Martin Faartoft](github.com/MartinFaartoft/n-queens-bdd/blob/master/report.tex).

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

```c++
inline uint64_t i_of_label(uint64_t N, label_t label)
{
  return label / N;
}

inline uint64_t j_of_label(uint64_t N, label_t label)
{
  return label % N;
}
```

Now we are ready to implement the recursive procedure that takes care of a row
and possibly recurses. For a sanity check, we also return the number of
solutions we have listed.

```c++
uint64_t n_queens_list(uint64_t N, uint64_t column,
                       std::vector<uint64_t>& partial_assignment,
                       const bdd& constraints)
{
  if (is_sink(constraints, is_false)) {
    return 0;
  }
  uint64_t solutions = 0;

  for (uint64_t row_q = 0; row_q < N; row_q++) {
    partial_assignment.push_back(row_q);

    // Construct the assignment for this entire column
    assignment_file column_assignment;

    { // The assignment_writer has to be detached, before we call any
      // bdd functions. It is automatically detached upon destruction,
      //  hence we have it in this little scope.
      assignment_writer aw(column_assignment);

      for (uint64_t row = 0; row < N; row++) {
        aw << create_assignment(label_of_position(N, row, column),
                                row == row_q);
      }
    }

    bdd restricted_constraints = bdd_restrict(constraints,
                                              column_assignment);

    if (bdd_pathcount(restricted_constraints) == 1) {
      solutions += 1;

      // Obtain the lexicographically minimal true assignment. Well,
      // only one exists, so we get the only one left.
      assignment_file forced = bdd_satmin(restricted_constraints);

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
          return j_of_label(N, a.label) < j_of_label(N, b.label);
        }
      };

      sort(forced, sort_by_column(N));

      // Copy the rest of the assignment into 'partial_assignment'
      assignment_stream<> fas(forced);
      while (fas.can_pull()) {
        assignment a = fas.pull();
        if (a.value) {
          partial_assignment.push_back(i_of_label(N, a.label));
        }
      }

      n_queens_print_solution(partial_assignment);

      for (uint64_t c = N-1; c > column; c--) {
        partial_assignment.pop_back();
      }
    } else if (is_sink(restricted_constraints, is_true)) {
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

uint64_t n_queens_list(uint64_t N, const bdd& board)
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


```c++
void n_queens_print_solution(std::vector<uint64_t>& assignment)
{
  for (uint64_t r : assignment) {
    std::cout << r << " ";
  }
  std::cout << "\n";
}
```

