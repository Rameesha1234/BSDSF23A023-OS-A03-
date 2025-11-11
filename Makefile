# Makefile for OS-A03 (Feature 4 with Readline)

CC := gcc
CFLAGS := -Wall -Wextra -Werror -std=gnu11 -g -Iinclude
LDFLAGS := -lreadline

SRCDIR := src
OBJDIR := obj
BINDIR := bin

SOURCES := $(SRCDIR)/main.c $(SRCDIR)/shell.c $(SRCDIR)/execute.c
OBJECTS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))

TARGET := $(BINDIR)/myshell

.PHONY: all clean run dist dirs

all: dirs $(TARGET)

dirs:
	@mkdir -p $(BINDIR)
	@mkdir -p $(OBJDIR)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	@$(TARGET)

clean:
	rm -rf $(OBJDIR) $(BINDIR) myshell-*.tar.gz
	@echo "Cleaned all build files."
