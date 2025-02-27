ESP_IDF_PATH ?= $(IDF_PATH)
TARGET ?= 

all: $(TARGET)

sender:
	@echo "Building Sender..."
	idf.py set-target esp32
	idf.py -DSENDER build
	idf.py flash

receiver:
	@echo "Building Receiver..."
	idf.py set-target esp32
	idf.py -DRECEIVER build
	idf.py flash

clean:
	@echo "Cleaning project..."
	idf.py fullclean

help:
	@echo "Usage: make TARGET=[sender|receiver]"
	@echo "Targets:"
	@echo "  sender    Build and flash sender firmware"
	@echo "  receiver  Build and flash receiver firmware"
	@echo "  clean     Clean build artifacts"
	@echo "  help      Display this help message"

.PHONY: all sender receiver clean help