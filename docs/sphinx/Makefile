# Makefile for Sphinx documentation (Mac/Linux).
# Written by Claude.
#
# Usage:
#   make docs       - Run Doxygen then build HTML with Sphinx
#   make doxygen    - Run Doxygen only (generates XML)
#   make sphinx     - Build Sphinx HTML only (assumes Doxygen XML exists)
#   make clean      - Remove all generated output

SPHINXBUILD   = sphinx-build
SPHINXOPTS    =
SOURCEDIR     = source
BUILDDIR      = build

.PHONY: docs doxygen sphinx clean help

help:
	@echo "Usage:"
	@echo "  make docs      Run Doxygen + Sphinx to build full HTML docs"
	@echo "  make doxygen   Run Doxygen only (generates XML for Breathe)"
	@echo "  make sphinx    Build Sphinx HTML only (needs Doxygen XML)"
	@echo "  make clean     Remove all generated output"
	@echo "  make open      Build docs and open in browser (Mac)"

docs: doxygen sphinx

doxygen:
	doxygen Doxyfile

sphinx:
	$(SPHINXBUILD) -b html $(SPHINXOPTS) $(SOURCEDIR) $(BUILDDIR)/html

clean:
	rm -rf $(BUILDDIR) xml

open: docs
	open $(BUILDDIR)/html/index.html
