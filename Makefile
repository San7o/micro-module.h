# SPDX-License-Identifier: MIT
# Author:  Giovanni Santini
# Mail:    giovanni.santini@proton.me
# License: MIT

#
# Compiler flags
#
CFLAGS       = -Wall -Werror -Wpedantic -std=c99
DEBUG_FLAGS  = -ggdb
MODULE_FLAGS = -fPIC -shared
LDFLAGS      = -ldl
CC?          = gcc

#
# Project files
#
OUT_NAME = example
OBJ      = example.o
MODULE_SRC := $(wildcard example_modules/example_module*.c)
MODULE_OBJ := $(patsubst example_modules/%.c,example_modules/compiled/%.so,$(MODULE_SRC))

#
# Commands
#
all: examples $(OUT_NAME)

debug: CFLAGS += $(DEBUG_FLAGS)
debug: examples $(OUT_NAME)

run: examples $(OUT_NAME)
	chmod +x $(OUT_NAME)
	./$(OUT_NAME)

examples: $(MODULE_OBJ)

clean:
	rm -f $(OBJ)

distclean: clean
	rm -f $(OUT_NAME) $(MODULE_NAME)

$(OUT_NAME): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) $(CFLAGS) -o $(OUT_NAME)

$(MODULE_NAME): $(MODULE_SOURCE)
	$(CC) $(MODULE_SOURCE) $(LDFLAGS) $(CFLAGS) $(MODULE_FLAGS) -o $(MODULE_NAME)

example_modules/compiled/%.so: example_modules/%.c
	$(CC) $< $(LDFLAGS) $(CFLAGS) $(MODULE_FLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
