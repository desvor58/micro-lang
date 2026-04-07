CC := gcc
CFLAGS := -Wall -Wno-format -Wno-missing-braces -std=c99 -Iinclude

SRC_TARGETS := src/c/*.c  \
			   src/codegen/386/c/*.c  \
			   src/codegen/386/c/statements/*.c  \
			   src/codegen/386/c/expr_get_atoms/*.c  \
			   src/codegen/c/*.c  \
			   src/asm/*.c  \
			   src/main.c

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

all: micro

micro: $(OBJS)
	@echo $(OBJS) > obj/o.list
	$(CC) -o bin/$@ @obj/o.list $(LDFLAGS)

debug: clean micro

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