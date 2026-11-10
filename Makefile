CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
LDFLAGS_CLIENT = -lcryptopp -lboost_program_options
LDFLAGS_TEST = -lunittest++ -lcryptopp -lboost_program_options

SRCS = interface.cpp data_reader.cpp data_writer.cpp communicator.cpp
OBJS = $(SRCS:.cpp=.o)

MAIN_SRC = main.cpp
MAIN_OBJ = main.o
TEST_SRC = tests.cpp
TEST_OBJ = tests.o

TARGET_CLIENT = client
TARGET_TEST = run_tests

all: $(TARGET_CLIENT) $(TARGET_TEST)

$(TARGET_CLIENT): $(OBJS) $(MAIN_OBJ)
	$(CXX) $(OBJS) $(MAIN_OBJ) -o $@ $(LDFLAGS_CLIENT)

$(TARGET_TEST): $(OBJS) $(TEST_OBJ)
	$(CXX) $(OBJS) $(TEST_OBJ) -o $@ $(LDFLAGS_TEST)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(MAIN_OBJ) $(TEST_OBJ) $(TARGET_CLIENT) $(TARGET_TEST)

.PHONY: all clean
