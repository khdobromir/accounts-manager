.DEFAULT_GOAL := debug

VCPKG_ROOT ?= $(HOME)/vcpkg
JOBS ?= 4

CMAKE_OPTIONS := -DCMAKE_CXX_COMPILER=clang++ \
	-DCMAKE_TOOLCHAIN_FILE="$(VCPKG_ROOT)/scripts/buildsystems/vcpkg.cmake"

RUN_CONFIG := $(if $(filter release,$(MAKECMDGOALS)),release,debug)

.PHONY: debug release run

debug:
	cmake -S . -B build/clang-debug $(CMAKE_OPTIONS) -DCMAKE_BUILD_TYPE=Debug
	cmake --build build/clang-debug --parallel $(JOBS)

release:
	cmake -S . -B build/clang-release $(CMAKE_OPTIONS) -DCMAKE_BUILD_TYPE=Release
	cmake --build build/clang-release --parallel $(JOBS)

run: $(RUN_CONFIG)
	./build/clang-$(RUN_CONFIG)/accounts-manager
