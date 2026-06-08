# Build and run the host-side unit tests for coap-simple.
#
# Usage:
#   make            # build the test runner
#   make test       # build and run
#   make clean

CXX      ?= g++
CXXFLAGS ?= -std=c++11 -Wall -Wextra -g
ROOT     := ..

# mocks/ provides Arduino.h and Udp.h; ROOT provides coap-simple.h.
INCLUDES := -I mocks -I . -I $(ROOT)

SOURCES  := test_main.cpp test_examples.cpp $(ROOT)/coap-simple.cpp
HEADERS  := mocks/Arduino.h mocks/Udp.h FakeUDP.h CoapCodec.h TestHarness.h
TARGET   := runner

$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SOURCES) -o $(TARGET)

.PHONY: test
test: $(TARGET)
	./$(TARGET)

.PHONY: clean
clean:
	rm -f $(TARGET)
