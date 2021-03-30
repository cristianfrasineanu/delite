INCLUDE_DIR=inc
SRC_DIR=src
OUT_DIR=bin
CC=gcc
CFLAGS=-I$(INCLUDE_DIR) \
	   -DUSE_COLOR_TABLE
LINK_FLAGS = -lm

# Actual list of files.
_HEADERS = bitmap.h
_OBJECT_FILES = main.o bitmap.o

# Generate full path.
HEADERS = $(patsubst %,$(INCLUDE_DIR)/%,$(_HEADERS))
OBJECT_FILES = $(patsubst %,$(OUT_DIR)/%,$(_OBJECT_FILES))

ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

$(OUT_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(OUT_DIR)/delite: $(OBJECT_FILES)
	$(CC) -o $@ $^ $(CFLAGS) $(LINK_FLAGS)

.PHONY: clean
clean:
	rm -f $(OUT_DIR)/*.o $(OUT_DIR)/delite

.PHONY: install
install: $(OUT_DIR)/delite
	sudo install -m 777 $(OUT_DIR)/delite $(PREFIX)/bin
