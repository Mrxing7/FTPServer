CXX = g++
CFLAGS = -O2 -Wall -g -pthread

TARGET = server
OBJS = ../code/progress_bar/*.cpp ../code/ftp_server/*.cpp  				\
			../code/ftp_conn/*.cpp	../code/pool/*.cpp	../code/timer/*.cpp	\
		 	../code/main.cpp	

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o ../bin/$(TARGET)

clean:
	rm -rf ../bin/$(TARGET)