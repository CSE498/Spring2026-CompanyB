.PHONY: build test serve dev shell clean image rebuild

# Build the project
build:
	docker compose run --rm build

# Build and run tests
test:
	docker compose run --rm test

# Build and serve with web server
serve:
	docker compose up serve

# Interactive development shell
dev:
	docker compose run --rm dev

shell: dev

# Build just the Docker image
image:
	docker build -t cse498-companyb-project .

# Clean output directory
clean:
	rm -rf output/*

# Rebuild image from scratch
rebuild:
	docker build --no-cache -t cse498-companyb-project .
