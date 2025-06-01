CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11
LDFLAGS = -lncursesw

# Object files
OBJS = main_1320.o 1320.o text_animator.o

TARGET = 1320

# Main target
all: $(TARGET)

# Link the program
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS)

# Compile main file
main_1320.o: main_1320.cpp 1320.h text_animator.h
	$(CXX) $(CXXFLAGS) -c main_1320.cpp

# Compile 1320 game
1320.o: 1320.cpp 1320.h
	$(CXX) $(CXXFLAGS) -c 1320.cpp

# Compile text animator
text_animator.o: text_animator.cpp text_animator.h
	$(CXX) $(CXXFLAGS) -c text_animator.cpp

# Clean up
clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run