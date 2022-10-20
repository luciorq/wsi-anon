#
# Test Makefile to run under Windows
#

GCC = gcc

C_FLAG = -c
O_FLAG = -o

SRCDIR = src

SOURCES = $(wildcard $(SRCDIR)/*.c)

console-app:	console-app.o wsi-anonymizer.o
				$(GCC) $(O_FLAG) console-app.o wsi-anonymizer.o

console-app.o:	src/console-app.c
				$(GCC) $(C_FLAG) src/console-app.c

wsi-anonymizer.o:	src/wsi-anonymizer.c src/wsi-anonymizer.h
					$(GCC) $(C_FLAG) src/wsi-anonymizer.c

clean:
	rm -f console-app.o wsi-anonymizer.o