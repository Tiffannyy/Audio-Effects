# makefile for AudioEffects program

# Compiler
CP = g++

# Flags
CFLAGS = -l portaudio

# Target Executable
TARGET = AudioEffects
TARGETSRC = main.cpp


all: $(TARGET)

$(TARGET): $(TARGETSRC)
	$(CP) $(TARGETSRC) -o $(TARGET) $(CFLAGS)
