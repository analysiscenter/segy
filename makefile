CC := g++
RM := del

MKDIR := mkdir
CHECK := if not exist
SRCDIR := src
BUILDDIR := build
BINDIR := bin
CFLAGS := -std=gnu++11 -I . -c
LDFLAGS := -static-libgcc -static-libstdc++
SOURCES := $(SRCDIR)/main.cpp $(SRCDIR)/anonymizer.cpp $(SRCDIR)/shift_geo.cpp $(SRCDIR)/path_handler.cpp
OBJECTS := $(BUILDDIR)/main.o $(BUILDDIR)/anonymizer.o $(BUILDDIR)/shift_geo.o $(BUILDDIR)/path_handler.o
EXECUTABLE := $(BINDIR)/anonymizer

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CHECK) $(BINDIR) $(MKDIR) $(BINDIR)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(CHECK) $(BUILDDIR) $(MKDIR) $(BUILDDIR)
	$(CC) $(CFLAGS) $< -o $@

clean:
	$(RM) -r $(BUILDDIR)\*.o /Q
