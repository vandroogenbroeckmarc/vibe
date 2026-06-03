# ViBe (C implementation) — modernized Makefile (2026).
#
# Targets:
#   make             -> build libvibe.a and the two example programs
#   make main        -> build the pure-C example only
#   make main-opencv -> build the OpenCV-based example (requires OpenCV 4)
#   make clean       -> remove all build artifacts
#   make install     -> install the static library + header under PREFIX
#
# Variables (override on the command line):
#   PREFIX      install prefix (default: /usr/local)
#   CC          C compiler (default: cc)
#   CXX         C++ compiler (default: c++)
#   PKG_CONFIG  pkg-config binary (default: pkg-config)
#   CFLAGS      extra C flags
#   CXXFLAGS    extra C++ flags
#
# Notes:
#   * OpenCV 4 ships its pkg-config module under the name "opencv4"; this
#     Makefile auto-detects "opencv4" first and falls back to "opencv".
#   * -Werror has been removed: new compiler warnings in GCC/Clang would
#     otherwise turn a benign note into a build-breaking failure.

PREFIX      ?= /usr/local
CC          ?= cc
CXX         ?= c++
PKG_CONFIG  ?= pkg-config
AR          ?= ar

# Auto-detect OpenCV 4 (fallback to the legacy "opencv" module).
OPENCV_PKG    := $(shell $(PKG_CONFIG) --exists opencv4 && echo opencv4 || echo opencv)
OPENCV_CFLAGS := $(shell $(PKG_CONFIG) --cflags $(OPENCV_PKG) 2>/dev/null)
OPENCV_LIBS   := $(shell $(PKG_CONFIG) --libs   $(OPENCV_PKG) 2>/dev/null)

CSTD    ?= -std=c11
CXXSTD  ?= -std=c++17
WARN    ?= -Wall -Wextra -Wpedantic \
           -Wno-unused-function -Wno-unused-parameter \
           -Wno-deprecated -Wno-deprecated-declarations
OPT     ?= -O3

CFLAGS   += $(CSTD) $(OPT) $(WARN) -fPIC
CXXFLAGS += $(CXXSTD) $(OPT) $(WARN) -fPIC

LIB_OBJ    = vibe-background-sequential.o
LIB_STAT   = libvibe.a
EXE_C      = main
EXE_CPP    = main-opencv

.PHONY: all clean install help
all: $(LIB_STAT) $(EXE_C) $(EXE_CPP)

# Library (static archive).
$(LIB_OBJ): vibe-background-sequential.c vibe-background-sequential.h
	$(CC) $(CFLAGS) -c vibe-background-sequential.c -o $@

$(LIB_STAT): $(LIB_OBJ)
	$(AR) rcs $@ $(LIB_OBJ)

# Pure-C example.
$(EXE_C): your-main-file-sequential.c $(LIB_OBJ)
	$(CC) $(CFLAGS) -o $@ your-main-file-sequential.c $(LIB_OBJ)

# OpenCV-based example.
$(EXE_CPP): main-opencv.cpp $(LIB_OBJ)
	@if [ -z "$(OPENCV_LIBS)" ]; then \
	    echo "ERROR: OpenCV not found via pkg-config (tried '$(OPENCV_PKG)')."; \
	    echo "       Install OpenCV 4 or set PKG_CONFIG_PATH appropriately."; \
	    exit 1; \
	fi
	$(CXX) $(CXXFLAGS) $(OPENCV_CFLAGS) -o $@ main-opencv.cpp $(LIB_OBJ) $(OPENCV_LIBS)

install: $(LIB_STAT)
	install -d $(DESTDIR)$(PREFIX)/lib $(DESTDIR)$(PREFIX)/include
	install -m 0644 $(LIB_STAT) $(DESTDIR)$(PREFIX)/lib/
	install -m 0644 vibe-background-sequential.h $(DESTDIR)$(PREFIX)/include/

clean:
	rm -f $(LIB_OBJ) $(LIB_STAT) $(EXE_C) $(EXE_CPP)

help:
	@echo "ViBe (C implementation) — make targets:"
	@echo ""
	@echo "  make             Build libvibe.a + main + main-opencv (default)."
	@echo "  make main        Build the pure-C example only."
	@echo "  make main-opencv Build the OpenCV-based example only."
	@echo "  make install     Install libvibe.a + header under PREFIX."
	@echo "  make clean       Remove all build artifacts."
	@echo "  make help        Show this help."
	@echo ""
	@echo "Detected OpenCV pkg-config module: $(OPENCV_PKG)"
