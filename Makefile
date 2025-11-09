# makefile for AudioEffects program

# Compiler
CP = g++

# Flags
CFLAGS = -std=c++11
LDFLAGS = -lportaudio
LDPATH = -I"C:\msys64\mingw64\bin" -L"C:\msys64\mingw64\lib" # Comment out if needed

# Target Executable
TARGET = start
TARGETSRC = main.cpp

all: $(TARGET)

$(TARGET): $(TARGETSRC)
	$(CP) $(CFLAGS) $(TARGETSRC) -o $(TARGET) $(LDPATH) $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o