# Thin Makefile wrapper around the CMake build for libvibe++ (modernized).
#
# Convenience targets:
#   make           -> build
#   make build     -> configure + build via CMake (idempotent)
#   make demo      -> run ViBe_8UC1 on the bundled input/driveway-320x240.avi
#                     clip with --no-median (interactive display)
#   make demo-benchmark
#                  -> run the same clip in --benchmark mode for reproducible
#                     timing (--benchmark --max-frames 500 --seed 42)
#   make clean     -> remove the build directory
#   make help      -> list available targets
#
# All paths are relative to this directory so the project stays
# self-contained; the .avi clip lives in input/driveway-320x240.avi.

BUILD_DIR      ?= build
CMAKE          ?= cmake
CMAKE_GEN      ?=

DEMO_VIDEO     := input/driveway-320x240.avi
DEMO_EXE       := $(BUILD_DIR)/bin/ViBe_8UC1
DEMO_EXE_COLOR := $(BUILD_DIR)/bin/ViBe_8UC3

.PHONY: all build demo demo-benchmark demo-color clean help check-video

all: build

$(BUILD_DIR)/CMakeCache.txt:
	$(CMAKE) -S . -B $(BUILD_DIR) $(CMAKE_GEN)

build: $(BUILD_DIR)/CMakeCache.txt
	$(CMAKE) --build $(BUILD_DIR) --parallel

# Private helper: verify the bundled clip is present.
check-video:
	@if [ ! -f "$(DEMO_VIDEO)" ]; then \
	  echo ""; \
	  echo "ERROR: $(DEMO_VIDEO) not found."; \
	  echo ""; \
	  echo "Drop 'driveway-320x240.avi' into the 'input/' folder of"; \
	  echo "this directory, then re-run the make target you just invoked."; \
	  echo ""; \
	  exit 2; \
	fi

demo: build check-video
	./$(DEMO_EXE) --no-median $(DEMO_VIDEO)

demo-color: build check-video
	./$(DEMO_EXE_COLOR) --no-median $(DEMO_VIDEO)

demo-benchmark: build check-video
	./$(DEMO_EXE) --benchmark --max-frames 500 --seed 42 $(DEMO_VIDEO)

clean:
	rm -rf $(BUILD_DIR)

help:
	@echo "libvibe++ (modernized) — make targets:"
	@echo ""
	@echo "  make               Equivalent to 'make build'."
	@echo "  make build         Configure + build via CMake (idempotent)."
	@echo "  make demo          Run ViBe_8UC1 on input/driveway-320x240.avi"
	@echo "                     with --no-median (interactive display)."
	@echo "  make demo-color    Same as 'make demo' but uses the color"
	@echo "                     ViBe_8UC3 binary."
	@echo "  make demo-benchmark"
	@echo "                     Run the benchmark report on the same clip:"
	@echo "                     --benchmark --max-frames 500 --seed 42."
	@echo "  make clean         Remove the build directory."
	@echo "  make help          Show this help."
