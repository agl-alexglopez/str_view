.PHONY: default install build gcc-rel gcc-deb clang-rel clang-deb test-deb test-rel clean

MAKE := $(MAKE)
MAKEFLAGS += --no-print-directory
# Adjust parallel build jobs based on your available cores.
JOBS ?= $(shell (command -v nproc > /dev/null 2>&1 && echo "-j$$(nproc)") || echo "")
BUILD_DIR := build/
PREFIX := install/

ifeq ($(words $(MAKECMDGOALS)),2)
  PREFIX := $(word 2, $(MAKECMDGOALS))
endif

default: build

build:
	cmake --build $(BUILD_DIR) $(JOBS)

install:
	cmake --build $(BUILD_DIR) --target install $(JOBS)

gcc-rel:
	cmake --preset=gcc-rel -DCMAKE_INSTALL_PREFIX=$(PREFIX)
	$(MAKE) build

gcc-deb:
	cmake --preset=gcc-deb -DCMAKE_INSTALL_PREFIX=$(PREFIX)
	$(MAKE) build

clang-rel:
	cmake --preset=clang-rel -DCMAKE_INSTALL_PREFIX=$(PREFIX)
	$(MAKE) build

clang-deb:
	cmake --preset=clang-deb -DCMAKE_INSTALL_PREFIX=$(PREFIX)
	$(MAKE) build

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
