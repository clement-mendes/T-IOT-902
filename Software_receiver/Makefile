TARGET ?= 

all: $(TARGET)

deploy:
	@echo "Building receiver..."
	idf.py set-target esp32
	idf.py build
	idf.py flash monitor

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
	@echo "  deploy    Build and flash receiver firmware"
	@echo "  tests     Run tests"
	@echo "  monitor   Monitor serial output"
	@echo "  clean     Clean build artifacts for test and receiver"
	@echo "  help      Display this help message"

.PHONY: all clean help tests deploy monitor