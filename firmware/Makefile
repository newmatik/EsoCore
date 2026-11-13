#
# EsoCore Firmware Build System
# Industrial IoT Edge Computing Platform
# Copyright © 2025 Newmatik. All rights reserved.
#

# Build configuration
PROJECT_NAME := esocore
VERSION := 1.5.0
BUILD_DIR := build
RELEASE_DIR := release

# Toolchain configuration
CC := arm-none-eabi-gcc
CXX := arm-none-eabi-g++
AS := arm-none-eabi-as
LD := arm-none-eabi-ld
OBJCOPY := arm-none-eabi-objcopy
OBJDUMP := arm-none-eabi-objdump
SIZE := arm-none-eabi-size

# Target configurations
TARGETS := edge sensors
EDGE_TARGET := stm32h7
SENSOR_TARGET := stm32g0

# Build options
DEBUG := 0
OPTIMIZATION := -O2
WARNINGS := -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
STANDARD := -std=c11

# Include paths
COMMON_INCLUDES := \
	-Icommon/communication \
	-Icommon/storage \
	-Icommon/safety \
	-Icommon/management \
	-Icommon/intelligence \
	-Icommon/ui \
	-Icommon/sensors \
	-Icommon/analog

EDGE_INCLUDES := \
	$(COMMON_INCLUDES) \
	-Istm32/edge \
	-Istm32/$(EDGE_TARGET)/hal \
	-Istm32/$(EDGE_TARGET)/cmsis \
	-Istm32/$(EDGE_TARGET)/bsp \
	-Istm32/$(EDGE_TARGET)/drivers

SENSOR_INCLUDES := \
	$(COMMON_INCLUDES) \
	-Istm32/sensors \
	-Istm32/$(SENSOR_TARGET)/hal \
	-Istm32/$(SENSOR_TARGET)/cmsis

# Source files
COMMON_SOURCES := \
	common/communication/protocol.c \
	common/communication/wifi_manager.c \
	common/communication/http_client.c \
	common/communication/modbus_rtu.c \
	common/communication/ethernet_manager.c \
	common/communication/interbus.c \
	common/communication/sensor_bus.c \
	common/analog/analog_input.c \
	common/analog/signal_chain.c \
	common/sensors/sensor_interface.c \
	common/storage/storage_system.c \
	common/safety/safety_io.c \
	common/management/power_management.c \
	common/management/config_manager.c \
	common/intelligence/event_system.c \
	common/intelligence/tinyml_engine.c \
	common/ui/oled_display.c

EDGE_SOURCES := \
	stm32/edge/main.c \
	stm32/$(EDGE_TARGET)/bsp/system_clock.c \
	stm32/$(EDGE_TARGET)/cmsis/system_stm32h7xx.c \
	stm32/$(EDGE_TARGET)/hal/hal_init.c \
	stm32/$(EDGE_TARGET)/hal/hal_gpio.c \
	stm32/$(EDGE_TARGET)/hal/hal_uart.c \
	stm32/$(EDGE_TARGET)/hal/hal_spi.c \
	stm32/$(EDGE_TARGET)/hal/hal_i2c.c \
	stm32/$(EDGE_TARGET)/hal/hal_timer.c \
	stm32/$(EDGE_TARGET)/hal/hal_adc.c \
	stm32/$(EDGE_TARGET)/hal/hal_eth.c \
	stm32/$(EDGE_TARGET)/hal/hal_dma.c \
	stm32/$(EDGE_TARGET)/drivers/drv_ksz8081.c \
	stm32/$(EDGE_TARGET)/drivers/drv_ads1274.c \
	stm32/$(EDGE_TARGET)/drivers/drv_pga280.c \
	stm32/$(EDGE_TARGET)/drivers/drv_tmux1101.c \
	stm32/$(EDGE_TARGET)/drivers/drv_iepe.c \
	stm32/$(EDGE_TARGET)/drivers/drv_ssr.c

# Startup code (vector table, reset handler) of the STM32H747. It is not in the
# repository yet, so the Edge firmware compiles (edge-objects) but does not link.
EDGE_STARTUP := stm32/$(EDGE_TARGET)/startup/startup_stm32h747xx.s
EDGE_LINKER_SCRIPT := stm32/$(EDGE_TARGET)/linker/STM32H747IITx_FLASH_CM7.ld

# Debug and release objects are kept apart, so changing DEBUG never mixes the two.
EDGE_OBJ_DIR := $(BUILD_DIR)/edge/obj/$(if $(filter 1,$(DEBUG)),debug,release)
EDGE_OBJECTS := $(patsubst %.c,$(EDGE_OBJ_DIR)/%.o,$(COMMON_SOURCES) $(EDGE_SOURCES))
EDGE_STARTUP_OBJECT := $(EDGE_OBJ_DIR)/$(EDGE_STARTUP:.s=.o)

SENSOR_SOURCES := \
	stm32/sensors/main.c \
	stm32/$(SENSOR_TARGET)/hal/hal_init.c \
	stm32/$(SENSOR_TARGET)/hal/gpio.c \
	stm32/$(SENSOR_TARGET)/hal/uart.c \
	stm32/$(SENSOR_TARGET)/hal/spi.c \
	stm32/$(SENSOR_TARGET)/hal/i2c.c \
	stm32/$(SENSOR_TARGET)/hal/timer.c \
	stm32/$(SENSOR_TARGET)/hal/adc.c \
	stm32/$(SENSOR_TARGET)/startup/startup_stm32g0xx.s

# Sensor-specific sources
VIBRATION_SOURCES := \
	stm32/sensors/vibration_sensor.c

ACOUSTIC_SOURCES := \
	stm32/sensors/acoustic_sensor.c

CURRENT_SOURCES := \
	stm32/sensors/current_sensor.c

AIR_QUALITY_SOURCES := \
	stm32/sensors/air_quality_sensor.c

OIL_QUALITY_SOURCES := \
	stm32/sensors/oil_quality_sensor.c

PRESSURE_SOURCES := \
	stm32/sensors/pressure_sensor.c

TEMPERATURE_SOURCES := \
	stm32/sensors/temperature_spot_sensor.c

PROXIMITY_SOURCES := \
	stm32/sensors/proximity_position_sensor.c

LIGHT_SOURCES := \
	stm32/sensors/light_sensor.c

# MCU-specific flags
EDGE_MCU_FLAGS := \
	-mcpu=cortex-m7 \
	-mthumb \
	-mfpu=fpv5-d16 \
	-mfloat-abi=hard \
	-DSTM32H747xx \
	-DCORE_CM7 \
	-DUSE_HAL_DRIVER

SENSOR_MCU_FLAGS := \
	-mcpu=cortex-m0plus \
	-mthumb \
	-DSTM32G031xx \
	-DUSE_HAL_DRIVER

# Compiler flags
CFLAGS := \
	$(OPTIMIZATION) \
	$(WARNINGS) \
	$(STANDARD) \
	-g3 \
	-ffunction-sections \
	-fdata-sections \
	-fno-common \
	-fmessage-length=0 \
	-fno-exceptions

# Linker flags
LDFLAGS := \
	-Wl,--gc-sections \
	-Wl,--print-memory-usage \
	-nostartfiles \
	-specs=nano.specs \
	-specs=nosys.specs

# Conditional compilation
ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG -O0
else
	CFLAGS += -DNDEBUG
endif

# Build rules
.PHONY: all clean edge edge-objects sensors help light test

all: edge sensors

help:
	@echo "EsoCore Firmware Build System v$(VERSION)"
	@echo ""
	@echo "Available targets:"
	@echo "  all       - Build all targets (edge and sensors)"
	@echo "  edge      - Build Edge device firmware (needs the STM32H747 startup file)"
	@echo "  edge-objects - Compile every Edge source file without linking"
	@echo "  sensors   - Build all sensor modules"
	@echo "  vibration   - Build vibration sensor module"
	@echo "  acoustic    - Build acoustic sensor module"
	@echo "  current     - Build current sensor module"
	@echo "  air_quality - Build air quality sensor module"
	@echo "  oil_quality - Build oil quality sensor module"
	@echo "  pressure    - Build pressure sensor module"
	@echo "  temperature - Build temperature sensor module"
	@echo "  proximity   - Build proximity sensor module"
	@echo "  light       - Build light sensor module"
	@echo "  test        - Build and run the host unit tests (no target hardware)"
	@echo "  clean       - Clean all build artifacts"
	@echo "  help        - Show this help message"
	@echo ""
	@echo "Build options:"
	@echo "  DEBUG=1   - Enable debug build"
	@echo "  TARGETS=  - Specify targets to build (edge, sensors)"

# Edge device build
edge: $(BUILD_DIR)/edge/$(PROJECT_NAME)_edge.elf

# Compile every Edge source file without linking. Works without the startup file.
edge-objects: $(EDGE_OBJECTS)
	@echo "Edge device sources compiled ($(words $(EDGE_OBJECTS)) files)"

$(EDGE_OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(EDGE_MCU_FLAGS) $(EDGE_INCLUDES) -MMD -MP -c $< -o $@

$(EDGE_OBJ_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $(EDGE_MCU_FLAGS) -c $< -o $@

$(EDGE_STARTUP):
	@echo "Missing $@: the STM32H747 startup file is not in the repository yet,"
	@echo "so the Edge firmware cannot be linked. 'make edge-objects' compiles it."
	@false

$(BUILD_DIR)/edge/$(PROJECT_NAME)_edge.elf: $(EDGE_OBJECTS) $(EDGE_STARTUP_OBJECT) $(EDGE_LINKER_SCRIPT)
	@echo "Linking Edge device firmware..."
	$(CC) $(EDGE_MCU_FLAGS) $(EDGE_OBJECTS) $(EDGE_STARTUP_OBJECT) \
		$(LDFLAGS) -T $(EDGE_LINKER_SCRIPT) \
		-o $@ -lm
	$(OBJCOPY) -O ihex $@ $(BUILD_DIR)/edge/$(PROJECT_NAME)_edge.hex
	$(OBJCOPY) -O binary $@ $(BUILD_DIR)/edge/$(PROJECT_NAME)_edge.bin
	$(SIZE) $@
	@echo "Edge device firmware built successfully"

-include $(EDGE_OBJECTS:.o=.d)

# Sensor modules build
sensors: vibration acoustic current air_quality oil_quality pressure temperature proximity light

vibration: $(BUILD_DIR)/sensors/vibration/$(PROJECT_NAME)_vibration.elf

$(BUILD_DIR)/sensors/vibration/$(PROJECT_NAME)_vibration.elf: $(COMMON_SOURCES) $(SENSOR_SOURCES) $(VIBRATION_SOURCES)
	@echo "Building vibration sensor firmware..."
	@mkdir -p $(BUILD_DIR)/sensors/vibration
	$(CC) $(CFLAGS) $(SENSOR_MCU_FLAGS) $(SENSOR_INCLUDES) \
		-DESOCORE_DEVICE_TYPE_VIBRATION \
		$(COMMON_SOURCES) $(SENSOR_SOURCES) $(VIBRATION_SOURCES) \
		$(LDFLAGS) -T stm32/$(SENSOR_TARGET)/linker/STM32G031G8Ux_FLASH.ld \
		-o $@ -lm
	$(OBJCOPY) -O ihex $@ $(BUILD_DIR)/sensors/vibration/$(PROJECT_NAME)_vibration.hex
	$(OBJCOPY) -O binary $@ $(BUILD_DIR)/sensors/vibration/$(PROJECT_NAME)_vibration.bin
	$(SIZE) $@
	@echo "Vibration sensor firmware built successfully"

acoustic: $(BUILD_DIR)/sensors/acoustic/$(PROJECT_NAME)_acoustic.elf

$(BUILD_DIR)/sensors/acoustic/$(PROJECT_NAME)_acoustic.elf: $(COMMON_SOURCES) $(SENSOR_SOURCES) $(ACOUSTIC_SOURCES)
	@echo "Building acoustic sensor firmware..."
	@mkdir -p $(BUILD_DIR)/sensors/acoustic
	$(CC) $(CFLAGS) $(SENSOR_MCU_FLAGS) $(SENSOR_INCLUDES) \
		-DESOCORE_DEVICE_TYPE_ACOUSTIC \
		$(COMMON_SOURCES) $(SENSOR_SOURCES) $(ACOUSTIC_SOURCES) \
		$(LDFLAGS) -T stm32/$(SENSOR_TARGET)/linker/STM32G031G8Ux_FLASH.ld \
		-o $@ -lm
	$(OBJCOPY) -O ihex $@ $(BUILD_DIR)/sensors/acoustic/$(PROJECT_NAME)_acoustic.hex
	$(OBJCOPY) -O binary $@ $(BUILD_DIR)/sensors/acoustic/$(PROJECT_NAME)_acoustic.bin
	$(SIZE) $@
	@echo "Acoustic sensor firmware built successfully"

current: $(BUILD_DIR)/sensors/current/$(PROJECT_NAME)_current.elf

$(BUILD_DIR)/sensors/current/$(PROJECT_NAME)_current.elf: $(COMMON_SOURCES) $(SENSOR_SOURCES) $(CURRENT_SOURCES)
	@echo "Building current sensor firmware..."
	@mkdir -p $(BUILD_DIR)/sensors/current
	$(CC) $(CFLAGS) $(SENSOR_MCU_FLAGS) $(SENSOR_INCLUDES) \
		-DESOCORE_DEVICE_TYPE_CURRENT \
		$(COMMON_SOURCES) $(SENSOR_SOURCES) $(CURRENT_SOURCES) \
		$(LDFLAGS) -T stm32/$(SENSOR_TARGET)/linker/STM32G031G8Ux_FLASH.ld \
		-o $@ -lm
	$(OBJCOPY) -O ihex $@ $(BUILD_DIR)/sensors/current/$(PROJECT_NAME)_current.hex
	$(OBJCOPY) -O binary $@ $(BUILD_DIR)/sensors/current/$(PROJECT_NAME)_current.bin
	$(SIZE) $@
	@echo "Current sensor firmware built successfully"

air_quality: $(BUILD_DIR)/sensors/air_quality/$(PROJECT_NAME)_air_quality.elf

$(BUILD_DIR)/sensors/air_quality/$(PROJECT_NAME)_air_quality.elf: $(COMMON_SOURCES) $(SENSOR_SOURCES) $(AIR_QUALITY_SOURCES)
	@echo "Building air quality sensor firmware..."
	@mkdir -p $(BUILD_DIR)/sensors/air_quality
	$(CC) $(CFLAGS) $(SENSOR_MCU_FLAGS) $(SENSOR_INCLUDES) \
		-DESOCORE_DEVICE_TYPE_AIR_QUALITY \
		$(COMMON_SOURCES) $(SENSOR_SOURCES) $(AIR_QUALITY_SOURCES) \
		$(LDFLAGS) -T stm32/$(SENSOR_TARGET)/linker/STM32G031G8Ux_FLASH.ld \
		-o $@ -lm
	$(OBJCOPY) -O ihex $@ $(BUILD_DIR)/sensors/air_quality/$(PROJECT_NAME)_air_quality.hex
	$(OBJCOPY) -O binary $@ $(BUILD_DIR)/sensors/air_quality/$(PROJECT_NAME)_air_quality.bin
	$(SIZE) $@
	@echo "Air quality sensor firmware built successfully"

# Additional sensor types (placeholders for future implementation)
oil_quality: $(BUILD_DIR)/sensors/oil_quality/$(PROJECT_NAME)_oil_quality.elf

$(BUILD_DIR)/sensors/oil_quality/$(PROJECT_NAME)_oil_quality.elf: $(COMMON_SOURCES) $(SENSOR_SOURCES) $(OIL_QUALITY_SOURCES)
	@echo "Building oil quality sensor firmware..."
	@mkdir -p $(BUILD_DIR)/sensors/oil_quality
	$(CC) $(CFLAGS) $(SENSOR_MCU_FLAGS) $(SENSOR_INCLUDES) \
		-DESOCORE_DEVICE_TYPE_OIL_QUALITY \
		$(COMMON_SOURCES) $(SENSOR_SOURCES) $(OIL_QUALITY_SOURCES) \
		$(LDFLAGS) -T stm32/$(SENSOR_TARGET)/linker/STM32G031G8Ux_FLASH.ld \
		-o $@ -lm
	$(OBJCOPY) -O ihex $@ $(BUILD_DIR)/sensors/oil_quality/$(PROJECT_NAME)_oil_quality.hex
	$(OBJCOPY) -O binary $@ $(BUILD_DIR)/sensors/oil_quality/$(PROJECT_NAME)_oil_quality.bin
	$(SIZE) $@
	@echo "Oil quality sensor firmware built successfully"

pressure: $(BUILD_DIR)/sensors/pressure/$(PROJECT_NAME)_pressure.elf

$(BUILD_DIR)/sensors/pressure/$(PROJECT_NAME)_pressure.elf: $(COMMON_SOURCES) $(SENSOR_SOURCES) $(PRESSURE_SOURCES)
	@echo "Building pressure sensor firmware..."
	@mkdir -p $(BUILD_DIR)/sensors/pressure
	$(CC) $(CFLAGS) $(SENSOR_MCU_FLAGS) $(SENSOR_INCLUDES) \
		-DESOCORE_DEVICE_TYPE_PRESSURE \
		$(COMMON_SOURCES) $(SENSOR_SOURCES) $(PRESSURE_SOURCES) \
		$(LDFLAGS) -T stm32/$(SENSOR_TARGET)/linker/STM32G031G8Ux_FLASH.ld \
		-o $@ -lm
	$(OBJCOPY) -O ihex $@ $(BUILD_DIR)/sensors/pressure/$(PROJECT_NAME)_pressure.hex
	$(OBJCOPY) -O binary $@ $(BUILD_DIR)/sensors/pressure/$(PROJECT_NAME)_pressure.bin
	$(SIZE) $@
	@echo "Pressure sensor firmware built successfully"

temperature: $(BUILD_DIR)/sensors/temperature/$(PROJECT_NAME)_temperature.elf

$(BUILD_DIR)/sensors/temperature/$(PROJECT_NAME)_temperature.elf: $(COMMON_SOURCES) $(SENSOR_SOURCES) $(TEMPERATURE_SOURCES)
	@echo "Building temperature sensor firmware..."
	@mkdir -p $(BUILD_DIR)/sensors/temperature
	$(CC) $(CFLAGS) $(SENSOR_MCU_FLAGS) $(SENSOR_INCLUDES) \
		-DESOCORE_DEVICE_TYPE_TEMPERATURE \
		$(COMMON_SOURCES) $(SENSOR_SOURCES) $(TEMPERATURE_SOURCES) \
		$(LDFLAGS) -T stm32/$(SENSOR_TARGET)/linker/STM32G031G8Ux_FLASH.ld \
		-o $@ -lm
	$(OBJCOPY) -O ihex $@ $(BUILD_DIR)/sensors/temperature/$(PROJECT_NAME)_temperature.hex
	$(OBJCOPY) -O binary $@ $(BUILD_DIR)/sensors/temperature/$(PROJECT_NAME)_temperature.bin
	$(SIZE) $@
	@echo "Temperature sensor firmware built successfully"

proximity: $(BUILD_DIR)/sensors/proximity/$(PROJECT_NAME)_proximity.elf

$(BUILD_DIR)/sensors/proximity/$(PROJECT_NAME)_proximity.elf: $(COMMON_SOURCES) $(SENSOR_SOURCES) $(PROXIMITY_SOURCES)
	@echo "Building proximity sensor firmware..."
	@mkdir -p $(BUILD_DIR)/sensors/proximity
	$(CC) $(CFLAGS) $(SENSOR_MCU_FLAGS) $(SENSOR_INCLUDES) \
		-DESOCORE_DEVICE_TYPE_PROXIMITY \
		$(COMMON_SOURCES) $(SENSOR_SOURCES) $(PROXIMITY_SOURCES) \
		$(LDFLAGS) -T stm32/$(SENSOR_TARGET)/linker/STM32G031G8Ux_FLASH.ld \
		-o $@ -lm
	$(OBJCOPY) -O ihex $@ $(BUILD_DIR)/sensors/proximity/$(PROJECT_NAME)_proximity.hex
	$(OBJCOPY) -O binary $@ $(BUILD_DIR)/sensors/proximity/$(PROJECT_NAME)_proximity.bin
	$(SIZE) $@
	@echo "Proximity sensor firmware built successfully"

light: $(BUILD_DIR)/sensors/light/$(PROJECT_NAME)_light.elf

$(BUILD_DIR)/sensors/light/$(PROJECT_NAME)_light.elf: $(COMMON_SOURCES) $(SENSOR_SOURCES) $(LIGHT_SOURCES)
	@echo "Building light sensor firmware..."
	@mkdir -p $(BUILD_DIR)/sensors/light
	$(CC) $(CFLAGS) $(SENSOR_MCU_FLAGS) $(SENSOR_INCLUDES) \
		-DESOCORE_DEVICE_TYPE_LIGHT \
		$(COMMON_SOURCES) $(SENSOR_SOURCES) $(LIGHT_SOURCES) \
		$(LDFLAGS) -T stm32/$(SENSOR_TARGET)/linker/STM32G031G8Ux_FLASH.ld \
		-o $@ -lm
	$(OBJCOPY) -O ihex $@ $(BUILD_DIR)/sensors/light/$(PROJECT_NAME)_light.hex
	$(OBJCOPY) -O binary $@ $(BUILD_DIR)/sensors/light/$(PROJECT_NAME)_light.bin
	$(SIZE) $@
	@echo "Light sensor firmware built successfully"

# Release packaging
release: all
	@echo "Creating release package..."
	@mkdir -p $(RELEASE_DIR)
	@cp $(BUILD_DIR)/edge/*.bin $(BUILD_DIR)/edge/*.hex $(RELEASE_DIR)/
	@cp $(BUILD_DIR)/sensors/*/*.bin $(BUILD_DIR)/sensors/*/*.hex $(RELEASE_DIR)/
	@tar -czf $(RELEASE_DIR)/$(PROJECT_NAME)_v$(VERSION).tar.gz -C $(RELEASE_DIR) .
	@echo "Release package created: $(RELEASE_DIR)/$(PROJECT_NAME)_v$(VERSION).tar.gz"

# Programming targets
flash_edge: $(BUILD_DIR)/edge/$(PROJECT_NAME)_edge.hex
	@echo "Flashing Edge device firmware..."
	# Add your programming command here (OpenOCD, ST-Link, etc.)
	# openocd -f interface/stlink.cfg -f target/stm32h7x.cfg -c "program $< verify reset exit"

flash_vibration: $(BUILD_DIR)/sensors/vibration/$(PROJECT_NAME)_vibration.hex
	@echo "Flashing vibration sensor firmware..."
	# Add your programming command here

flash_acoustic: $(BUILD_DIR)/sensors/acoustic/$(PROJECT_NAME)_acoustic.hex
	@echo "Flashing acoustic sensor firmware..."
	# Add your programming command here

flash_current: $(BUILD_DIR)/sensors/current/$(PROJECT_NAME)_current.hex
	@echo "Flashing current sensor firmware..."
	# Add your programming command here

flash_air_quality: $(BUILD_DIR)/sensors/air_quality/$(PROJECT_NAME)_air_quality.hex
	@echo "Flashing air quality sensor firmware..."
	# Add your programming command here

# Host unit tests: hardware-independent modules compiled with the host compiler
# and run on the development machine. Needs a C11 compiler, not the ARM toolchain.
HOST_CC ?= cc
HOST_TEST_CFLAGS := $(STANDARD) -Wall -Wextra -Werror -Wno-unused-function -Wno-unused-parameter \
	-g -fsanitize=address,undefined -fno-sanitize-recover=undefined
HOST_TEST_DIR := $(BUILD_DIR)/tests

test: $(HOST_TEST_DIR)/test_modbus_rtu
	@echo "Running host unit tests..."
	@$(HOST_TEST_DIR)/test_modbus_rtu

$(HOST_TEST_DIR)/test_modbus_rtu: tests/host/test_modbus_rtu.c common/communication/modbus_rtu.c common/communication/modbus_rtu.h
	@mkdir -p $(HOST_TEST_DIR)
	$(HOST_CC) $(HOST_TEST_CFLAGS) -Icommon/communication -o $@ \
		tests/host/test_modbus_rtu.c common/communication/modbus_rtu.c

# Code analysis
analyze:
	@echo "Running static analysis..."
	# Add your analysis tools here (cppcheck, clang-tidy, etc.)

# Documentation
docs:
	@echo "Generating documentation..."
	# Add documentation generation commands here (doxygen, etc.)

# Dependency management
deps:
	@echo "Installing build dependencies..."
	# Add dependency installation commands here

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR)
	@rm -rf $(RELEASE_DIR)

# Very clean (including generated files)
distclean: clean
	@echo "Cleaning all generated files..."
	# Add commands to clean generated source files

# Build information
info:
	@echo "EsoCore Firmware Build System Information"
	@echo "========================================="
	@echo "Project: $(PROJECT_NAME)"
	@echo "Version: $(VERSION)"
	@echo "Edge Target: $(EDGE_TARGET)"
	@echo "Sensor Target: $(SENSOR_TARGET)"
	@echo "Debug Build: $(if $(filter 1,$(DEBUG)),Yes,No)"
	@echo "Optimization: $(OPTIMIZATION)"
	@echo "Build Directory: $(BUILD_DIR)"
	@echo "Release Directory: $(RELEASE_DIR)"

# Default target
.DEFAULT_GOAL := all
