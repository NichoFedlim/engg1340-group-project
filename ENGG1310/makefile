CXX = g++
CXXFLAGS = -Wall -std=c++11
LDFLAGS = -lncursesw

OBJECTS = main.o text_animator.o 1310.o

TARGET = 1310

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS) $(LDFLAGS)

main.o: main.cpp text_animator.h 1310.h
	$(CXX) $(CXXFLAGS) -c main.cpp

text_animator.o: text_animator.cpp text_animator.h
	$(CXX) $(CXXFLAGS) -c text_animator.cpp

1310.o: 1310.cpp 1310.h text_animator.h
	$(CXX) $(CXXFLAGS) -c 1310.cpp

clean:
	rm -f $(TARGET) $(OBJECTS)

run: $(TARGET)
	./$(TARGET)