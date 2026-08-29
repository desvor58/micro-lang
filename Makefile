CC ?= gcc

SCT_DIR := lib/sct
SCT_LIB_DIR := $(SCT_DIR)/lib
SCT_INC_DIR := $(SCT_DIR)/include

CFLAGS := -Wall -Wextra -Wshadow -Wpointer-arith  \
          -Wno-format -Wno-missing-braces -Wno-unused-parameter -Wno-unused-variable  -Wno-switch  \
          -fno-strict-aliasing  \
          -std=c99 -Iinclude -I$(SCT_INC_DIR)
LDFLAGS :=
AR := gcc-ar

MODE ?= release-fast

ifeq ($(MODE), debug)
	CFLAGS += -O0 -g
endif
ifeq ($(MODE), release-fast)
	CFLAGS += -O3 -ffast-math
endif
ifeq ($(MODE), release-size)
	CFLAGS += -Os
endif

OBJDIR := obj/$(MODE)

ifeq ($(OS),Windows_NT)
    RM_DIR = if exist "$(subst /,\,$(1))" rmdir /s /q "$(subst /,\,$(1))"
    RM_FILE = if exist "$(subst /,\,$(1))" del /q /f "$(subst /,\,$(1))"
    FIX_PATH = $(subst /,\,$(1))
    MKDIR = if not exist "$(call FIX_PATH,$(1))" mkdir "$(call FIX_PATH,$(1))"
    SCT_LIB_FILE := sct-win
    EXE_EXT := .exe

    SCT_SM_CHECK := @if not exist "$@" (git submodule update --init --recursive --remote --merge)
    SCT_CLEAN    := @if exist "$(SCT_DIR)\Makefile" $(MAKE) -C $(SCT_DIR) clean
else
    RM_DIR = rm -rf "$(1)"
    RM_FILE = rm -f "$(1)"
    FIX_PATH = $(1)
    MKDIR = mkdir -p "$(1)"
    SCT_LIB_FILE := sct-elf
    EXE_EXT :=

    ifeq ($(MODE),debug)
        CFLAGS +=  -fsanitize=address
        LDFLAGS += -fsanitize=address
    endif

    SCT_SM_CHECK := @if [ ! -f "$@" ]; then git submodule update --init --recursive --remote --merge || (exit 1;); fi
    SCT_CLEAN    := @if [ -f "$(SCT_DIR)/Makefile" ]; then $(MAKE) -C $(SCT_DIR) clean; fi
endif

ifeq ($(CC),clang)
    TARGET_FLAGS := --target=x86_64-w64-windows-gnu
    CFLAGS += $(TARGET_FLAGS)
    LDFLAGS += $(TARGET_FLAGS) -fuse-ld=lld
    AR := llvm-ar
endif

MICRO_SRC := src/common.c \
             src/instr.c \
             src/asm/*.c \
             src/codegen/*.c \
             src/codegen/386/*.c \
             src/codegen/386/lowering/*.c \
             src/codegen/386/lowering/expr_ops/*.c \

MICRO_SRC := $(wildcard $(MICRO_SRC))

MICROC_SRCS := src/microc/lexer.c \
               src/microc/instrgen/genfuns.c \
               src/microc/instrgen/instrgen.c \
               src/microc/instrgen/statements/*.c \

MICROC_SRCS := $(wildcard $(MICROC_SRCS))

MICRO_OBJS := $(patsubst src/%.c, $(OBJDIR)/%.o, $(MICRO_SRC))
MICROC_OBJS := $(patsubst src/%.c, $(OBJDIR)/%.o, $(MICROC_SRCS))
DEPS := $(MICRO_OBJS:.o=.d) $(MICROC_OBJS:.o=.d)

TEST_CFLAGS := $(CFLAGS) -Itests/include -I$(SCT_INC_DIR) -O3
MICROC_LDFLAGS := $(LDFLAGS) -L$(SCT_LIB_DIR) -l$(SCT_LIB_FILE)
TEST_LDFLAGS := $(LDFLAGS) -L$(SCT_LIB_DIR) -l$(SCT_LIB_FILE)

EXAMPLES_SRCS := $(wildcard examples/*/main.c)
EXAMPLES_BINS := $(patsubst examples/%/main.c, bin/examples/%, $(EXAMPLES_SRCS))
EXAMPLES_CFLAGS := $(CFLAGS) -Iinclude -I$(SCT_INC_DIR) -O3

.PHONY: all libmicro microc test test-debug test-release _run_tests examples clean SCT

all: microc libmicro

libmicro: SCT $(MICRO_OBJS)
	@$(call MKDIR,lib)
	$(AR) rcs lib/libmicro.a $(MICRO_OBJS) $(SCT_LIB_DIR)/lib$(SCT_LIB_FILE).a

microc: SCT $(MICRO_OBJS) $(MICROC_OBJS) src/microc/microc.c
	@$(call MKDIR,bin)
	$(CC) $(CFLAGS) src/microc/microc.c $(MICROC_OBJS) $(MICRO_OBJS) -o bin/microc$(EXE_EXT) $(MICROC_LDFLAGS)

examples: SCT $(EXAMPLES_BINS)

SCT: $(SCT_DIR)/Makefile
	@$(MAKE) -C $(SCT_DIR) CC=$(CC) MODE=$(MODE)

$(SCT_DIR)/Makefile:
	$(SCT_SM_CHECK)

bin/examples/%: examples/%/main.c $(MICRO_OBJS) $(MICROC_OBJS)
	@$(call MKDIR,bin/examples)
	$(CC) $(EXAMPLES_CFLAGS) $< $(MICROC_OBJS) $(MICRO_OBJS) -o $@$(EXE_EXT) $(TEST_LDFLAGS)

test: SCT test-debug test-release

test-debug:
	$(MAKE) MODE=debug _run_tests

test-release:
	$(MAKE) MODE=release-fast _run_tests

_run_tests: tests/bin/$(MODE)/tests$(EXE_EXT)
	.$(call FIX_PATH,/tests/bin/$(MODE)/tests$(EXE_EXT)) $(TEST_FLAGS)

tests/bin/$(MODE)/tests$(EXE_EXT): $(MICRO_OBJS) $(MICROC_OBJS) tests/src/munit.c tests/src/main.c tests/src/*.h
	@$(call MKDIR,tests/bin/$(MODE))
	$(CC) $(TEST_CFLAGS) tests/src/munit.c tests/src/main.c $(MICROC_OBJS) $(MICRO_OBJS) -o tests/bin/$(MODE)/tests$(EXE_EXT) $(TEST_LDFLAGS)

$(OBJDIR)/%.o: src/%.c
	@$(call MKDIR,$(dir $@))
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	@$(call RM_DIR,obj)
	@$(call RM_DIR,bin)
	@$(call RM_DIR,tests/bin)
	@$(call RM_FILE,lib/libmicro.a)
	@$(call MKDIR,bin)
	@$(call MKDIR,obj)
	@$(call MKDIR,tests/bin)
	@$(call RM_DIR,lib/sct)
