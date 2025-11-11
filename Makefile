# Makefile for ROLL_NO-OS-A03
# Compiles only: main.c, shell.c, execute.c

CC := gcc
CFLAGS := -Wall -Wextra -Werror -std=gnu11 -g -Iinclude
LDFLAGS := -lreadline

SRCDIR := src
BINDIR := bin
OBJDIR := obj

# Explicitly mention only these 3 source files
SOURCES := $(SRCDIR)/main.c $(SRCDIR)/shell.c $(SRCDIR)/execute.c
OBJECTS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))

TARGET := $(BINDIR)/myshell

.PHONY: all clean run dist dirs help

all: dirs $(TARGET)

dirs:
	@mkdir -p $(BINDIR)
	@mkdir -p $(OBJDIR)

# Link step
$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

# Compilation of each file
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Run the shell
run: all
	@echo "Launching $(TARGET)..."
	@$(TARGET)

# Create a tar.gz binary release file
dist: all
	tar czvf myshell-$(shell date +%Y%m%d).tar.gz -C $(BINDIR) myshell
	@echo "Created archive: myshell-*.tar.gz"

clean:
	rm -rf $(OBJDIR) $(BINDIR) myshell-*.tar.gz
	@echo "Cleaned all build files."

help:
	@echo "Usage:"
	@echo "  make        Build the shell (creates $(TARGET))"
	@echo "  make run    Build then run the shell"
	@echo "  make dist   Build and create tar.gz of binary"
	@echo "  make clean  Remove build artifacts"
