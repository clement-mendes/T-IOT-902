TARGET ?= 

all: $(TARGET)

deploy:
	@echo "Building Sender..."
	idf.py set-target esp32
	idf.py build
	idf.py flash monitor

config:
	@echo "Configuring project..."
	idf.py menuconfig

monitor:
	idf.py monitor

tests:
	@echo "Running tests..."
	cd test && idf.py build
	cd test && idf.py flash monitor

clean:
	@echo "Cleaning project..."
	idf.py fullclean
	cd test && idf.py fullclean

help:
	@echo "Usage: make TARGET=[deploy|clean]"
	@echo "Targets:"
	@echo "  deploy    Build and flash sender firmware"
	@echo "  config    Configure project"
	@echo "  tests     Run tests"
	@echo "  monitor   Monitor serial output"
	@echo "  clean     Clean build artifacts for test and sender"
	@echo "  help      Display this help message"

.PHONY: all clean help tests deploy monitor config