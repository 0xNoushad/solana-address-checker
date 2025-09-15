# Makefile for Solana Address Monitor

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c17 -O2
LDFLAGS = -lcurl

# Directories
SRCDIR = src
BUILDDIR = build
TARGET = $(BUILDDIR)/solanachecker

# Source files
SOURCES = $(SRCDIR)/solanachecker.c

# Default target
.PHONY: all clean install deps help

all: $(TARGET)

# Create build directory and compile
$(TARGET): $(SOURCES) | $(BUILDDIR)
	@echo "Compiling Solana Address Monitor..."
	@if pkg-config --exists json-c; then \
		echo "Using pkg-config for json-c"; \
		$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $$(pkg-config --cflags --libs json-c); \
	else \
		echo "Using manual linking for json-c"; \
		$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) -ljson-c; \
	fi
	@echo "Build complete! Executable: $(TARGET)"

# Create build directory
$(BUILDDIR):
	@mkdir -p $(BUILDDIR)

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILDDIR)
	@echo "Clean complete!"

# Install dependencies (macOS with Homebrew)
deps-mac:
	@echo "Installing dependencies for macOS..."
	@if command -v brew >/dev/null 2>&1; then \
		brew install json-c curl; \
		echo "Dependencies installed!"; \
	else \
		echo "Homebrew not found. Please install Homebrew first."; \
		echo "Visit: https://brew.sh/"; \
	fi

# Install dependencies (Ubuntu/Debian)
deps-linux:
	@echo "Installing dependencies for Linux..."
	@sudo apt-get update
	@sudo apt-get install -y libcurl4-openssl-dev libjson-c-dev build-essential
	@echo "Dependencies installed!"

# Install dependencies (CentOS/RHEL/Fedora)
deps-redhat:
	@echo "Installing dependencies for RedHat-based systems..."
	@if command -v dnf >/dev/null 2>&1; then \
		sudo dnf install -y libcurl-devel json-c-devel gcc make; \
	else \
		sudo yum install -y libcurl-devel json-c-devel gcc make; \
	fi
	@echo "Dependencies installed!"

# Test the build
test: $(TARGET)
	@echo "Testing the executable..."
	@echo "Running with invalid arguments to test error handling:"
	@$(TARGET) || true
	@echo ""
	@echo "Showing help:"
	@$(TARGET) --help || true

# Install system-wide (optional)
install: $(TARGET)
	@echo "Installing to /usr/local/bin..."
	@sudo cp $(TARGET) /usr/local/bin/solanachecker
	@sudo chmod +x /usr/local/bin/solanachecker
	@echo "Installed! You can now run 'solanachecker' from anywhere."

# Uninstall system-wide installation
uninstall:
	@echo "Uninstalling from /usr/local/bin..."
	@sudo rm -f /usr/local/bin/solanachecker
	@echo "Uninstalled!"

# Debug build with additional flags
debug: CFLAGS += -g -DDEBUG -fsanitize=address -fsanitize=undefined
debug: LDFLAGS += -fsanitize=address -fsanitize=undefined
debug: $(TARGET)
	@echo "Debug build complete!"

# Show help
help:
	@echo "Solana Address Monitor - Makefile Commands"
	@echo "=========================================="
	@echo ""
	@echo "Building:"
	@echo "  make          - Build the project"
	@echo "  make debug    - Build with debug flags"
	@echo "  make clean    - Clean build artifacts"
	@echo ""
	@echo "Dependencies:"
	@echo "  make deps-mac     - Install deps on macOS (Homebrew)"
	@echo "  make deps-linux   - Install deps on Ubuntu/Debian"
	@echo "  make deps-redhat  - Install deps on CentOS/RHEL/Fedora"
	@echo ""
	@echo "Testing:"
	@echo "  make test     - Test the built executable"
	@echo ""
	@echo "Installation:"
	@echo "  make install    - Install system-wide to /usr/local/bin"
	@echo "  make uninstall  - Remove system-wide installation"
	@echo ""
	@echo "Usage after building:"
	@echo "  ./build/solanachecker <address> [interval]"
	@echo ""
	@echo "Example:"
	@echo "  ./build/solanachecker So11111111111111111111111111111111111111112 30"