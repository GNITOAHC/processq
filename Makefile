# Compiler and flags
CC = gcc
CFLAGS = -Wall -g

# Source and output directories
SRCDIR = .
OBJDIR = obj
BINDIR = bin
TARGET = $(BINDIR)/main

# Get all .c files in the source directory
SRC = $(shell find $(SRCDIR) -name '*.c')

# Generate corresponding .o files in the obj directory
OBJ = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRC))

# Default rule to build the binary
all: $(TARGET)

# Rule to build the target binary from object files
$(TARGET): $(OBJ) | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

# Rule to compile .c files into .o files
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	@mkdir -p $(dir $@)  # Create necessary subdirectories in obj/
	$(CC) $(CFLAGS) -c $< -o $@

# Create the object directory if it doesn't exist
$(OBJDIR):
	@mkdir -p $(OBJDIR)

# Create the binary directory if it doesn't exist
$(BINDIR):
	@mkdir -p $(BINDIR)

# Clean up compiled files
clean:
	rm -rf $(OBJDIR) $(BINDIR)

.PHONY: all clean
