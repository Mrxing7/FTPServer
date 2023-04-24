CXX = g++
CFLAGS = -Wall -g 

TARGET = client

OBJS = ../../code/progress_bar/*.cpp  ../demo/*.cpp \

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o $(TARGET)

clean:
	rm -rf $(TARGET)

