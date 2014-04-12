include config.mk

# these change -> everything is rebuilt
ALLDEPS=$(HEADERS) Makefile config.mk
OBJECTS=$(SOURCES:.c=.o)
.PHONY: all clean clean-all touch
.SUFFIXES: .c .o

.c.o:
	@echo "    [CC] $<"
	@$(CC) $(CFLAGS) -c $<

all: $(EXECUTABLE) $(ALLDEPS)

touch:
	touch $(SOURCES) $(HEADERS)

$(EXECUTABLE): $(OBJECTS) $(ALLDEPS)
	@echo "    [CC] $(EXECUTABLE)"
	@$(CC) $(CFLAGS) -o $(EXECUTABLE) $(OBJECTS) $(LDFLAGS)

$(OBJECTS): $(ALLDEPS)

clean:
	-@make clean-all 2>/dev/null >/dev/null

clean-all:
	-@rm $(EXECUTABLE) $(OBJECTS)
