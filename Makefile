#
# Default Makefile
#

CONSOLE_TARGET   = wsi-anon.out
WASM_TARGET = wsi-anon.js
CONSOLE_DBG_TARGET = wsi-anon-dbg.out
STATIC_LIBRARY_TARGET = libwsianon.a
SHARED_LIBRARY_TARGET = libwsianon.so
SO_NAME = libwsianon
TEST_TARGET = utests

CC       = gcc
CFLAGS   = -Wall -I. -O2
CFLAGS_DEBUG = -g -ggdb -O0 -Wall

LFLAGS   = -Wall -I.

EMCC 	 = emcc

LFLAGS_TESTS = -lcunit

SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin
TESTDIR	 = test/unit

SOURCES  := $(filter-out $(SRCDIR)/js-file.c $(SRCDIR)/wsi-anonymizer-wasm.c, $(wildcard $(SRCDIR)/*.c))
SOURCES_LIB = $(filter-out $(SRCDIR)/console-app.c $(SRCDIR)/js-file.c $(SRCDIR)/wsi-anonymizer-wasm.c, $(wildcard $(SRCDIR)/*.c))
SOURCES_WASM = $(filter-out $(SRCDIR)/console-app.c $(SRCDIR)/native-file.c, $(wildcard $(SRCDIR)/*.c))

OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
OBJECTS_DBG  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/debug/%.o)
OBJECTS_SHARED := $(SOURCES_LIB:$(SRCDIR)/%.c=$(OBJDIR)/shared/%.o)

UNIT_TEST_FILES = $(TESTDIR)/utils-test.c $(TESTDIR)/ini-parser-test.c $(TESTDIR)/wsi-anonymizer-test.c $(TESTDIR)/test-runner.c

default: static-lib shared-lib console-app

shared-lib: makedirs $(BINDIR)/$(SHARED_LIBRARY_TARGET)

$(BINDIR)/$(SHARED_LIBRARY_TARGET): 
	@$(CC) -shared -Wl,-soname,$(SO_NAME) -o $(BINDIR)/$(SHARED_LIBRARY_TARGET) -fPIC $(SOURCES_LIB)

static-lib: makedirs $(BINDIR)/$(STATIC_LIBRARY_TARGET)

$(BINDIR)/$(STATIC_LIBRARY_TARGET): $(OBJECTS) 
	@ar rcs $@ $^
	@echo "Building static lib "$@" complete!"

console-app: $(BINDIR)/$(CONSOLE_TARGET)
	@echo "Building console app "$<

$(BINDIR)/$(CONSOLE_TARGET): makedirs $(OBJECTS)
	@$(CC) $(OBJECTS) $(LFLAGS) -o $@
	@echo "Linking complete!"

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiling "$<"..."

wasm: makedirs $(BINDIR)/$(WASM_TARGET)

$(BINDIR)/$(WASM_TARGET): makedirs
	@$(EMCC) -Wall $(SOURCES_WASM) -Os -o $(BINDIR)/$(WASM_TARGET) --extern-pre-js wrapper/js/anonymized-stream.js -s WASM=1 -s WASM_BIGINT -s ALLOW_MEMORY_GROWTH=1 -s ASYNCIFY -s SINGLE_FILE=1 -s EXPORTED_RUNTIME_METHODS='["cwrap"]'

tests: makedirs
	@$(CC) -o $(BINDIR)/$(TEST_TARGET) $(SOURCES_LIB) $(UNIT_TEST_FILES) -g $(LFLAGS_TESTS)

console-app-debug: makedirs $(BINDIR)/$(CONSOLE_DBG_TARGET)

$(BINDIR)/$(CONSOLE_DBG_TARGET): makedirs $(OBJECTS_DBG)
	@$(CC) $(OBJECTS_DBG) $(LFLAGS) -o $@
	@echo "Linking complete!"

$(OBJECTS_DBG): $(OBJDIR)/debug/%.o : $(SRCDIR)/%.c
	@$(CC) $(CFLAGS_DEBUG) -c $< -o $@
	@echo "Compiling "$<"..."

makedirs: 
	@mkdir -p $(OBJDIR)
	@mkdir -p $(OBJDIR)/debug
	@mkdir -p $(BINDIR)

.PHONY: clean
clean:
	@rm -f $(OBJECTS) $(BINDIR)/*.a $(BINDIR)/*.out || true
	@rm -r $(OBJDIR) $(BINDIR) || true
	@echo "Cleanup done!"