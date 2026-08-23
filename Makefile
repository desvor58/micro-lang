CC ?= gcc
CFLAGS := -Wall -Wextra -Wshadow -Wpointer-arith  \
          -Wno-format -Wno-missing-braces -Wno-unused-parameter -Wno-unused-variable  -Wno-switch  \
          -fno-strict-aliasing  \
          -std=c99 -Iinclude
LDFLAGS :=
MODE ?= release
AR := gcc-ar

ifeq ($(MODE),debug)
    CFLAGS += -O0 -g
else
    CFLAGS += -O3 -flto
    LDFLAGS += -flto
endif

OBJDIR := obj/$(MODE)

ifeq ($(OS),Windows_NT)
    RM_DIR = if exist "$(subst /,\,$(1))" rmdir /s /q "$(subst /,\,$(1))"
    RM_FILE = if exist "$(subst /,\,$(1))" del /q /f "$(subst /,\,$(1))"
    FIX_PATH = $(subst /,\,$(1))
    MKDIR = if not exist "$(call FIX_PATH,$(1))" mkdir "$(call FIX_PATH,$(1))"
    SCT_LIB_FILE := sct-win
    EXE_EXT := .exe
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
endif

ifeq ($(CC),clang)
    TARGET_FLAGS := --target=x86_64-w64-windows-gnu
    CFLAGS += $(TARGET_FLAGS)
    LDFLAGS += $(TARGET_FLAGS) -fuse-ld=lld
    AR := llvm-ar
endif

SRC_TARGETS := src/*.c \
               src/instrgen/*.c \
               src/instrgen/statements/*.c \
               src/asm/*.c \
               src/codegen/*.c \
               src/codegen/386/*.c \
               src/codegen/386/lowering/*.c \
               src/codegen/386/lowering/expr_ops/*.c \

SRCS := $(wildcard $(SRC_TARGETS))

SRCS := $(filter-out src/microc/microc.c, $(SRCS))
OBJS := $(patsubst src/%.c, $(OBJDIR)/%.o, $(SRCS))
DEPS := $(OBJS:.o=.d)

TEST_CFLAGS := $(CFLAGS) -Itests/include -O3
MICROC_LDFLAGS := $(LDFLAGS) -Llib -l$(SCT_LIB_FILE)
TEST_LDFLAGS := $(LDFLAGS) -Llib -l$(SCT_LIB_FILE)

.PHONY: all libmicro microc test test-debug test-release _run_tests clean

all: microc

libmicro: $(OBJS)
	@$(call MKDIR,lib)
	$(AR) rcs lib/libmicro.a $(OBJS)

microc: $(OBJS) src/microc/microc.c
	@$(call MKDIR,bin)
	$(CC) $(CFLAGS) src/microc/microc.c $(OBJS) -o bin/microc$(EXE_EXT) $(MICROC_LDFLAGS)

test: test-debug test-release
	@echo "All tests passed (debug + release)"

test-debug:
	$(MAKE) MODE=debug _run_tests

test-release:
	$(MAKE) MODE=release _run_tests

_run_tests: tests/bin/$(MODE)/tests$(EXE_EXT)
	.$(call FIX_PATH,/tests/bin/$(MODE)/tests$(EXE_EXT)) $(TEST_FLAGS)

tests/bin/$(MODE)/tests$(EXE_EXT): $(OBJS) tests/src/munit.c tests/src/main.c
	@$(call MKDIR,tests/bin/$(MODE))
	$(CC) $(TEST_CFLAGS) tests/src/munit.c tests/src/main.c $(OBJS) -o tests/bin/$(MODE)/tests$(EXE_EXT) $(TEST_LDFLAGS)

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
