# See https://github.com/bxparks/EpoxyDuino for documentation about using
# EpoxyDuino to compile and run AUnit tests natively on Linux or MacOS.

APP_NAME := MemoryBenchmark
ARDUINO_LIBS := AceCommon AceRoutine
MORE_CLEAN := more_clean
include ../../../EpoxyDuino/EpoxyDuino.mk

.PHONY: benchmarks

TARGETS := attiny.txt nano.txt micro.txt stm32.txt \
	esp8266.txt esp32.txt teensy32.txt

README.md: generate_readme.py generate_table.awk $(TARGETS)
	./generate_readme.py > $@

benchmarks: $(TARGETS)

attiny.txt:
	./collect.sh --cli attiny $@

nano.txt:
	./collect.sh nano $@

micro.txt:
	./collect.sh micro $@

stm32.txt:
	./collect.sh stm32 $@

esp8266.txt:
	./collect.sh nodemcu $@

esp32.txt:
	./collect.sh esp32 $@

# Teensy does not support ArduinoCLI as far as I know.
teensy32.txt:
	./collect.sh --ide teensy32 $@

epoxy:
	./validate_using_epoxy_duino.sh

more_clean:
	echo "Use 'make clean_benchmarks' to remove *.txt files"

clean_benchmarks:
	rm -f $(TARGETS)
