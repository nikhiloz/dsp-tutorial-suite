# Makefile - Simplified build for C-only FFT-DSP Toolkit
# Requirements: gcc/clang, make

CC ?= gcc
CFLAGS := -Wall -Wextra -Werror -std=c99 -Iinclude -fPIC
CFLAGS_DEBUG := $(CFLAGS) -g -O0 -DDEBUG
CFLAGS_RELEASE := $(CFLAGS) -O3 -DNDEBUG
LDFLAGS := -lm

# Build directories
BUILD_DIR := build
BIN_DIR := $(BUILD_DIR)/bin
LIB_DIR := $(BUILD_DIR)/lib
OBJ_DIR := $(BUILD_DIR)/obj

# Source files
SOURCES := src/fft.c src/filter.c src/dsp_utils.c
OBJECTS := $(patsubst src/%.c, $(OBJ_DIR)/%.o, $(SOURCES))

EXAMPLES := examples/fft_demo.c examples/filter_demo.c
TESTS := tests/test_fft.c tests/test_filter.c

# Targets
all: release

# Create directories
$(BUILD_DIR) $(BIN_DIR) $(LIB_DIR) $(OBJ_DIR):
	mkdir -p $@

# Object compilation
$(OBJ_DIR)/%.o: src/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS_RELEASE) -c $< -o $@

# Debug build
debug: CFLAGS_RELEASE = $(CFLAGS_DEBUG)
debug: $(OBJ_DIR) $(BIN_DIR) $(LIB_DIR) \
	$(BIN_DIR)/fft_demo \
	$(BIN_DIR)/filter_demo \
	$(BIN_DIR)/test_fft \
	$(BIN_DIR)/test_filter

# Release build
release: $(OBJ_DIR) $(BIN_DIR) $(LIB_DIR) \
	$(BIN_DIR)/fft_demo \
	$(BIN_DIR)/filter_demo \
	$(BIN_DIR)/test_fft \
	$(BIN_DIR)/test_filter

# Static library
$(LIB_DIR)/libfft_dsp.a: $(OBJECTS)
	ar rcs $@ $^

# Shared library
$(LIB_DIR)/libfft_dsp.so: $(OBJECTS)
	$(CC) -shared -fPIC $(OBJECTS) $(LDFLAGS) -o $@

# Examples
$(BIN_DIR)/fft_demo: examples/fft_demo.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) $< $(OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/filter_demo: examples/filter_demo.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) $< $(OBJECTS) $(LDFLAGS) -o $@

# Tests
$(BIN_DIR)/test_fft: tests/test_fft.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) -Itests $< $(OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/test_filter: tests/test_filter.c $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) -Itests $< $(OBJECTS) $(LDFLAGS) -o $@

# Run tests
test: $(BIN_DIR)/test_fft $(BIN_DIR)/test_filter
	@echo "=== Running FFT tests ==="
	$(BIN_DIR)/test_fft
	@echo "\n=== Running Filter tests ==="
	$(BIN_DIR)/test_filter

# Run examples
run: $(BIN_DIR)/fft_demo $(BIN_DIR)/filter_demo
	@echo "=== Running FFT demo ==="
	$(BIN_DIR)/fft_demo
	@echo "\n=== Running Filter demo ==="
	$(BIN_DIR)/filter_demo

# Code formatting
format:
	clang-format -i src/*.c include/*.h examples/*.c tests/test_*.c

# Static analysis
lint:
	clang-tidy src/*.c -- -Iinclude

# Memory checking
memcheck: debug
	valgrind --leak-check=full --error-exitcode=1 $(BIN_DIR)/test_fft
	valgrind --leak-check=full --error-exitcode=1 $(BIN_DIR)/test_filter

# Profiling (Linux only)
profile: $(BIN_DIR)/fft_demo
	perf record -g $(BIN_DIR)/fft_demo
	perf report

# Clean
clean:
	rm -rf $(BUILD_DIR)

# Deep clean
distclean: clean
	find . -name "*.o" -o -name "*.a" -o -name "*.so" -o -name "perf.data*" | xargs rm -f

# Install
install: release
	@echo "Installing to /usr/local..."
	mkdir -p /usr/local/include/fft_dsp /usr/local/lib
	cp include/*.h /usr/local/include/fft_dsp/
	cp $(LIB_DIR)/* /usr/local/lib/
	ldconfig

# Help
help:
	@echo "FFT-DSP Toolkit Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  make release     - Build release version (default)"
	@echo "  make debug       - Build debug version with symbols"
	@echo "  make test        - Run unit tests"
	@echo "  make run         - Run examples"
	@echo "  make memcheck    - Run tests with valgrind"
	@echo "  make profile     - Profile fft_demo with perf"
	@echo "  make format      - Format code with clang-format"
	@echo "  make lint        - Static analysis with clang-tidy"
	@echo "  make install     - Install headers & libraries to /usr/local"
	@echo "  make clean       - Remove build directory"
	@echo "  make distclean   - Remove all generated files"
	@echo "  make help        - Show this help message"

.PHONY: all debug release test run memcheck profile format lint clean distclean install help
