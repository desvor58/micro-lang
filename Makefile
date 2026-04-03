СС := gcc
CFLAGS := -Wall -Wno-format -Wno-missing-braces -std=c99 -Iinclude

SRC_TARGETS := src/c/*.c  \
			   src/codegen/386/c/*.c  \
			   src/codegen/386/c/statements/*.c  \
			   src/codegen/386/c/expr_get_atoms/*.c  \
			   src/codegen/c/*.c  \
			   src/asm/*.c  \
			   src/micro.c

SRCS := $(wildcard $(SRC_TARGETS))
OBJS := $(addprefix obj/, $(notdir $(SRCS:.c=.o)))
VPATH := $(sort $(dir $(SRCS)))

ifeq ($(OS),Windows_NT)
    CLEAN_CMD := del /q
	SCT_LIB_FILE := sct-win
else
    CLEAN_CMD=rm -rf
	SCT_LIB_FILE := sct-elf
endif

LDFLAGS := -Llib -l$(SCT_LIB_FILE)

all: clean micro

micro: $(OBJS)
	$(CC) -o bin/$@ $(OBJS) $(LDFLAGS)

obj/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
	
.PHONY: clean
clean:
	$(CLEAN_CMD) bin
	$(CLEAN_CMD) obj

	-mkdir bin
	-mkdir obj