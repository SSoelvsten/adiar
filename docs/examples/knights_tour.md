---
layout: default
title: Queens
nav_order: 2
parent: Examples
description: "Knight's Tour example"
permalink: examples/knights_tour
---

# Knight's Tour
{: .no_toc }

The Knight's Tour problem is the following combinatorial problem
{: .fs-6 .fw-300 }

> Given N, then how many hamiltonian paths exist for a Knight (starting at any
> position) on an N x N chess board?
{: .fs-6 .fw-300 }

The *closed* variant of this is for a *hamiltonian cycle*. We are going to use
an N<sup>4</sup> encoding of which only N<sup>2</sup> positions are part of
every final solution. Since this is very sparse, then we will solve this problem
using ZDDs.
{: .fs-6 .fw-300 }

## Table of Contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Variable Encoding

Our encoding is based off of the solution by Randal Bryant
[here](https://github.com/rebryant/Cloud-BDD/blob/conjunction_streamlined/hamiltonian/hpath.py).
To get around the entire board, the Knight will need to use N<sup>2</sup>
time-steps. Our variable ordering will represent every *time-slice* together,
where the board of each slice is similar to the [Queens example](queens.md).
```cpp
inline adiar::label_t label_of_position(int N, int i, int j, int t)
{
  return (N * N * t) + (N * i) + j;
}
```

## Closed Tour

To only consider closed tours, i.e. hamiltonian cycles, we merely fix it to
start in the top-left corner. Since we are counting *unoriented* paths, i.e.
where the going backwards is not counted as another path, then we can also fix
the next square at time step *t* = 1 and the last one at *t* = N<sup>2</sup>-1.
The following code recognises whether a coordinate *i* and *j* is one of these
squares.

```cpp
const int closed_squares [3][2] = {{0,0}, {1,2}, {2,1}};

bool is_closed_square(int i, int j)
{
  return (i == closed_squares[0][0] && j == closed_squares[0][1])
      || (i == closed_squares[1][0] && j == closed_squares[1][1])
      || (i == closed_squares[2][0] && j == closed_squares[2][1]);
}
```

To encode the constraint for time step *t* = 0, *t* = 1, and *t* =
N<sup>2</sup>-1 we need to add a long "don't care" chain in between. Inside of
this chain can also immediately add the [hamiltonian
constraint](#hamiltonian-constraint) to not visit any of these squares again.

```cpp
template<bool filter_closed_squares = false>
void constraint_dont_care(adiar::node_writer &out_writer, adiar::ptr_t &curr_root,
                          int N, int max_t, int min_t)
{
  for (int curr_t = max_t; curr_t >= min_t; curr_t--) {
    for (int i = N - 1; i >= 0; i--) {
      for (int j = N - 1; j >= 0; j--) {
        if (filter_closed_squares && is_closed_square(i,j)) {
          continue;
        }

        adiar::node_t out_node = adiar::create_node(label_of_position(N,i,j,curr_t),
                                                    0,
                                                    curr_root,
                                                    curr_root);

        out_writer << out_node;
        curr_root = out_node.uid;
      }
    }
  }
}
```

Finally, the following creates the constraint of where to start, go to, and end.

```cpp
adiar::zdd constraint_closed(uint64_t N)
{
  adiar::node_file out;

  { adiar::node_writer out_writer(out);

    // Set final time step to (2,1)
    adiar::node_t n_t_end = adiar::create_node(label_of_position(N,
                                                                 closed_squares[2][0],
                                                                 closed_squares[2][1],
                                                                 N*N-1),
                                               adiar::MAX_ID,
                                               adiar::create_sink_ptr(false),
                                               adiar::create_sink_ptr(true));
    out_writer << n_t_end;

    // Create don't care on all intermediate steps
    adiar::ptr_t curr_root = n_t_end.uid;
    constraint_dont_care<true>(out_writer, curr_root, N, N*N-2, 2);

    // Set t = 1 to (1,2)
    adiar::node_t n_t_1 = adiar::create_node(label_of_position(N,
                                                               closed_squares[1][0],
                                                               closed_squares[1][1],
                                                               1),
                                             adiar::MAX_ID,
                                             adiar::create_sink_ptr(false),
                                             curr_root);
    out_writer << n_t_1;

    // Set t = 0 to (0,0)
    adiar::node_t n_t_0 = adiar::create_node(label_of_position(N,
                                                               closed_squares[0][0],
                                                               closed_squares[0][1],
                                                               0),
                                             adiar::MAX_ID,
                                             adiar::create_sink_ptr(false),
                                             n_t_1.uid);
    out_writer << n_t_0;
  }

  largest_nodes = std::max(largest_nodes, out.size());
  return out;
}
```

## Transition Function

Before tackling the construction of the ZDD for the transition function, let us
first look at a single move. Since the main bottleneck is computing the ZDD
operations, then the following computations will not necessarily be heavily
optimised.

A Knight moves in an *L*-shape across the board, and so the following piece of
code computes whether two coordinates represent a legal move.

```cpp
constexpr int row_moves[8]    = { -2, -2, -1, -1,  1,  1,  2,  2 };
constexpr int column_moves[8] = { -1,  1, -2,  2, -2,  2, -1,  1 };

bool is_move(int i_from, int j_from, int i_to, int j_to)
{
  for (int idx = 0; idx < 8; idx++) {
    if (i_from + row_moves[idx] == i_to &&
        j_from + column_moves[idx] == j_to) {
      return true;
    }
  }
  return false;
}
```

We will in the later construction of the transition function "iterate" over
these moves (in the order they are defined in the two `constexpr` arrays). Here,
we will also incorporate the time-dimension directly by not returning the tuple
of coordinates but an `adiar::ptr` with said coordinate and already at the next
time step.

```cpp
adiar::ptr_t ptr_to_first(int N, int i_from, int j_from, int t)
{
  for (int idx = 0; idx < 8; idx++) {
    int i_to = i_from + row_moves[idx];
    int j_to = j_from + column_moves[idx];

    if (i_to < 0 || N <= i_to || j_to < 0 || N <= j_to) { continue; }
    adiar::label_t from_label = label_of_position(N, i_from, j_from, t);
    adiar::label_t to_label = label_of_position(N, i_to, j_to, t+1);

    return adiar::create_node_ptr(to_label, from_label);
  }
  return adiar::create_sink_ptr(false);
}

adiar::ptr_t ptr_to_next(int N, int i_from, int j_from, int i_to, int j_to, int t)
{
  bool seen_move = false;

  for (int idx = 0; idx < 8; idx++) {
    int i = i_from + row_moves[idx];
    int j = j_from + column_moves[idx];

    if (i < 0 || N <= i || j < 0 || N <= j) { continue; }

    if (seen_move) {
      adiar::label_t from_label = label_of_position(N, i_from, j_from, t);
      adiar::label_t to_label = label_of_position(N, i, j, t+1);

      return adiar::create_node_ptr(to_label, from_label);
    }
    seen_move = i == i_to && j == j_to;
  }
  return adiar::create_sink_ptr(false);
}
```

The transition function only encodes the time step *t* to *t+1*. That is, it
needs to allow anything to happen at all other time steps. Hence, it also needs
a somewhat similar "don't care" chain as the one for the [closed
constraint](#closed-constraint) above but for some other time limits *min_t* to
some *max_t* (inclusive).

Combining everything above, we can create the transition function as follows.

```cpp
adiar::zdd constraint_transition(int N, int t)
{
  adiar::node_file out;

  { // When calling `out.size()` below, we have to make it read-only. So, we
    // have to detach the node_writer before we do. This is automatically done
    // on garbage collection, which is why we add an inner scope.
    adiar::node_writer out_writer(out);

    adiar::ptr_t curr_root = adiar::create_sink_ptr(true);

    // "Don't care" for future time steps
    constraint_dont_care<>(out_writer, curr_root, N, N*N-1, t+2);

    // Node chains at time-step t+1 where exactly one possible square (i',j') is
    // visited for every possible one (i,j) at time step t.
    for (int i_t1 = N-1; i_t1 >= 0; i_t1--) {
      for (int j_t1 = N-1; j_t1 >= 0; j_t1--) {
        adiar::label_t next_pos = label_of_position(N, i_t1, j_t1, t+1);

        // We encode the (i,j) inside of the node id to make linking easy.
        for (int i_t = N-1; i_t >= 0; i_t--) {
          for (int j_t = N-1; j_t >= 0; j_t--) {

            // Is (i',j') reachable from (i,j)?
            if (is_move(i_t, j_t, i_t1, j_t1)) {
              adiar::label_t curr_pos = label_of_position(N, i_t, j_t, t);
              adiar::ptr_t low = ptr_to_next(N, i_t, j_t, i_t1, j_t1, t);

              adiar::node_t out_node = adiar::create_node(next_pos,
                                                          curr_pos,
                                                          low,
                                                          curr_root);
              out_writer << out_node;
            }
          }
        }
      }
    }

    // Node chain at time step t
    curr_root = adiar::create_sink_ptr(false);

    for (int i = N-1; i >= 0; i--) {
      for (int j = N-1; j >= 0; j--) {
        adiar::label_t out_label = label_of_position(N, i, j, t);

        adiar::ptr_t high = ptr_to_first(N, i, j, t);

        adiar::node_t out_node = adiar::create_node(out_label, 0, curr_root, high);

        out_writer << out_node;
        curr_root = out_node.uid;
      }
    }

    // "Don't care" for previous time steps
    constraint_dont_care<>(out_writer, curr_root, N, t-1, 0);
  }

  return out;
}
```

## Hamiltonian Constraint

Only using the above we would obtain *all* paths of length N<sup>2</sup>; also
the ones where a position is visited twice.

```cpp
adiar::zdd constraint_exactly_once(uint64_t N, uint64_t i, uint64_t j)
{
  adiar::node_file out;

  { adiar::node_writer out_writer(out);

    adiar::ptr_t next0 = adiar::create_sink_ptr(false);
    adiar::ptr_t next1 = adiar::create_sink_ptr(true);

    for (int curr_t = N*N-1; curr_t >= 0; curr_t--) {
      for (int curr_i = N-1; curr_i >= 0; curr_i--) {
        for (int curr_j = N-1; curr_j >= 0; curr_j--) {
          adiar::label_t out_label = label_of_position(N, curr_i, curr_j, curr_t);

          bool is_ij = i == curr_i && j == curr_j;

          // If we are talking about (i,j) and we already have counted it once, then
          // it is forced to zero. In ZDDs this is represented by skipping it.
          if (!is_ij && (curr_t > 0 || curr_i > i)) {
            adiar::node_t out_n1 = adiar::create_node(out_label, 1, next1, next1);
            out_writer << out_n1;
            next1 = out_n1.uid;
          }

          adiar::node_t out_n0 = adiar::create_node(out_label, 0,
                                                    next0,
                                                    is_ij ? next1 : next0);
          out_writer << out_n0;
          next0 = out_n0.uid;
        }
      }
    }
  }

  largest_nodes = std::max(largest_nodes, out.size());
  return out;
}
```

## Counting the Number of Paths

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
      largest_nodes = std::max(largest_nodes, zdd_nodecount(paths));
    }

    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
        if (only_closed && is_closed_square(i,j)) { continue; }

        paths &= constraint_exactly_once(N,i,j);
        largest_nodes = std::max(largest_nodes, zdd_nodecount(paths));
      }
    }
  }
  
  return paths;
}
```

Since every element of the final ZDD represents a unique path through the board,
then the number of solutions is exactly the size of the set.

```cpp
size_t knights_tour(uint64_t N, bool closed)
{
  adiar::zdd paths = combine_constraints(N, closed);
  uint64_t solutions = zdd_size(paths);
  return solutions;
}
```

