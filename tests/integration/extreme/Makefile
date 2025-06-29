CXX = g++
CXXFLAGS = -std=c++17 -Wall -I../../../src/binary_parser -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib -ltinyxml2

SRCS = test_extreme_parser.cpp ../../../src/binary_parser/binary_parser.cpp ../../../src/binary_parser/xml_struct_parser.cpp
TARGET = test_extreme_parser

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) extreme_test.bin

.PHONY: all run clean