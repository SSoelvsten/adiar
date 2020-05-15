report:
	@cd ./report/
	@pdflatex report
	@biber report
	@pdflatex report
	@cp ./report.tex ../

clean-all:
	@rm -r -f *.aux
	@rm -r -f *.bbl
	@rm -r -f *.bcf
	@rm -r -f *.blg
	@rm -r -f *.idx
	@rm -r -f *.lof
	@rm -r -f *.log
	@rm -r -f *.maf
	@rm -r -f *.mtc*
	@rm -r -f *.out
#	@rm -r -f *.pdf
	@rm -r -f *.run.xml
	@rm -r -f *.toc
