TARGET=program
CXX=g++
DEBUG=-g
INC=./
CXX_FLAGS=-std=c++11
SRCS=main.cpp\
	tcp_client.cpp\
	data_fifo.cpp\
	algorithm.cpp
OBJS=$(SRCS:.cpp=.o)

$(TARGET):$(OBJS)
	$(CXX) -I$(INC) $(DEBUG) $(CXX_FLAGS) $^ -o $@

.cpp.o:
	$(CXX) -I$(INC) $(DEBUG) $(CXX_FLAGS) -c $^ -o $@

clean:
	rm ./*.o ./$(TARGET)
