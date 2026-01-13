# makefile for AudioEffects program

# Compiler
CP = g++

# Flags
CFLAGS = -std=c++11
LDFLAGS = -lasound

# Target Executable
TARGET = start
TARGETSRC = main.cpp


all: $(TARGET)

$(TARGET): $(TARGETSRC)
	$(CP) $(CFLAGS) $(TARGETSRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o
