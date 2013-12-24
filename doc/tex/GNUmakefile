TARGET = photon_correlation
OUTPUT = build

LATEX = latexmk -pdf -silent

TEX_FILES = ${shell find . -name '*.tex'}

default: ${TARGET}.pdf

${TARGET}.pdf: ${TEX_FILES} ${BIB_FILES}
	${LATEX} ${TARGET}

clean: 
	rm -f *.aux *.bbl *.blg ${TARGET}*.pdf *latexmk *.log *.toc *.fls
