TARGET=enum
LIBS=#-lm

BINDIR=bin
SRCDIR=src

# Everything below here is sacred
# --------------------------------

CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -pthread -D_GNU_SOURCE -Wno-parentheses -Wno-unused-but-set-variable
CFLAGSD=-ggdb -fstack-protector-all
CFLAGSO=-O3 -fomit-frame-pointer -D SILENT
TARGETS=$(TARGET) d$(TARGET) o$(TARGET)

.PHONY: default all clean
.PRECIOUS: $(TARGET $(OBJECTS)

default: $(TARGETS)
all: clean default

OBJECTS=$(patsubst $(SRCDIR)/%.c, $(BINDIR)/%.o, $(wildcard $(SRCDIR)/*.c))
OBJECTSD=$(patsubst $(SRCDIR)/%.c, $(BINDIR)/%.d.o, $(wildcard $(SRCDIR)/*.c))
OBJECTSO=$(patsubst $(SRCDIR)/%.c, $(BINDIR)/%.o.o, $(wildcard $(SRCDIR)/*.c))
HEADERS=$(wildcard $(SRCDIR)/*.h)

$(BINDIR)/%.o: $(SRCDIR)/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

$(BINDIR)/%.d.o: $(SRCDIR)/%.c $(HEADERS)
	$(CC) $(CFLAGS) $(CFLAGSD) -c $< -o $@

$(BINDIR)/%.o.o: $(SRCDIR)/%.c $(HEADERS)
	$(CC) $(CFLAGS) $(CFLAGSO) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) $(LIBS) -o $@

d$(TARGET): $(OBJECTSD)
	$(CC) $(OBJECTSD) $(CFLAGS) $(CFLAGSD) $(LIBS) -o $@

o$(TARGET): $(OBJECTSO)
	$(CC) $(OBJECTSO) $(CFLAGS) $(CFLAGSO) $(LIBS) -o $@

clean:
	rm -f $(BINDIR)/*.o $(TARGETS)
