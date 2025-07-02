# ESP32 MVC Framework Makefile
# Provides convenient commands for building and managing the project

# Variables
LOG_FILE ?= log.txt
ELF_FILE ?= .pio/build/esp32cam/firmware.elf
ADDR2LINE := xtensa-esp32-elf-addr2line
ADDRESS_FILE := addresses.txt
BACKTRACE_FILE := backtrace.txt
PIO_CMD := pio

# Default target
.PHONY: all build upload monitor clean install test docs help dev prod check size fs info
all: build

# Build the project
build:
	@echo "Building ESP32 MVC Framework..."
	$(PIO_CMD) run

# Upload to ESP32
upload:
	@echo "Uploading to ESP32..."
	$(PIO_CMD) run -t upload

# Monitor serial output
monitor:
	@echo "Starting serial monitor..."
	$(PIO_CMD) device monitor

# Upload and monitor in one command
flash: upload monitor

# Clean build files
clean:
	@echo "Cleaning build files..."
	$(PIO_CMD) run -t clean

# Install/update dependencies
install:
	@echo "Installing dependencies..."
	$(PIO_CMD) lib install

# Update dependencies
update:
	@echo "Updating dependencies..."
	$(PIO_CMD) lib update

# Build filesystem image and upload
fs:
	@echo "Building and uploading filesystem..."
	$(PIO_CMD) run -t uploadfs

# Run tests (if implemented)
test:
	@echo "Running tests..."
	$(PIO_CMD) test

# Generate documentation
docs:
	@echo "Generating documentation..."
	@echo "Documentation available in README.md"

# Show project info
info:
	@echo "ESP32 MVC Framework Project Information:"
	@echo "======================================="
	@echo "Platform: ESP32"
	@echo "Framework: Arduino + Custom MVC"
	@echo "Build system: PlatformIO"
	@echo ""
	@echo "Available commands:"
	@echo "- make build    : Build the project"
	@echo "- make upload   : Upload to ESP32"
	@echo "- make monitor  : Start serial monitor"
	@echo "- make flash    : Upload and monitor"
	@echo "- make clean    : Clean build files"
	@echo "- make install  : Install dependencies"
	@echo "- make update   : Update dependencies"
	@echo "- make fs       : Upload filesystem"
	@echo "- make test     : Run tests"
	@echo "- make docs     : Generate documentation"
	@echo "- make help     : Show this help"

# Show help
help: info

# Development shortcuts
dev: clean build upload monitor

# Production build
prod:
	@echo "Building for production..."
	$(PIO_CMD) run -e esp32cam

# Check code
check:
	@echo "Checking code..."
	$(PIO_CMD) check

# Size analysis
size:
	@echo "Analyzing binary size..."
	$(PIO_CMD) run -t size
all: help

# Help message
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  extract       - Extract addresses from log file"
	@echo "  backtrace     - Generate backtrace from extracted addresses"
	@echo "  run           - Run platformio build"
	@echo "  debug_serial  - Run platformio debug over serial"
	@echo "  clean         - Clean temporary files"
	@echo "  full          - Run extract, backtrace, and build"
	@echo "  capture_logs  - Capture logs to a text file"
	@echo "  decode_bt     - Decode backtrace from command line"

# Extract addresses from log file
.PHONY: extract
extract:
	@echo "Extracting addresses from $(LOG_FILE)..."
	@grep -oE '0x[0-9a-fA-F]+' $(LOG_FILE) | sort -u > $(ADDRESS_FILE)
	@echo "Addresses saved to $(ADDRESS_FILE)."

# Generate backtrace using addr2line
.PHONY: backtrace
backtrace:
	@echo "Generating backtrace..."
	@cat $(ADDRESS_FILE) | xargs -I {} sh -c '$(ADDR2LINE) -pfiaC -e $(ELF_FILE) {}' > $(BACKTRACE_FILE)
	@echo "Backtrace saved to $(BACKTRACE_FILE)."

# Run platformio build
.PHONY: run
run:
	@echo "Running PlatformIO build..."
	@$(PIO_CMD) run

# Run platformio debug over serial with optional baud rate
.PHONY: debug_serial
debug_serial:
	@echo "Starting PlatformIO debug (serial)..."
	@$(PIO_CMD) debug

# Or specify baud rate explicitly
.PHONY: debug_serial_baud
debug_serial_baud:
	@echo "Starting PlatformIO debug (serial) at baud rate $(BAUD_RATE)..."
	@$(PIO_CMD) debug -b $(BAUD_RATE)

# Clean temporary files
.PHONY: clean
clean:
	rm -f $(ADDRESS_FILE) $(BACKTRACE_FILE)

# Run extract, backtrace, and build in sequence
.PHONY: full
full: extract backtrace run

# Target to capture logs to a text file
.PHONY: capture_logs
capture_logs:
	@echo "Capturing logs to $(LOG_FILE)..."
	@$(PIO_CMD) device monitor > $(LOG_FILE) 2>&1 &
	@echo "Logs are being written to $(LOG_FILE). Use 'kill %%' to stop."

# Direct backtrace decoding (usage: make decode_bt ADDRESSES="0x4013084f 0x400d6ca5")
.PHONY: decode_bt
decode_bt:
	@if [ -z "$(ADDRESSES)" ]; then \
		echo "Error: No addresses provided. Usage: make decode_bt ADDRESSES=\"0x4013084f 0x400d6ca5\"" >&2; \
		exit 1; \
	fi
	@echo "Decoding backtrace addresses: $(ADDRESSES)"
	@$(ADDR2LINE) -pfiaC -e $(ELF_FILE) $(ADDRESSES)