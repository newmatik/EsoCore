# Makefile for EsoCore Edge Device (STM32F4)
#
# This Makefile builds the Edge device firmware using ARM GCC toolchain.
# Make sure you have the STM32CubeF4 HAL library installed.
#
# Usage:
#   make all        - Build all targets
#   make clean      - Clean build artifacts
#   make flash      - Flash firmware to device
#   make debug      - Start debugging session

# ============================================================================
# Configuration
# ============================================================================

# Project name
PROJECT_NAME = esocore_edge

# Build directory
BUILD_DIR = build

# Source directories
SRC_DIRS = . ../../common
INC_DIRS = . ../../common

# Target MCU
MCU = STM32F407VG

# Toolchain
CC = arm-none-eabi-gcc
CXX = arm-none-eabi-g++
AS = arm-none-eabi-as
CP = arm-none-eabi-objcopy
SZ = arm-none-eabi-size
GDB = arm-none-eabi-gdb

# ============================================================================
# Compiler Flags
# ============================================================================

# CPU and architecture
CPU = -mcpu=cortex-m4
FPU = -mfpu=fpv4-sp-d16
FLOAT_ABI = -mfloat-abi=hard

# MCU specific flags
MCU_FLAGS = $(CPU) $(FPU) $(FLOAT_ABI) -mthumb

# Optimization
OPT = -O2

# Debug
DEBUG = -g3

# C standard
CSTD = -std=c11

# C++ standard
CXXSTD = -std=c++11

# Warning flags
WARN_FLAGS = -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion

# ============================================================================
# Source Files
# ============================================================================

# C source files
C_SOURCES = $(wildcard *.c)
C_SOURCES += $(wildcard ../../common/*.c)

# Assembly source files
ASM_SOURCES = $(wildcard *.s)

# Include directories
INCLUDES = $(addprefix -I, $(INC_DIRS))

# ============================================================================
# Build Rules
# ============================================================================

.PHONY: all clean flash debug

all: $(BUILD_DIR)/$(PROJECT_NAME).elf $(BUILD_DIR)/$(PROJECT_NAME).hex $(BUILD_DIR)/$(PROJECT_NAME).bin

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile C sources
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(MCU_FLAGS) $(CSTD) $(OPT) $(DEBUG) $(WARN_FLAGS) $(INCLUDES) -c $< -o $@

# Compile common C sources
$(BUILD_DIR)/%.o: ../../common/%.c | $(BUILD_DIR)
	$(CC) $(MCU_FLAGS) $(CSTD) $(OPT) $(DEBUG) $(WARN_FLAGS) $(INCLUDES) -c $< -o $@

# Assemble assembly sources
$(BUILD_DIR)/%.o: %.s | $(BUILD_DIR)
	$(AS) $(MCU_FLAGS) -c $< -o $@

# Link
$(BUILD_DIR)/$(PROJECT_NAME).elf: $(C_SOURCES:%.c=$(BUILD_DIR)/%.o) $(ASM_SOURCES:%.s=$(BUILD_DIR)/%.o)
	$(CC) $(MCU_FLAGS) $^ -o $@ $(LDFLAGS)
	$(SZ) $@

# Generate hex file
$(BUILD_DIR)/$(PROJECT_NAME).hex: $(BUILD_DIR)/$(PROJECT_NAME).elf
	$(CP) -O ihex $< $@

# Generate binary file
$(BUILD_DIR)/$(PROJECT_NAME).bin: $(BUILD_DIR)/$(PROJECT_NAME).elf
	$(CP) -O binary $< $@

# Flash to device (requires OpenOCD)
flash: $(BUILD_DIR)/$(PROJECT_NAME).elf
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
		-c "program $(BUILD_DIR)/$(PROJECT_NAME).elf verify reset exit"

# Debug (requires OpenOCD and GDB)
debug: $(BUILD_DIR)/$(PROJECT_NAME).elf
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg &
	$(GDB) $(BUILD_DIR)/$(PROJECT_NAME).elf -ex "target remote localhost:3333"

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)

# ============================================================================
# Dependencies
# ============================================================================

# Include dependency files if they exist
-include $(wildcard $(BUILD_DIR)/*.d)

# Generate dependency files
$(BUILD_DIR)/%.d: %.c | $(BUILD_DIR)
	$(CC) $(MCU_FLAGS) $(CSTD) $(OPT) $(DEBUG) $(INCLUDES) -MM -MT $(@:.d=.o) $< > $@

# ============================================================================
# Help
# ============================================================================

help:
	@echo "EsoCore Edge Device Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  all        - Build all targets (default)"
	@echo "  clean      - Clean build artifacts"
	@echo "  flash      - Flash firmware to device (requires OpenOCD)"
	@echo "  debug      - Start debugging session (requires OpenOCD and GDB)"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Files generated:"
	@echo "  $(BUILD_DIR)/$(PROJECT_NAME).elf - ELF executable"
	@echo "  $(BUILD_DIR)/$(PROJECT_NAME).hex - Intel HEX file"
	@echo "  $(BUILD_DIR)/$(PROJECT_NAME).bin - Binary file"
