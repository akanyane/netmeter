CC      ?= gcc
TARGET  := netmeter
BUILD   := build
SRCS    := $(wildcard src/*.c)
OBJS    := $(patsubst src/%.c,$(BUILD)/%.o,$(SRCS))

ifeq ($(OS),Windows_NT)
TARGET := netmeter.exe
endif

CURL_CFLAGS := $(shell pkg-config --cflags libcurl 2>/dev/null)
CURL_LIBS   := $(shell pkg-config --libs libcurl 2>/dev/null)
ifeq ($(strip $(CURL_LIBS)),)
CURL_LIBS := -lcurl
endif

CFLAGS += -std=c99 -Wall -Wextra -O2 -Iinclude $(CURL_CFLAGS)
LDLIBS += $(CURL_LIBS) -lm

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)

$(BUILD)/%.o: src/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD):
	mkdir -p $(BUILD)

# Pass options with: make run ARGS="-s 5 -j"
run: all
	./$(TARGET) $(ARGS)

clean:
	rm -rf $(BUILD) $(TARGET)
