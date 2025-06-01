CXX = g++
CXXFLAGS = -Wall -std=c++11
LDFLAGS = -lncursesw

SOURCES = main_1340.cpp 1340.cpp text_animator.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = 1340

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

run: $(TARGET)
	./$(TARGET)