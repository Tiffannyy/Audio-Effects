# makefile for AudioEffects program

# Compiler
CXX = g++ -I /cpp

# Flags
CFLAGS = -std=c++11
LDFLAGS = -lasound -lwiringPi

# Target Executable
TARGET = start
SRCS = 	cpp/src/main.cpp \
	cpp/src/callback.cpp \
	cpp/src/menu.cpp \
	cpp/src/Engine.cpp \
	cpp/src/peripherals.cpp \
	cpp/src/audio.cpp


all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
