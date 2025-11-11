CC := gcc
CFLAGS := -Wall -Wextra -Werror -std=gnu11 -g -Iinclude
LDFLAGS :=

SRCDIR := src
BINDIR := bin
OBJDIR := obj

SOURCES := $(wildcard $(SRCDIR)/*.c)
OBJECTS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))
TARGET := $(BINDIR)/myshell

.PHONY: all clean run dist dirs

all: dirs $(TARGET)

dirs:
	@mkdir -p $(BINDIR)
	@mkdir -p $(OBJDIR)

# Link
$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

# Compile
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Convenience: run the shell
run: all
	@echo "Launching $(TARGET)"
	@$(TARGET)

# Create a tar.gz distribution of the binary
dist: all
	tar czvf myshell-$(shell date +%Y%m%d)-v1.0-base.tar.gz -C $(BINDIR) myshell
	@echo "Created archive: myshell-*.tar.gz"

clean:
	rm -rf $(OBJDIR) $(BINDIR) myshell-*.tar.gz

# show help
help:
	@echo "Usage:"
	@echo "  make        Build the shell (creates $(TARGET))"
	@echo "  make run    Build then run the shell"
	@echo "  make dist   Build and create tar.gz of binary"
	@echo "  make clean  Remove build artifacts"
