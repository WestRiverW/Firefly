######################################
#
#
######################################
  
#target you can change test to what you want
TARGET  := game
  
#compile and lib parameter
CC      := g++
LIBS    := -L./ -L/usr/local/lib/ -L/usr/local/lib64/ -L/usr/lib64/mysql/ -L../../lib/ -lcore -llua -ldl -lgflags -lpublic -lpbproto -lcurl -lhiredis
LDFLAGS :=
DEFINES :=
INCLUDE := -I/usr/local/include/ -I/usr/include/mysql/ -I../../core/ -I../../public/ -I../../pb/
CFLAGS  := -std=c++11 -fPIC -g -Wall -O3 $(DEFINES) $(INCLUDE)
CXXFLAGS:= $(CFLAGS) -DHAVE_CONFIG_H
SHARE   := -shared -lpthread -o 
  
#i think you should do anything here
  
#source file
SOURCE  := $(wildcard *.c) $(wildcard *.cpp)
OBJS    := $(patsubst %.cc,%.o, $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE))))

.PHONY : everything objs clean veryclean rebuild
  
everything : $(TARGET)
  
all : $(TARGET)
  
objs : $(OBJS)
  
rebuild: veryclean everything
                
clean:

	rm -rf  *.o
    
veryclean: clean

	rm -rf $(TARGET)
  
$(TARGET): $(OBJS)
	$(CC) $(CXXFLAGS) -lprotobuf -lglog -lmysqlclient -lpthread -levent -o $@ $(OBJS) $(LDFLAGS) $(LIBS) 
	cp -f $(TARGET) ../../lib
	rm -rf *.o
