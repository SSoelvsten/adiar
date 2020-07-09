# Documentation
We provide a LaTeX_ document acting as a Blue Paper for documentation.

<!-- markdown-toc start - Don't edit this section. Run M-x markdown-toc-refresh-toc -->
**Table of Contents**

- [Documentation](#documentation)
    - [LaTeX Blue Paper](#latex-blue-paper)
        - [Dependencies](#dependencies)
        - [Compilation](#compilation)

<!-- markdown-toc end -->


## LaTeX Blue Paper
To allow reusability and maintainability, then many parts of the source has been
split into multiple files. The following are the different folders

| Folder     | Purpose                             |
|------------|-------------------------------------|
| `listing/` | All _lstlisting_ code snippets      |
| `tikz/`    | All _tikz_ drawings                 |
| `report/`  | The *tex*t files for the blue paper |

If the benchmarks seem promising and worth reporting, then the report will be
reworked into one or more papers, which may also be placed in their respective
folders.

### Dependencies
The main versions of all documents uses my preamble, which can be found [here](
https://github.com/SSoelvsten/LaTeX-Preamble_and_Examples) on the
`preamble-only` branch. For convenience, it is already included as a submodule.
Everything will be set up, if you run the following command.
```
git submodule update --init --recursive
```

### Compilation
Either compile `main.tex` in the `report/` folder or just use the provided
`Makefile` with the `report` target.
