BUILD_DIR := build
BINARY    := $(BUILD_DIR)/mion_engine

.PHONY: all configure build run clean test

all: build

configure:
	cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug

build: configure
	cmake --build $(BUILD_DIR) -j$(shell nproc)

run: build
	./$(BINARY)

test: build
	cd $(BUILD_DIR) && ctest --output-on-failure

clean:
	rm -rf $(BUILD_DIR)

# Build release (otimizado, menor binário)
release:
	cmake -B build_release -DCMAKE_BUILD_TYPE=Release
	cmake --build build_release -j$(shell nproc)
