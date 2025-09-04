# Whoop Alternative Firmware Makefile
# Comprehensive build system for all modules

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -g
INCLUDES = -Imodules/ppg_pipeline -Imodules/imu_algorithms -Imodules/health_monitor -Idrivers/ppg -Idrivers/imu -I.

# Mock Zephyr dependencies for host compilation
DEFINES = -DCONFIG_PPG_SAMPLE_RATE=50 -DCONFIG_LOG_DEFAULT_LEVEL=3

# Source files
PPG_SOURCES = modules/ppg_pipeline/ppg_simulator.c \
              modules/ppg_pipeline/ppg_pipeline.c

IMU_SOURCES = modules/imu_algorithms/imu_algorithms.c

HEALTH_SOURCES = modules/health_monitor/health_monitor.c

# Output directory
BUILD_DIR = build

.PHONY: all clean ppg-test imu-test health-test

all: ppg-test imu-test health-test

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# PPG Test executable
ppg-test: $(BUILD_DIR)
	@echo "🔬 Compiling PPG Simulator Test..."
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) \
		tests/ppg_host_test.c \
		-lm -o $(BUILD_DIR)/ppg_test

# IMU Test executable  
imu-test: $(BUILD_DIR)
	@echo "🚶 Compiling IMU Algorithm Test..."
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) \
		tests/imu_final_test.c \
		-lm -o $(BUILD_DIR)/imu_test

# Health Monitoring Test executable
health-test: $(BUILD_DIR)
	@echo "🩺 Compiling Health Monitor Test..."
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) \
		tests/health_host_test.c \
		-lm -o $(BUILD_DIR)/health_test

clean:
	rm -rf $(BUILD_DIR)

# Run tests
run-ppg-test: ppg-test
	@echo "🔬 Running PPG Simulator Test..."
	./$(BUILD_DIR)/ppg_test

run-imu-test: imu-test
	@echo "🚶 Running IMU Algorithm Test..."
	./$(BUILD_DIR)/imu_test

run-health-test: health-test
	@echo "🩺 Running Health Monitor Test..."
	./$(BUILD_DIR)/health_test

run-all-tests: ppg-test imu-test health-test
	@echo "🧪 Running All Firmware Tests..."
	@echo ""
	@$(MAKE) run-ppg-test
	@echo ""
	@$(MAKE) run-imu-test
	@echo ""
	@$(MAKE) run-health-test
