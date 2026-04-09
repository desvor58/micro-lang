CC := gcc
CFLAGS := -Wall -Wno-format -Wno-missing-braces -std=c99 -Iinclude

SRC_TARGETS := src/*.c                             \
			   src/codegen/*.c                     \
			   src/codegen/386/*.c                 \
			   src/codegen/386/statements/*.c      \
			   src/codegen/386/expr_get_atoms/*.c  \
			   src/asm/*.c

SRCS := $(wildcard $(SRC_TARGETS))

ifeq ($(OS),Windows_NT)
    NUMS := $(shell for /l %%i in (1,1,$(words $(SRCS))) do @echo | set /p=%%i )
    CLEAN_CMD := del /q
	SCT_LIB_FILE := sct-win
else
    NUMS := $(shell seq 1 $(words $(SRCS)))
    CLEAN_CMD := rm -rf
	SCT_LIB_FILE := sct-elf
endif

NUMS := $(strip $(NUMS))

LDFLAGS := -Llib -l$(SCT_LIB_FILE)

OBJS := $(addprefix obj/o, $(addsuffix .o, $(NUMS)))
VPATH := $(sort $(dir $(SRCS)))

MODE ?= release

ifeq ($(MODE), debug)
	CFLAGS += -O0
else
	CFLAGS += -O3
endif

all: micro

micro: $(OBJS)
	@echo $(OBJS) > obj/o.list
	$(CC) -o bin/$@ @obj/o.list $(LDFLAGS)

define GEN_RULE
$(word $(1),$(OBJS)): $(word $(1),$(SRCS))
	$(CC) $(CFLAGS) -c $$(<) -o $$(@)
endef

$(foreach i,$(NUMS),$(eval $(call GEN_RULE,$(i))))

.PHONY: clean
clean:
	$(CLEAN_CMD) bin
	$(CLEAN_CMD) obj

	-mkdir bin
	-mkdir obj