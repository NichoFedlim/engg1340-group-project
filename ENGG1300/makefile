# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -std=c++11
LDFLAGS = -lncursesw

# Source files
SOURCES = main_1300.cpp 1300.cpp text_animator.cpp
HEADERS = 1300.h text_animator.h

# Output executable
TARGET = 1300

# Default rule
all: $(TARGET)

# Rule to build the executable
$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)

# Clean rule
clean:
	rm -f $(TARGET)

# Run rule
run: $(TARGET)
	./$(TARGET)
