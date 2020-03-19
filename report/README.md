# Report
In an attempt to reduce any code duplication, the reports content is moved into
two completely separate files `abstract.tex` and `main.tex`. This way, merely
these two files have to included in the appropriate places of any given
preamble. The report can then easily be repurposed into into an actual paper and
submitted to with a conference-specific preamble, if we should get that lucky.

## Compilation
To compile the primary version, run the commands `pdflatex arxiv`, `biber
arxiv`, and finally `pdflatex arxiv` again. Alternatively, just run `make
arxiv`.

## Dependencies
The `arxiv` version uses my preamble that can be found [here](
https://github.com/SSoelvsten/LaTeX-Preamble_and_Examples), which is expected to
be cloned to the same level as the root of this repository.
