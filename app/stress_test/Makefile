CXX = g++
CFLAGS = -Wall -g 

TARGET = stress

OBJS = ../../code/progress_bar/*.cpp  ../stress_test/*.cpp \

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o $(TARGET)

clean:
	rm -rf $(TARGET)

