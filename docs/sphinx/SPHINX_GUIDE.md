# Sphinx Documentation -- Quick Start Guide

*Written by Claude.*

This project uses **Sphinx + Breathe + Exhale + Doxygen** to auto-generate
HTML documentation from the C++ source code. Doxygen parses the `@brief`,
`@param`, etc. comments you already write; Exhale + Breathe bridge that into
Sphinx, which renders polished, searchable HTML with the Read the Docs theme.

**New classes, files, and namespaces are picked up automatically** -- no manual
steps required beyond writing your normal Doxygen comments in the source.

## How It Works (the pipeline)

```
  C++ headers          Doxygen           Exhale + Breathe         Sphinx
  (@brief, etc.)  -->  XML output  -->  auto-generated RST  -->  HTML docs
```

All configuration lives under `docs/sphinx/`:

| File / Dir          | Purpose                                       |
|---------------------|-----------------------------------------------|
| `Doxyfile`          | Standalone Doxygen config (for `make doxygen`) |
| `source/conf.py`    | Sphinx + Exhale config -- theme, extensions    |
| `source/index.rst`  | Top-level table of contents                    |
| `requirements.txt`  | Python dependencies for pip                    |
| `Makefile`          | Build commands (Mac/Linux)                     |
| `make.bat`          | Build commands (Windows)                       |

Generated output (all git-ignored):

| Dir            | Contents                                      |
|----------------|-----------------------------------------------|
| `xml/`         | Doxygen XML (intermediate)                    |
| `source/api/`  | Exhale's auto-generated RST files             |
| `build/`       | Final Sphinx HTML in `build/html`             |

---

## Prerequisites

You need **three** tools installed. All are cross-platform.

### 1. Python 3 + pip

- **Mac**: `brew install python` (or use the system Python 3 if available)
- **Windows**: Download from https://www.python.org/downloads/ -- check
  "Add Python to PATH" during install

### 2. Doxygen

- **Mac**: `brew install doxygen`
- **Windows**: Download the installer from https://www.doxygen.nl/download.html
  and add it to your PATH, or use `choco install doxygen.install` if you have
  Chocolatey

### 3. Sphinx + Breathe + Exhale + Theme (Python packages)

From the project root (or anywhere, really):

```bash
pip install -r docs/sphinx/requirements.txt
```

This installs `sphinx`, `breathe`, `exhale`, and `sphinx-rtd-theme`.

---

## Building the Docs

### Mac / Linux

```bash
cd docs/sphinx
make docs          # runs Doxygen then Sphinx (Exhale runs automatically)
make open          # same, but also opens the result in your browser
```

### Windows

```cmd
cd docs\sphinx
make.bat docs      # runs Doxygen then Sphinx (Exhale runs automatically)
make.bat open      # same, but also opens the result in your browser
```

The generated HTML lands in `docs/sphinx/build/html/index.html`.

### Other targets

| Command          | What it does                             |
|------------------|------------------------------------------|
| `make doxygen`   | Run Doxygen only (regenerate XML)        |
| `make sphinx`    | Run Sphinx only (Exhale handles Doxygen) |
| `make clean`     | Delete all generated output              |

Note: Exhale is configured to run Doxygen itself during the Sphinx build, so
`make sphinx` alone is actually sufficient to produce the full docs. The
`make docs` target runs standalone Doxygen first as well, which is redundant
but harmless and ensures the `xml/` directory is always up to date.

---

## Adding Documentation for New Classes

Just write Doxygen comments in your header -- **that's it**. Exhale
automatically discovers all classes, structs, namespaces, files, and free
functions in `source/` and generates an RST page for each one.

```cpp
namespace cse498 {

/// @brief A brief description of your class.
class MyNewClass {
public:
    /// @brief Do something useful.
    /// @param value The input value to process.
    /// @return The processed result.
    int DoSomething(int value);
};

} // namespace cse498
```

Run `make docs` and `MyNewClass` will appear in the API Reference tree,
complete with its member documentation, inheritance diagram, and a link to
the full source listing.

---

## Writing Prose / Guides Alongside API Docs

One of Sphinx's strengths over standalone Doxygen is mixing hand-written
documentation with auto-generated API references. To add a written guide:

1. Create a new `.rst` file, e.g. `docs/sphinx/source/getting-started.rst`
2. Add it to the `toctree` in `index.rst`
3. Write in reStructuredText (RST) -- here's a quick cheat sheet:

```rst
Section Title
=============

Subsection
----------

This is a paragraph. **Bold**, *italic*, ``inline code``.

.. code-block:: cpp

   // A code block with syntax highlighting
   auto agent = world.AddAgent<PacingAgent>("Pacer");

- Bullet list item
- Another item

See :cpp:class:`cse498::AgentBase` for cross-reference to API docs.
```

---

## CI/CD Integration

Because the build is a single command (`make docs` or `make sphinx`), it drops
right into a CI pipeline. A minimal GitHub Actions step would look like:

```yaml
- name: Install doc dependencies
  run: |
    sudo apt-get install -y doxygen
    pip install -r docs/sphinx/requirements.txt

- name: Build documentation
  run: |
    cd docs/sphinx
    make docs
```

This ensures docs are always up to date and will catch broken Doxygen comments
(e.g., `@param` names that don't match the code).

---

## Sphinx vs. Doxygen -- Key Differences

| Aspect                  | Doxygen alone                   | Sphinx + Exhale                 |
|-------------------------|---------------------------------|---------------------------------|
| Output quality          | Functional but dated look       | Modern, themed, searchable      |
| Auto-discovery          | Built-in                        | Built-in (via Exhale)           |
| Prose documentation     | Awkward (mainpage, groups)      | First-class RST/Markdown        |
| Cross-referencing       | Automatic but limited           | Rich, customizable              |
| Hosting                 | Self-host HTML                  | Read the Docs (free), GitHub Pages, or self-host |
| Comment syntax          | Doxygen tags (`@brief`, etc.)   | Same -- Breathe reads them      |
| Learning curve          | Low                             | Slightly higher (RST syntax)    |

The bottom line: your existing Doxygen comments work unchanged. Sphinx just
gives you a nicer front-end and the ability to write real documentation pages
alongside the API reference -- and Exhale means nobody has to remember to
update RST files when they add a new class.

---

## Troubleshooting

**"doxygen: command not found"** -- Install Doxygen (see Prerequisites above).

**"sphinx-build: command not found"** -- Make sure you ran
`pip install -r docs/sphinx/requirements.txt` and that your Python Scripts
directory is on your PATH. On Windows this is typically
`C:\Users\<you>\AppData\Local\Programs\Python\Python3X\Scripts`.

**Doxygen warnings about undocumented parameters** -- These come from mismatches
between `@param` tags and actual parameter names (e.g., unnamed parameters in
the source). They're harmless but worth fixing in the source when convenient.
