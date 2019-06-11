CXX=g++
CXX_FLAGS=-std=c++11
TARGET=simu
DEBUG=-g
SRCS=main.cpp
INC=./
OBJS=$(SRCS:.cpp=.o)

$(TARGET):$(OBJS)
	$(CXX) -I$(INC) $(CXX_FLAGS) $(DEBUG) $^ -o $@

.cpp.o:
	$(CXX) -I$(INC) $(CXX_FLAGS) $(DEBUG) -c $^ -o $@

clean:
	rm ./*.o ./$(TARGET)
