@ECHO OFF
REM Build script for Sphinx documentation (Windows).
REM Written by Claude.
REM
REM Usage:
REM   make.bat docs       - Run Doxygen then build HTML with Sphinx
REM   make.bat doxygen    - Run Doxygen only (generates XML)
REM   make.bat sphinx     - Build Sphinx HTML only (assumes Doxygen XML exists)
REM   make.bat clean      - Remove all generated output

SET SPHINXBUILD=sphinx-build
SET SOURCEDIR=source
SET BUILDDIR=build

IF "%1"=="" GOTO help
IF "%1"=="help" GOTO help
IF "%1"=="docs" GOTO docs
IF "%1"=="doxygen" GOTO doxygen
IF "%1"=="sphinx" GOTO sphinx_build
IF "%1"=="clean" GOTO clean
IF "%1"=="open" GOTO open
GOTO help

:help
ECHO Usage:
ECHO   make.bat docs      Run Doxygen + Sphinx to build full HTML docs
ECHO   make.bat doxygen   Run Doxygen only (generates XML for Breathe)
ECHO   make.bat sphinx    Build Sphinx HTML only (needs Doxygen XML)
ECHO   make.bat clean     Remove all generated output
ECHO   make.bat open      Build docs and open in browser
GOTO end

:docs
CALL :doxygen
CALL :sphinx_build
GOTO end

:doxygen
doxygen Doxyfile
GOTO :EOF

:sphinx_build
%SPHINXBUILD% -b html %SOURCEDIR% %BUILDDIR%\html
GOTO :EOF

:clean
IF EXIST %BUILDDIR% rmdir /s /q %BUILDDIR%
IF EXIST xml rmdir /s /q xml
GOTO end

:open
CALL :docs
start %BUILDDIR%\html\index.html
GOTO end

:end
