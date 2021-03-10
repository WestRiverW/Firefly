######################################
#
#
######################################
  
#target you can change test to what you want
TARGET  := libcore.so
  
#compile and lib parameter
CC      := g++
LIBS    :=-L/usr/local/lib64/
LDFLAGS :=
DEFINES :=
INCLUDE :=-I/usr/include/mysql/ -I/usr/include/ -I/usr/local/include/ -I./ -I./common/ -I./utils/ -I./net -I../
CFLAGS  := -std=c++11 -fPIC -g -Wall -O3 $(DEFINES) $(INCLUDE)
CXXFLAGS:= $(CFLAGS) -DHAVE_CONFIG_H
SHARE   := -shared -lpthread -o 
  
#i think you should do anything here
  
#source file
SOURCE  := $(wildcard *.c) $(wildcard ./common/*.cpp) $(wildcard ./config/*.cpp) $(wildcard ./data/*.cpp) $(wildcard ./external/json/*.cpp) $(wildcard ./external/sha1/*.cpp) $(wildcard ./net/*.cpp) $(wildcard ./redis/*.cpp) $(wildcard ./utils/*.cpp)
OBJS    := $(patsubst %.cc,%.o,$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE))))

.PHONY : everything objs clean veryclean rebuild
  
everything : $(TARGET)
  
all : $(TARGET)
  
objs : $(OBJS)
  
rebuild: veryclean everything
                
clean:

	rm -rf  $(OBJS)
    
veryclean: clean

	rm -rf $(TARGET)
  
$(TARGET): $(OBJS)
	$(CC) $(CXXFLAGS) $(SHARE) $@ $(OBJS) $(LDFLAGS) $(LIBS)
	cp  ./$(TARGET) ../lib/ 
	
