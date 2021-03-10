#target you can change to what you want
TARGET  := libjdddz.so
  
#compile and lib parameter
CC      := g++
LIBS    := -L../../lib/
LDFLAGS :=
DEFINES :=
INCLUDE := -I/usr/include/mysql/ -I/usr/include/ -I../../
CFLAGS  := -std=c++11 -fPIC -g -Wall -O3 $(DEFINES) $(INCLUDE)
CXXFLAGS:= $(CFLAGS) -DHAVE_CONFIG_H
SHARE   := -shared -lpthread -o 
  
#i think you should do anything here

#source file
SOURCE  := $(wildcard *.c) $(wildcard *.cpp) $(wildcard ./proto/*.cc)
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
	cp ./$(TARGET) ../../lib
	rm -rf *.o
	
