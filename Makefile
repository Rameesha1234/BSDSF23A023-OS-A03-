# -------------------------------
# Makefile for BSDSF23A023-OS-A03  (Feature 2 - Built-in Commands)
# -------------------------------

CC := gcc
CFLAGS := -Wall -Wextra -Werror -std=gnu11 -g -Iinclude
LDFLAGS :=

SRCDIR := src
BINDIR := bin
OBJDIR := obj

# Only compile .c files that are part of this feature
SOURCES := $(SRCDIR)/main.c $(SRCDIR)/shell.c $(SRCDIR)/execute.c
OBJECTS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))

TARGET := $(BINDIR)/myshell

.PHONY: all clean run dist dirs help

# -------------------------------
# Default build
# -------------------------------
all: dirs $(TARGET)

dirs:
	@mkdir -p $(BINDIR)
	@mkdir -p $(OBJDIR)

# -------------------------------
# Linking
# -------------------------------
$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

# -------------------------------
# Compilation
# -------------------------------
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# -------------------------------
# Run
# -------------------------------
run: all
	@echo "Launching $(TARGET)..."
	@$(TARGET)

# -------------------------------
# Create tar.gz distribution
# -------------------------------
dist: all
	tar czvf myshell-$(shell date +%Y%m%d)-v2.tar.gz -C $(BINDIR) myshell
	@echo "Created archive: myshell-*.tar.gz"

# -------------------------------
# Clean up
# -------------------------------
clean:
	rm -rf $(OBJDIR) $(BINDIR) myshell-*.tar.gz
	@echo "Cleaned all build files."

# -------------------------------
# Help
# -------------------------------
help:
	@echo "Usage:"
	@echo "  make        Build the shell (creates $(TARGET))"
	@echo "  make run    Build then run the shell"
	@echo "  make dist   Build and create tar.gz of binary"
	@echo "  make clean  Remove build artifacts"
