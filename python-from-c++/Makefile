# Convenience Makefile for the Python port of libvibe++.
#
# Convenience targets:
#   make help            -> list targets (default)
#   make install         -> pip-install the Python dependencies
#   make demo            -> run main.py on the bundled
#                           input/driveway-320x240.avi clip with
#                           --no-median --color (interactive color display)
#   make demo-gray       -> same clip, grayscale variant (no --color)
#   make demo-benchmark  -> run benchmark report on the same clip
#                           (--benchmark --max-frames 500 --seed 42)
#   make clean           -> remove Python caches
#
# All paths are relative to this directory so the project stays
# self-contained; the .avi clip lives in input/driveway-320x240.avi.

PYTHON      ?= python3
PIP         ?= $(PYTHON) -m pip

DEMO_VIDEO  := input/driveway-320x240.avi
DEMO_MAIN   := main.py

.PHONY: help install demo demo-gray demo-benchmark clean check-video

help:
	@echo "libvibe++ Python port — make targets:"
	@echo ""
	@echo "  make install          Pip-install requirements.txt."
	@echo "  make demo             Run main.py on input/driveway-320x240.avi"
	@echo "                        with --no-median --color (color display)."
	@echo "  make demo-gray        Same clip without --color (grayscale)."
	@echo "  make demo-benchmark   Run benchmark report on the same clip:"
	@echo "                        --benchmark --max-frames 500 --seed 42."
	@echo "  make clean            Remove Python __pycache__ directories."
	@echo "  make help             Show this help (default target)."

install:
	$(PIP) install -r requirements.txt

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

demo: check-video
	$(PYTHON) $(DEMO_MAIN) --no-median --color $(DEMO_VIDEO)

demo-gray: check-video
	$(PYTHON) $(DEMO_MAIN) --no-median $(DEMO_VIDEO)

demo-benchmark: check-video
	$(PYTHON) $(DEMO_MAIN) --benchmark --max-frames 500 --seed 42 $(DEMO_VIDEO)

clean:
	find . -name '__pycache__' -type d -prune -exec rm -rf {} + 2>/dev/null || true
	find . -name '*.pyc' -delete 2>/dev/null || true
