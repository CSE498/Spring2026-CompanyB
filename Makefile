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

.PHONY: default all build test clean debug opt quick grumpy \
        src-% test-% help docker-build-emscripten docker-build-native \
        docker-test-emscripten docker-test-native \
        docker-serve docker-dev docker-shell docker-clean \
        docker-image docker-rebuild

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

# Program build variants (forwarded to source/)
debug opt quick grumpy:
	$(MAKE) -C source $@

# Clean everything
clean:
	$(MAKE) -C source clean
	$(MAKE) -C tests clean

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
	docker compose run --rm build-emscripten

# Build the project natively with Qt (ignores emscripten)
docker-build-native:
	docker compose run --rm build-native

# Build and run emscripten tests
docker-test-emscripten:
	docker compose run --rm test-emscripten

# Build and run native tests
docker-test-native:
	docker compose run --rm test-native

# Build and serve with web server
docker-serve:
	docker compose up serve

# Interactive development shell
docker-dev:
	docker compose run --rm dev

docker-shell: docker-dev

# Build just the Docker image
docker-image:
	docker build -t cse498-companyb-project .

# Clean output directory
docker-clean:
	rm -rf build/*

# Rebuild image from scratch
docker-rebuild:
	docker build --no-cache -t cse498-companyb-project .

help:
	@echo "Top-level targets:"
	@echo "  make / make build      Build program(s) in source/"
	@echo "  make test              Build + run unit tests in tests/"
	@echo "  make all               Build program(s) + run tests"
	@echo "  make debug|opt|quick|grumpy   Build program(s) with that mode (source/)"
	@echo "  make clean             Clean source/ and tests/"
	@echo
	@echo "Forwarding targets:"
	@echo "  make src-<tgt>         Run 'make <tgt>' in source/"
	@echo "  make test-<tgt>        Run 'make <tgt>' in tests/"
	@echo
	@echo "Docker Build System"
	@echo "  make docker-build              Build with Emscripten (outputs to /build/emscripten)"
	@echo "  make docker-build-native       Build natively with Qt"
	@echo "  make docker-test-emscripten    Build + run Emscripten Catch2 tests"
	@echo "  make docker-test-native        Build + run native Catch2 tests with Qt"
	@echo "  make docker-serve              Build with Emscripten and serve the output"
	@echo "  make docker-dev                Interactive development shell"
	@echo "  make docker-shell              Alias for docker-dev"
	@echo "  make docker-image              Build the Docker image"
	@echo "  make docker-clean              Clean the output directory"
	@echo "  make docker-rebuild            Rebuild the Docker image without cache"
