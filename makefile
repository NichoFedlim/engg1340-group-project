# Main Makefile for Year 1 Engineering Experience

# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -std=c++11
LDFLAGS = -lncursesw

# Main source files
MAIN_SOURCES = main_menu.cpp
MAIN_TARGET = year1_experience

# Default rule
all: $(MAIN_TARGET)

# Rule to build the main executable
$(MAIN_TARGET): $(MAIN_SOURCES)
	$(CXX) $(CXXFLAGS) $(MAIN_SOURCES) -o $(MAIN_TARGET) $(LDFLAGS)

# Clean up
clean:
	rm -f $(MAIN_TARGET)

# Run the main program
run: $(MAIN_TARGET)
	./$(MAIN_TARGET)

.PHONY: all clean run