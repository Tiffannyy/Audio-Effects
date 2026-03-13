# makefile for AudioEffects program

# Python
MODULE := engine
PYTHON_INCLUDES := $(shell python3 -m pybind11 --includes)
PYTHON_SUFFIX := $(shell python3-config --extension-suffix)

# Compiler
CXX = g++ -I /cpp

# Flags
CFLAGS = -O3 -Wall -Werror -Wextra -shared -std=c++11 -fPIC $(PYTHON_INCLUDES)
LDFLAGS = -lasound -lwiringPi

# Target Executable
TARGET = $(MODULE)$(PYTHON_SUFFIX)
SRCS = cpp/src/binding.cpp \
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
