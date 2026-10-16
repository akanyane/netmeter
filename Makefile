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

PREFIX ?= /usr/local
BINDIR := $(PREFIX)/bin

TEST_TARGET := $(BUILD)/test_netmeter
ifeq ($(OS),Windows_NT)
TEST_TARGET := $(BUILD)/test_netmeter.exe
endif

.PHONY: all clean run install uninstall test

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

# Unit tests do not touch the network
$(TEST_TARGET): tests/test_netmeter.c $(BUILD)/utils.o $(BUILD)/ping.o | $(BUILD)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

install: all
	mkdir -p $(DESTDIR)$(BINDIR)
	install -m 755 $(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)

clean:
	rm -rf $(BUILD) $(TARGET)
