# LaTeX
To allow reusability and maintainability, then many parts of the source has been
split into multiple files. The following are the different folders

- `listing/`: All _lstlisting_ code snippets
- `tikz/`: All _tikz_ drawings
- `report/`: The primary document is the technical report
  - [ ] An overall writeup of the algorithms in detail
    - **DONE:** Reduce, Restrict, Apply, Equality, Evaluate 
    - **TODO:** Existential, Relational Product
  - [ ] Proposal of different optimisations
  - [ ] A description of the implementation with TPIE and the corresponding
        benchmarks
    - [ ] vs a simple recursive implementation of our own.
    - [ ] vs. [Sylvan](https://github.com/utwente-fmt/sylvan)

If the benchmarks seem promising and worth reporting, then the report will be
reworked into one or more papers.

## Compilation
Either compile the `main.tex` file or just use the provided `Makefile` with the
`report` target.

## Dependencies
The main versions of all documents uses my preamble, which can be found [here](
https://github.com/SSoelvsten/LaTeX-Preamble_and_Examples) on the
`preamble-only` branch. For convenience, it is already included as a submodule.
Everything will be set up, if you run the following command.
```
git submodule update --init --recursive
```
