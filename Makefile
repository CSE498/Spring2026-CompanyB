# Top-level Makefile (repo root)
#
# Delegates builds to:
#   source/   (program executables)
#   tests/    (Catch2 unit tests)
#
# Common usage:
#   make                # build default program(s)
#   make test           # build+run unit tests
#   make all            # build program(s) + run tests
#   make clean          # clean both
#   make debug          # debug build (programs)
#   make opt            # optimized build (programs)
#
# You can also forward any target directly:
#   make src-debug
#   make src-grumpy
#   make test-build

# Pass host UID/GID into Docker so build outputs are not owned by root
HOST_UID := $(shell id -u)
HOST_GID := $(shell id -g)
export HOST_UID HOST_GID NO_QT TARGET_MAIN

.PHONY: default all build test clean debug opt quick grumpy \
        src-% test-% help docker-build-emscripten docker-build-native \
        docker-test-emscripten docker-test-native \
        docker-serve docker-dev docker-shell docker-clean \
        docker-image docker-rebuild run-native

# ---------- High-level targets ----------

default: build

# Build program(s) (uses source/ Makefile "default" target)
build:
	$(MAKE) -C source

# Build + run unit tests (uses tests/ Makefile "test" target)
test:
	$(MAKE) -C tests test

# Build program(s) + run tests
all: build test

DOCKER_BUILD_GOALS := $(filter docker-build-% docker-serve docker-dev,$(MAKECMDGOALS))

ifeq ($(words $(DOCKER_BUILD_GOALS)),0) # Check if there aren't any build goals for Docker
debug opt quick grumpy:
	$(MAKE) -C source $@
else
debug opt quick grumpy:
	@:
endif


# Clean everything
clean:
	rm -rf ./build

# Forward anything to source/ by prefixing with src-
#   make src-debug
#   make src-grumpy
src-%:
	$(MAKE) -C source $*

# Forward anything to tests/ by prefixing with test-
#   make test-build
#   make test-list
#   make test-clean
test-%:
	$(MAKE) -C tests $*

# Build the project with Emscripten (ignores Qt)
docker-build-emscripten:
	mkdir -p build
	bash scripts/docker-build.sh build-emscripten $(filter-out $@,$(MAKECMDGOALS))

# Build the project natively with Qt (ignores emscripten)
docker-build-native:
	mkdir -p build
	bash scripts/docker-build.sh build-native $(filter-out $@,$(MAKECMDGOALS))

# Build and run emscripten tests
docker-test-emscripten:
	mkdir -p build
	docker compose run --rm test-emscripten

# Build and run native tests
docker-test-native:
	mkdir -p build
	docker compose run --rm test-native

# Build and serve with web server
docker-serve:
	mkdir -p build
	bash scripts/docker-build.sh serve $(filter-out $@,$(MAKECMDGOALS))

# Interactive development shell
docker-dev:
	mkdir -p build
	bash scripts/docker-build.sh dev $(filter-out $@,$(MAKECMDGOALS))

docker-shell: docker-dev

# Build just the Docker image
docker-image:
	docker build -t cse498-companyb-project .

# Rebuild image from scratch
docker-rebuild:
	docker build --no-cache -t cse498-companyb-project .

# Forwards the display from WSL to Windows
run-native:
	DISPLAY=:0 XDG_RUNTIME_DIR=/run/user/1000 WAYLAND_DISPLAY=wayland-0 build/docker-native/app

make format:
	sudo find source tests -iname "*.cpp" -o -iname "*.hpp" ! -name "MazeWorld.hpp" ! -name "TrashInterface.hpp" ! -name "PacingAgent.hpp" | xargs clang-format-20 -i -style=Google

help:
	@echo "Top-level targets:"
	@echo "  make / make build      Build program(s) in source/"
	@echo "  make test              Build + run unit tests in tests/"
	@echo "  make all               Build program(s) + run tests"
	@echo "  make debug|opt|quick|grumpy   Build program(s) with that mode (source/)"
	@echo "  make clean             Remove /build"
	@echo
	@echo "Flags:"
	@echo "  NO_QT=1                Skip Qt. Excludes Interfaces/gui sources. (native only)"
	@echo "                         Works with: build, test, all, debug, opt, quick, grumpy"
	@echo "                         Example: make test NO_QT=1"
	echo "  TARGET_MAIN=<file>      Entry point file (searches source/ then demo/)"
	echo "                          Default: simple_main.cpp (native), web_main.cpp (emscripten)"
	@echo
	@echo "Forwarding targets:"
	@echo "  make src-<tgt>         Run 'make <tgt>' in source/"
	@echo "  make test-<tgt>        Run 'make <tgt>' in tests/"
	@echo
	@echo "Docker Build System"
	@echo "  Note: Both build targets, serve, and dev/shell accepts variants."
	@echo "  Ex: make docker-build-emscripten [default|debug|opt|quick|grumpy]"
	@echo
	@echo "  make docker-build-emscripten Build with Emscripten (outputs to build/emscripten)"
	@echo "  make docker-build-native     Build natively with Qt (outputs to build/docker-native)"
	@echo "  make docker-test-emscripten  Build + run Emscripten Catch2 tests"
	@echo "  make docker-test-native      Build + run native Catch2 tests with Qt"
	@echo "  make docker-serve            Build with Emscripten and serve the output"
	@echo "  make docker-dev              Interactive development shell"
	@echo "  make docker-shell            Alias for docker-dev"
	@echo "  make docker-image            Build the Docker image"
	@echo "  make docker-rebuild          Rebuild the Docker image without cache"
	@echo "  make run-native              Enables x11 forwarding for the GUI from WSL to a Windows host"
