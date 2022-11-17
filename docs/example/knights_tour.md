\page knights_tour Knight's Tour

The Knight's Tour problem is the following combinatorial problem

> Given N, then how many hamiltonian paths exist for a Knight (starting at any
> position) on an N x N chess board?

The *closed* variant of this is for a *hamiltonian cycle*. We are going to use
an N<sup>4</sup> encoding of which only N<sup>2</sup> positions are part of
every final solution. Since this is very sparse, then we will solve this problem
using ZDDs.

[TOC]

- - -

Manual Construction of Constraints
================================================================================

Our encoding is based off of the solution by Randal Bryant
[here](https://github.com/rebryant/Cloud-BDD/blob/conjunction_streamlined/hamiltonian/hpath.py).
To get around the entire board, the Knight will need to use N<sup>2</sup>
time-steps. Our variable ordering will represent every *time-slice* together,
where the board of each slice is similar to the \ref queens example.
```cpp
inline typename adiar::zdd::label_t int_of_position(int N, int r, int c, int t = 0)
{
  return (N * N * t) + (N * r) + c;
}
```

Closed Tour
--------------------------------------------------------------------------------

To only consider closed tours, i.e. hamiltonian cycles, we merely fix it to
start in the top-left corner. Since we are counting *unoriented* paths, i.e.
where the going backwards is not counted as another path, then we can also fix
the next square at time step \f$ t=1 \f$ and the last one at \f$ t=N^2-1 \f$.
The following code recognises whether a coordinate *i* and *j* is one of these
squares.

```cpp
const int closed_squares [3][2] = { {0,0}, {1,2}, {2,1} };

bool is_closed_square(int r, int c)
{
  return (r == closed_squares[0][0] && c == closed_squares[0][1])
      || (r == closed_squares[1][0] && c == closed_squares[1][1])
      || (r == closed_squares[2][0] && c == closed_squares[2][1]);
}
```

To encode the closed tours, we may force the Knight to start in the top-left
corner and take one of the two solutions. By multiplying with a constant factor,
we can then take the mirrored path and all possible starting positions into
account.

So we need to encode that the Knight at time step \f$ t=0 \f$ is at position
(0,0), at \f$ t=1 \f$ at (1,2), and at time step \f$ t=N^2-1 \f$ at (2,1). This
is easy in ZDDs, since this is equivalent to skipping variables at these three
time steps, such that only the "desired" positions are present and forced to
true.

All remaining time steps remains one long "don't care" chain in between. Yet,
inside of this chain we can encode the hamiltonian constraint for these three
cells on the board by skipping them within the chain.

Hence, we can create the ZDD by hand using the `adiar::zdd_builder` as follows.

```cpp
adiar::zdd constraint_closed(int N)
{
  adiar::zdd_builder builder;

  // Fix t = max_time to be (1,2)
  const int max_time = N*N-1;

  const int stepMax_position = int_of_position(N,
                                               closed_squares[2][0],
                                               closed_squares[2][1],
                                               max_time);

  typename adiar::zdd_ptr root = builder.add_node(stepMax_position, false, true);

  // All in between is as-is but take the hamiltonian constraint into account.
  for (int t = max_time - 1; t > 1; t--) {
    for (int r = N-1; r >= 0; r--) {
      for (int c = N-1; c >= 0; c--) {
        if (is_closed_square(r,c)) { continue; }

        root = builder.add_node(int_of_position(N,r,c,t), root, root);
      }
    }
  }

  // Fix t = 1 to be (2,1)
  const int step1_position = int_of_position(N,
                                             closed_squares[1][0],
                                             closed_squares[1][1],
                                             1);
  root = builder.add_node(step1_position, false, root);

  // Fix t = 0 to be (0,0)
  const int step0_position = int_of_position(N,
                                             closed_squares[0][0],
                                             closed_squares[0][1],
                                             0);
  root = builder.add_node(step0_position, false, root);

  // Finalize
  return builder.build();
}
```

Transition Function
--------------------------------------------------------------------------------

Before tackling the construction of the ZDD for the transition function, let us
first look at a single move. Since the main bottleneck is computing the ZDD
operations, then the following computations will not necessarily be heavily
optimised.

A Knight moves in an *L*-shape across the board, and so the following piece of
code computes whether two coordinates represent a legal move.

```cpp
constexpr int row_moves[8]    = { -2, -2, -1, -1,  1,  1,  2,  2 };
constexpr int column_moves[8] = { -1,  1, -2,  2, -2,  2, -1,  1 };

bool is_legal_move(int /*N*/, int r_from, int c_from, int r_to, int c_to)
{
  for (int idx = 0; idx < 8; idx++) {
    if (r_from + row_moves[idx] == r_to &&
        c_from + column_moves[idx] == c_to) {
      return true;
    }
  }
  return false;
}
```

This can be extended to whether any position even is reachable.

```cpp
bool is_legal_position(int N, int r, int c, int t = 0)
{
  if (r < 0 || N-1 < r)  { return false; }
  if (c < 0 || N-1 < c)  { return false; }
  if (t < 0 || N*N-1 < t) { return false; }

  return true;
}

bool is_reachable(int /*N*/, int r, int c)
{
  for (int idx = 0; idx < 8; idx++) {
    if (is_legal_position(N, r + row_moves[idx], c + column_moves[idx])) {
      return true;
    }
  }
  return false;
}
```

The transition function only encodes the time step *t* to *t+1*. That is, it
needs to allow anything to happen at all other time steps. Hence, it also needs
a somewhat similar "don't care" chain as the one for the closed constraint.

Combining everything above, we can create the transition function as follows.

```cpp
adiar::zdd constraint_transition(int N, int t)
{
  adiar::zdd_builder builder;

  const int max_cell = N-1;

  // Time steps t' > t+1:
  adiar::zdd_ptr post_chain_root = builder.add_node(true);

  const int max_time = N*N-1;

  for (int time = max_time; time > t+1; time--) {
    for (int row = max_cell; row >= 0; row--) {
      for (int col = max_cell; col >= 0; col--) {
        if (!is_reachable(N, row, col)) { continue; }

        const int this_label = int_of_position(N, row, col, time);
        post_chain_root = builder.add_node(this_label,
                                           post_chain_root,
                                           post_chain_root);
      }
    }
  }

  // Time step t+1:
  //   Chain with each possible position reachable from some position at time 't'.
  std::vector<adiar::zdd_ptr> to_chains(N*N, builder.add_node(false));

  for (int row = max_cell; row >= 0; row--) {
    for (int col = max_cell; col >= 0; col--) {
      const int this_label = int_of_position(N, row, col, t+1);

      for (int row_t = max_cell; row_t >= 0; row_t--) {
        for (int col_t = max_cell; col_t >= 0; col_t--) {
          if (!is_reachable(N, row_t, col_t)) { continue; }
          if (!is_legal_move(N, row_t, col_t, row, col)) { continue; }

          const int vector_idx = int_of_position(N, row_t, col_t);

          to_chains.at(vector_idx) = builder.add_node(this_label,
                                                      to_chains.at(vector_idx),
                                                      post_chain_root);
        }
      }
    }
  }

  // Time step t:
  //   For each position at time step 't', check whether we are "here" and go to
  //   the chain checking "where we go to" at 't+1'.
  typename adiar::zdd_ptr root = builder.add_node(false);

  for (int row = max_cell; row >= 0; row--) {
    for (int col = max_cell; col >= 0; col--) {
      if (!is_reachable(N, row, col)) { continue; }

      const int this_label = int_of_position(N, row, col, t);
      const int to_chain_idx = int_of_position(N, row, col);
      root = builder.add_node(this_label, root, to_chains.at(to_chain_idx));
    }
  }

  // Time-step t' < t:
  //   Just allow everything, i.e. add no constraints
  for (int pos = int_of_position(N, max_cell, max_cell, t-1); pos >= 0; pos--) {
    root = builder.add_node(pos, root, root);
  }

  // Finalize
  return builder.build();
}
```

Hamiltonian Constraint
--------------------------------------------------------------------------------

Only using the above we would obtain *all* paths of length \f$ N^2 \f$; also
the ones where a position is visited twice.

```cpp
adiar::zdd constraint_exactly_once(int N, int r, int c)
{
  adiar::zdd_builder builder;

  adiar::zdd_ptr out_never = builder.add_node(false);
  adiar::zdd_ptr out_once = builder.add_node(true);

  const int max_time = N*N-1;
  const int max_cell = N-1;

  for (int this_t = max_time; this_t >= 0; this_t--) {
    for (int this_r = max_cell; this_r >= 0; this_r--) {
      for (int this_c = max_cell; this_c >= 0; this_c--) {
        const int this_label = int_of_position(N, this_r, this_c, this_t);
        const bool is_rc = r == this_r && c == this_c;

        if (!is_rc && (this_t > 0 || this_r > r)) {
          out_once = builder.add_node(this_label, out_once, out_once);
        }

        out_never = builder.add_node(this_label,
                                     out_never,
                                     is_rc ? out_once : out_never);
      }
    }
  }

  // Finalize
  return builder.build();
}
```

Combining Constraints
================================================================================

Notice, that each constraint above uses "don't care" nodes for everything that
is not part of its problem domain. Furthermore, every time step is explicitly
represented. So, the final set of solutions is the *intersection* (the `&`
operator) of all the above.

```cpp
adiar::zdd combine_constraints(uint64_t N, bool closed)
{
  adiar::zdd paths;

  if (N == 1) {
    paths = adiar::zdd_ithvar(0);
  } else {
    paths = only_closed ? constraint_closed(N) : constraint_transition(N,0);
    for (int t = 1; t < N*N-1; t++) {
      paths &= constraint_transition(N,t);
    }

    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
        if (only_closed && is_closed_square(i,j)) { continue; }

        paths &= constraint_exactly_once(N,i,j);
      }
    }
  }
  
  return paths;
}
```

Counting the Number of Paths
================================================================================

Since every element of the final ZDD represents a unique path through the board,
then the number of solutions is exactly the size of the set.

```cpp
size_t knights_tour(uint64_t N, bool closed)
{
  adiar::zdd paths = combine_constraints(N, closed);
  uint64_t solutions = adiar::zdd_size(paths);
  return solutions;
}
```

