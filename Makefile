CONSOLE_TARGET   = wsi-anon.out
CONSOLE_DBG_TARGET = wsi-anon-dbg.out
STATIC_LIBRARY_TARGET = libwsianon.a
SHARED_LIBRARY_TARGET = libwsianon.so

CC       = gcc
CFLAGS   = -Wall -I. -O2
CFLAGS_DEBUG = -g -ggdb -O0 -Wall

LINKER   = gcc
LFLAGS   = -Wall -I.

SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin

SOURCES  := $(wildcard $(SRCDIR)/*.c)
SOURCES_LIB = $(filter-out $(SRCDIR)/console-app.c, $(wildcard $(SRCDIR)/*.c))
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
OBJECTS_DBG  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/debug/%.o)
OBJECTS_SHARED := $(SOURCES_LIB:$(SRCDIR)/%.c=$(OBJDIR)/shared/%.o)

default: static-lib shared-lib console-app

shared-lib: makedirs $(BINDIR)/$(SHARED_LIBRARY_TARGET)

$(BINDIR)/$(SHARED_LIBRARY_TARGET): $(OBJECTS_SHARED)
	@$(CC) -shared -o $@ $^
	@echo "Linking shared lib "$@" complete!"

$(OBJECTS_SHARED): $(OBJDIR)/shared/%.o : $(SOURCES_LIB)
	@$(CC) $(CFLAGS) -c -fPIC $< >$@
	@echo "Compiling "$@"..."

static-lib: makedirs $(BINDIR)/$(STATIC_LIBRARY_TARGET)

$(BINDIR)/$(STATIC_LIBRARY_TARGET): $(OBJECTS) 
	@ar rcs $@ $^
	@echo "Building static lib "$@" complete!"

console-app: $(BINDIR)/$(CONSOLE_TARGET)
	@echo "Building console app "$<

$(BINDIR)/$(CONSOLE_TARGET): makedirs $(OBJECTS)
	@$(LINKER) $(OBJECTS) $(LFLAGS) -o $@
	@echo "Linking complete!"

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiling "$<"..."

console-app-debug: makedirs $(BINDIR)/$(CONSOLE_DBG_TARGET)

$(BINDIR)/$(CONSOLE_DBG_TARGET): makedirs $(OBJECTS_DBG)
	@$(LINKER) $(OBJECTS_DBG) $(LFLAGS) -o $@
	@echo "Linking complete!"

$(OBJECTS_DBG): $(OBJDIR)/debug/%.o : $(SRCDIR)/%.c
	@$(CC) $(CFLAGS_DEBUG) -c $< -o $@
	@echo "Compiling "$<"..."

makedirs: 
	@mkdir -p $(OBJDIR)
	@mkdir -p $(OBJDIR)/shared
	@mkdir -p $(OBJDIR)/debug
	@mkdir -p $(BINDIR)

.PHONY: clean
clean:
	@rm -f $(OBJECTS) $(BINDIR)/*.a $(BINDIR)/*.out
	@rm -r $(OBJDIR) $(BINDIR)
	@echo "Cleanup complete!"