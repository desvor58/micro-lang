CC ?= gcc
CFLAGS := -Wall -Wno-format -Wno-missing-braces -std=c99 -Iinclude
TEST_CFLAGS := $(CFLAGS) -Itests/include -O3
TEST_FLAGS ?= --iterations 5
TEST_LDFLAGS := -Llib -lmicro
LIBUTIL_FMT = ar rcs $(1) $(2)

SRC_TARGETS := src/*.c                             \
			   src/types/*.c					   \
			   src/asm/*.c
# 			   src/codegen/*.c                     \
# 			   src/codegen/386/*.c                 \
# 			   src/codegen/386/statements/*.c      \
# 			   src/codegen/386/expr_get_atoms/*.c
SRCS := $(wildcard $(SRC_TARGETS))

ifeq ($(OS),Windows_NT)
	NUMS := $(shell for /l %%i in (1,1,$(words $(SRCS))) do @echo | set /p=%%i )
	CLEAN_CMD := del /q
	MKDIR_CMD = if not exist "$(1)" mkdir "$(1)"
	SCT_LIB_FILE := sct-win
else
	NUMS := $(shell seq 1 $(words $(SRCS)))
	CLEAN_CMD := rm -rf
	MKDIR_CMD = mkdir -p "$(1)"
	SCT_LIB_FILE := sct-elf
endif

NUMS := $(strip $(NUMS))

LDFLAGS := -Llib -l$(SCT_LIB_FILE)

ifeq ($(CC),clang)
	LDFLAGS += --target=x86_64-w64-windows-gnu
	CFLAGS += --target=x86_64-w64-windows-gnu
	TEST_LDFLAGS += --target=x86_64-w64-windows-gnu
	TEST_CFLAGS += --target=x86_64-w64-windows-gnu
endif

OBJS := $(addprefix obj/o, $(addsuffix .o, $(NUMS)))
VPATH := $(sort $(dir $(SRCS)))

MODE ?= release

ifeq ($(MODE), debug)
	CFLAGS += -O0 -g
else
	CFLAGS += -O3
endif

all: microc

dbg_build: clean microc

libmicro: $(OBJS)
	@echo $(OBJS) > obj/o.list
	$(call LIBUTIL_FMT,"lib/libmicro.a","@obj/o.list")

microc: $(OBJS)
	@echo $(OBJS) > obj/o.list
	$(CC) $(CFLAGS) -o bin/$@ @obj/o.list src/microc/microc.c $(LDFLAGS)

define GEN_RULE
$(word $(1),$(OBJS)): $(word $(1),$(SRCS))
	$(CC) $(CFLAGS) -c $$(<) -o $$(@)
endef

$(foreach i,$(NUMS),$(eval $(call GEN_RULE,$(i))))

test: test_comp
	./tests/bin/tests $(TEST_FLAGS)

test_comp: libmicro
	$(CC) $(TEST_CFLAGS) -o tests/bin/tests tests/src/munit.c tests/src/main.c $(TEST_LDFLAGS)

.PHONY: clean
clean:
	$(CLEAN_CMD) bin
	$(CLEAN_CMD) obj
	$(CLEAN_CMD) "tests/bin"

	$(call MKDIR_CMD,bin)
	$(call MKDIR_CMD,obj)
	$(call MKDIR_CMD,tests/bin)