# makeenv optional file for additional setting
# LATEXARGS additional flags to latex to be set in makeenv
# LATEXMK latex compilation defaulting to latexmk, can be changed in makeenv
# POST_CMD additional action after latexmk
# Example: insert LATEXARGS=-file-line-error into makeenv
# Example: insert POST_CMD=&& /bin/cp -fv $(TRGT).pdf ~/win/Documents/

-include makeenv
TRGT=paper
# TRGT=abstract

LATEXMK?=latexmk

all:
	@$(LATEXMK) -pdf $(LATEXARGS) $(TRGT) $(POST_CMD)

clean:
	@latexmk -c $(TRGT)

dist: all
	mkdir sat21_atpg_submission
	mkdir sat21_atpg_submission/results
	mkdir sat21_atpg_submission/data
	cp *.tex *.bib *.bst *.cls paper.pdf sat21_atpg_submission/
	cp paper.pdf sat21_atpg_submission/JanotaATPG.pdf
	cp -r data/NewHistogram.pdf data/*.tex sat21_atpg_submission/data/
	cp -r results/*.tex  sat21_atpg_submission/results/
	cp -r tikzfigures sat21_atpg_submission/
	cd sat21_atpg_submission/ && zip sat21_atpg_submission.zip -r .

allclean: clean
	$(LATEXMK) -C $(TRGT)
	rm -fv $(TRGT).bbl

b:
	bibtex $(TRGT)
