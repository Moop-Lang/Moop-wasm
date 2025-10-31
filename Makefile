# Rio + RioVN Unified Compiler Makefile

CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2 -g -I./src
LDFLAGS =

# Source files
SRCS = src/main.c src/surface_parser.c src/simple_parser.c src/unified_compiler.c src/hr_ir.c src/consistency_checker.c src/l5_moop.c src/l3_turchin.c
OBJS = $(SRCS:.c=.o)
TARGET = august_rio_compiler

# API Library
API_SRCS = src/rio_api.c src/surface_parser.c src/simple_parser.c src/hr_ir.c src/l5_moop.c src/l3_turchin.c
API_OBJS = $(API_SRCS:.c=.o)
API_LIB = libaugust_rio.so
API_STATIC = libaugust_rio.a

.PHONY: all clean test debug

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

# API Library targets
api: $(API_LIB) $(API_STATIC)

$(API_LIB): $(API_OBJS)
	$(CC) -shared -o $@ $^

$(API_STATIC): $(API_OBJS)
	ar rcs $@ $^

src/%.o: src/%.c src/architecture.h src/surface_parser.h src/rio_api.h src/consistency_checker.h src/l3_turchin.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(API_OBJS) $(TARGET) $(API_LIB) $(API_STATIC) src/*.o

test: $(TARGET)
	./$(TARGET)

debug: $(TARGET)
	./$(TARGET) --debug

strict: $(TARGET)
	./$(TARGET) --strict

json: $(TARGET)
	./$(TARGET) --json

compile-sample: $(TARGET)
	./$(TARGET) examples/sample.rio --debug --json

# API library targets
api-all: api

# Example API usage
api-example: examples/api_example.c api
	$(CC) $(CFLAGS) -L. -laugust_rio examples/api_example.c -o api_example
	./api_example

# HRIR demo
hrir-demo: examples/hrir_demo.c src/hr_ir.o
	$(CC) $(CFLAGS) examples/hrir_demo.c src/hr_ir.o -o hrir_demo
	./hrir_demo

# Python bindings demo
python-demo: bindings/python/rio_py.py api
	python3 bindings/python/rio_py.py

# Consistency checker demo
consistency-demo: examples/consistency_demo.c src/hr_ir.o src/consistency_checker.o
	$(CC) $(CFLAGS) examples/consistency_demo.c src/hr_ir.o src/consistency_checker.o -o consistency_demo
	./consistency_demo

help:
	@echo "August-Rio - Unified Programming System"
	@echo ""
	@echo "Targets:"
	@echo "  all               - Build the compiler"
	@echo "  api               - Build embeddable API library (libaugust_rio.so + libaugust_rio.a)"
	@echo "  api-all           - Same as api"
	@echo "  api-example       - Build and run API usage example"
	@echo "  clean             - Remove build artifacts"
	@echo "  test              - Run with default demo"
	@echo "  debug             - Run in debug mode"
	@echo "  strict            - Run in strict mode"
	@echo "  json              - Run with JSON output"
	@echo "  compile-sample    - Compile examples/sample.rio with debug + json"
	@echo "  hrir-demo         - Build and run L1 HRIR demonstration"
	@echo "  python-demo       - Build and run Python bindings demonstration"
	@echo "  consistency-demo  - Build and run dual-memory consistency checker"
	@echo "  help              - Show this help message"
	@echo ""
	@echo "CLI Options:"
	@echo "  --debug           - Enable verbose debug output"
	@echo "  --strict          - Enforce explicit D-term tagging"
	@echo "  --json            - Output results in JSON format"
	@echo "  --no-auto-hoist   - Disable automatic hierarchy generation"
	@echo "  file.rio          - Load and compile a .rio file"
	@echo ""
	@echo "API Library:"
	@echo "  libaugust_rio.so  - Shared library for embedding"
	@echo "  libaugust_rio.a   - Static library for linking"

# Development targets
.PHONY: help
