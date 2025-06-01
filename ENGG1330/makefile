CXX = g++
CXXFLAGS = -std=c++11 -Wall
LIBS = -lncursesw

TARGET = 1330

all: $(TARGET)

$(TARGET): main_1330.o 1330.o text_animator.o
	$(CXX) $(CXXFLAGS) -o $(TARGET) main_1330.o 1330.o text_animator.o $(LIBS)

main_1330.o: main_1330.cpp 1330.h text_animator.h
	$(CXX) $(CXXFLAGS) -c main_1330.cpp

1330.o: 1330.cpp 1330.h
	$(CXX) $(CXXFLAGS) -c 1330.cpp

text_animator.o: text_animator.cpp text_animator.h
	$(CXX) $(CXXFLAGS) -c text_animator.cpp

clean:
	rm -f *.o $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run