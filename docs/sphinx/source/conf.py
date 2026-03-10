# Sphinx configuration for CSE498 Group Project documentation.
# Written by Claude.
#
# This file is executed as Python, so standard Python syntax applies.

import os

project = 'CSE498 Company B Project'
author = 'Company B'
copyright = '2026'

# -- Extensions ---------------------------------------------------------------

extensions = [
    'breathe',              # Bridge between Doxygen XML and Sphinx
    'exhale',               # Auto-generate RST pages for all classes/files
    'sphinx.ext.autodoc',   # Auto-generate docs from docstrings
    'sphinx.ext.viewcode',  # Add links to source code
]

# -- Breathe configuration ----------------------------------------------------
# Tell Breathe where the Doxygen XML output lives (relative to this file).

breathe_projects = {
    'cse498': os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'xml')),
}
breathe_default_project = 'cse498'

# -- Exhale configuration -----------------------------------------------------
# Exhale auto-generates an RST page for every class, struct, file, namespace,
# etc. found in the Doxygen XML, organized into a browsable tree.

exhale_args = {
    # The directory where Exhale will write its auto-generated .rst files.
    # This path is relative to `source/` (i.e., this file's directory).
    'containmentFolder':     './api',
    # The root document Exhale creates (referenced from index.rst toctree).
    'rootFileName':          'library_root.rst',
    # Title shown at the top of the generated API hierarchy page.
    'rootFileTitle':         'API Reference',
    # Have Exhale run Doxygen for us so `make sphinx` is the only command needed.
    'exhaleExecutesDoxygen': True,
    'exhaleDoxygenStdin':    '''
        INPUT              = ../../../source
        RECURSIVE          = YES
        FILE_PATTERNS      = *.hpp *.h *.cpp
        EXTRACT_ALL        = YES
        EXTRACT_PRIVATE    = YES
        EXTRACT_STATIC     = YES
        EXCLUDE_PATTERNS   = */third-party/*
        QUIET              = YES
        WARNINGS           = YES
    ''',
    # Strip the absolute source path prefix so docs show relative paths.
    'doxygenStripFromPath':  os.path.abspath(os.path.join(os.path.dirname(__file__),
                                                          '..', '..', '..')),
    'createTreeView':        True,
}

# -- General configuration ----------------------------------------------------

templates_path = ['_templates']
exclude_patterns = []

# -- HTML output options -------------------------------------------------------

html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']
