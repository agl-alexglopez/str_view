.PHONY: default install build grel gdeb crel cdeb test-deb test-rel clean

MAKE := $(MAKE)
MAKEFLAGS += --no-print-directory
# Adjust parallel build jobs based on your available cores.
JOBS ?= $(shell (command -v nproc > /dev/null 2>&1 && echo "-j$$(nproc)") || echo "")
BUILD_DIR := build/

default: build

build:
	cmake --build $(BUILD_DIR) $(JOBS)

install:
	cmake --build $(BUILD_DIR) --target install $(JOBS)

grel:
	cmake --preset=grel
	$(MAKE) install

gdeb:
	cmake --preset=gdeb
	$(MAKE) install

crel:
	cmake --preset=crel
	$(MAKE) install

cdeb:
	cmake --preset=cdeb
	$(MAKE) install

format:
	cmake --build $(BUILD_DIR) --target format

tidy:
	cmake --build $(BUILD_DIR) --target tidy $(JOBS)

test-deb: build
	$(BUILD_DIR)deb/run_tests $(BUILD_DIR)deb/tests/
	@echo "RAN TESTS"

test-rel: build
	$(BUILD_DIR)rel/run_tests $(BUILD_DIR)rel/tests/
	@echo "RAN TESTS"

clean:
	rm -rf build/ install/
