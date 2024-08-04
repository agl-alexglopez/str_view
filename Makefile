.PHONY: default install build gcc-rel gcc-deb clang-rel clang-deb tests samples gcc-all-deb gcc-all-rel clang-all-deb clang-all-rel str_view test-deb test-rel clean

MAKE := $(MAKE) -f Makefile
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

str_view:
	cmake --preset=default-rel -DCMAKE_INSTALL_PREFIX=$(PREFIX)
	cmake --build $(BUILD_DIR) --target install $(JOBS)

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

tests:
	cmake --build $(BUILD_DIR) --target tests $(JOBS)

samples:
	cmake --build $(BUILD_DIR) --target samples $(JOBS)

gcc-all-deb:
	$(MAKE) gcc-deb
	$(MAKE) tests
	$(MAKE) samples

gcc-all-rel:
	$(MAKE) gcc-rel
	$(MAKE) tests
	$(MAKE) samples

clang-all-deb:
	$(MAKE) clang-deb
	$(MAKE) tests
	$(MAKE) samples

clang-all-rel:
	$(MAKE) clang-rel
	$(MAKE) tests
	$(MAKE) samples

test-deb: tests
	$(BUILD_DIR)debug/bin/run_tests $(BUILD_DIR)debug/bin/tests/
	@echo "RAN TESTS"

test-rel: tests
	$(BUILD_DIR)bin/run_tests $(BUILD_DIR)bin/tests/
	@echo "RAN TESTS"

clean:
	rm -rf build/ install/
