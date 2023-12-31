# BEGIN LICENSE BLOCK
# Version: CMPL 1.1
#
# The contents of this file are subject to the Cisco-style Mozilla Public
# License Version 1.1 (the "License"); you may not use this file except
# in compliance with the License.  You may obtain a copy of the License
# at www.eclipse-clp.org/license.
# 
# Software distributed under the License is distributed on an "AS IS"
# basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
# the License for the specific language governing rights and limitations
# under the License. 
# 
# The Original Code is  The ECLiPSe Constraint Logic Programming System. 
# The Initial Developer of the Original Code is  Cisco Systems, Inc. 
# Portions created by the Initial Developer are
# Copyright (C) 2006 Cisco Systems, Inc.  All Rights Reserved.
# 
# Contributor(s): 
# 
# END LICENSE BLOCK

DESTFILE = userman

ROOTFILE = umsroot

CHAPTERS = $(ROOTFILE).tex umsintro.tex umsterm.tex umsusing.tex umscompiler.tex \
	umssyntax.tex umsmodules.tex umsarith.tex umsio.tex umsexcept.tex \
	umsarrays.tex umsdebug.tex umsdeveltools.tex umsmemory.tex \
	umsopsys.tex umssocket.tex umsengines.tex umstkeclipse.tex \
	umsmacros.tex extmeta.tex exthsusp.tex extsuspend.tex \
	umslanguage.tex umsdynamic.tex umsporting.tex \
	umsopers.tex umsstyle.tex umscmdlineopts.tex \
	umserrors.tex umsbugs.tex

EPSFILES = boxmodel.eps resolv.eps dismat.eps dismat2.eps reexport.eps \
	tktop.eps xrefDavinci.eps remotecon.eps remotecon2.eps \
	remotetools.eps remotetools2.eps


GENFILES = err1.tex err2.tex err3.tex err4.tex err5.tex err6.tex \
	err7.tex err8.tex err9.tex err10.tex

# the files that are touched by latex as a side effect
CHAPAUX = $(CHAPTERS:%.tex=%.aux) $(ROOTFILE).toc $(ROOTFILE).idx


all:	$(ROOTFILE).pdf $(ROOTFILE)/$(ROOTFILE).html
	@echo Do make install to install

install:	../../doc/$(DESTFILE).pdf $(ROOTFILE)/$(ROOTFILE).html
	rm -rf ../../doc/$(DESTFILE)
	mv $(ROOTFILE) ../../doc/$(DESTFILE)

../../doc/$(DESTFILE).pdf:	$(ROOTFILE).pdf
	cp $(ROOTFILE).pdf ../../doc/$(DESTFILE).pdf
	chmod 644 ../../doc/$(DESTFILE).pdf


$(ROOTFILE).pdf:	$(CHAPAUX) $(ROOTFILE).bbl $(ROOTFILE).ind 
	@echo ======== UPDATE-PASS =========
	@for f in $(CHAPAUX); do \
		mv $$f $$f.old; cp $$f.old $$f; \
	    done
	TEXINPUTS=../texinputs:$$TEXINPUTS pdflatex $(ROOTFILE)
	# The touches below are supposed to work around nfs clock skew problems:
	# The latex run may leave the pdf file with a modification time in the
	# future. Touch it to make sure the time is right relative to the
	# subsequent touches of the aux files. Sleep in between to make sure
	# the pdf is strictly older than the touched aux files.
	touch $(ROOTFILE).pdf 
	sleep 1
	@for f in $(CHAPAUX); do \
		if cmp -s $$f $$f.old; then mv $$f.old $$f; \
		else rm $$f.old; echo ==== $$f was modified ====; touch $$f; \
		fi \
	    done
	$(MAKE) $(ROOTFILE).pdf

$(CHAPTERS:%.tex=%.aux):	$(CHAPTERS) $(EPSFILES)
	@echo ======== PASS 1 =========
	TEXINPUTS=../texinputs:$$TEXINPUTS pdflatex $(ROOTFILE)

$(ROOTFILE).idx:	$(ROOTFILE).aux
	touch $(ROOTFILE).idx

$(ROOTFILE).bbl:	$(ROOTFILE).aux
	BIBINPUTS=../texinputs bibtex $(ROOTFILE)

$(ROOTFILE).ind:	$(ROOTFILE).idx
	-makeindex $(ROOTFILE)

# $(ROOTFILE).pdf:	$(ROOTFILE).ps
# 	ps2pdf $(ROOTFILE).ps
# 
# $(ROOTFILE).ps:	$(ROOTFILE).dvi
# 	dvips -Ppdf -t a4 -o $(ROOTFILE).ps $(ROOTFILE)
# 
# $(ROOTFILE).ps.Z:	$(ROOTFILE).ps
# 	compress -f $(ROOTFILE).ps

genfiles:
	eclipse -e "[gen_error],gen_errors"

# This preprocesses all *.tex files
# to insert/update the hyperlinks to the bip pages
biprefs:
	eclipse -e "[makerefs],do" -- *.tex

$(ROOTFILE)/$(ROOTFILE).html:	$(CHAPTERS)
	mkdir -p $(ROOTFILE)
	hevea -fix underscore.hva url.hva $(ROOTFILE)
	# replace the hard link index.html -> $(ROOTFILE).html by a soft link
	# because of problems with tar on Windows
	hacha -tocbis $(ROOTFILE).html
        # copy the files needed for the html to $(ROOTFILE) directory
	cp $(ROOTFILE)[0-9]??.* $(ROOTFILE)/
	cp $(ROOTFILE).css $(ROOTFILE)/
	cp contents_motif.* next_motif.* previous_motif.* $(ROOTFILE)
	cp index.html $(ROOTFILE)/$(ROOTFILE).html
	ln -s $(ROOTFILE).html $(ROOTFILE)/index.html

clean:
	rm -f *.bak *.idx *.ind *.bbl *.aux *.dvi *.pdf *.toc *.log *.ilg *.blg
	rm -f *.hind *.haux *.hind *.htoc $(ROOTFILE).image.*
	rm -f *_motif.* *.hidx *.out *.css
	rm -f $(ROOTFILE)[0-9]??.*
	rm -f $(ROOTFILE).ps $(ROOTFILE).ps.Z $(ROOTFILE).pdf $(ROOTFILE).html
	rm -rf $(ROOTFILE) index.html
