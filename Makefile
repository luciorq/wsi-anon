CC		= gcc
CLANG	= clang
CFLAGS_DEBUG = -g -ggdb -O0 -Wall
CFLAGS	= -Wall
LDFLAGS = -ltiff
DEPS 	= wsi-anonymizer.h tiff-based-io.h mirax-io.h ini-parser.h utils.h
OBJ 	= src/wsi-anonymizer.c src/tiff-based-io.c src/mirax-io.c src/ini-parser.c src/utils.c
CONSOLE = src/console-app.c src/wsi-anonymizer.c src/tiff-based-io.c src/mirax-io.c src/ini-parser.c src/utils.c

default: wsi-anonymizer-debug.out

debug: wsi-anonymizer-debug.out

console-wsi-anonymizer.a: $(CONSOLE)
	$(CC) $(CFLAGS_DEBUG) -o $@ $^ $(LDFLAGS)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) $@ $< $(LDFLAGS)

wsi-anonymizer-debug.out: $(OBJ)
	$(CC) $(CFLAGS_DEBUG) -o $@ $^ $(LDFLAGS)

wsi-anonymizer.out: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o *.out *.exe *.a