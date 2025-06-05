# makefile for AudioEffects program

# Compiler
CP = clang++

# Flags
CFLAGS = -std=c++17
LDFLAGS = -lportaudio

# Target Executable
TARGET = AudioEffects
TARGETSRC = main.cpp


all: $(TARGET)

$(TARGET): $(TARGETSRC)
	$(CP) $(CFLAGS) $(TARGETSRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o