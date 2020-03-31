# LaTeX
To allow reusability and maintainability, then many parts of the source has been
split into multiple files.

- `listing`: All _lstlisting_ code snippets
- `tikz`: All _tikz_ drawings

From these we provide the following documents:
- [X] `report`: An overall writeup, that presents the algorithms in detail,
      provides analysis of the algorithm and proposed changes and optimisations.
- [ ] `article_theory`: Contains the same as _report_, but without the
      pseudocode to make it fit for publication.
- [ ] `article_impl`: Contains benchmarks of the implementation compared the
      modern OBDD implementation of
      [Sylvan](https://github.com/utwente-fmt/sylvan).

## Compilation
Either compile the `main.tex` file or use the `Makefile`.

## Dependencies
The main versions of all documents uses my preamble, which can be found [here](
https://github.com/SSoelvsten/LaTeX-Preamble_and_Examples). The documents expect
this to be cloned to the same level as the root of this repository.
