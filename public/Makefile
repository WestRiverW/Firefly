######################################
#
#
######################################
  
#target you can change test to what you want
TARGET  := libpublic.so
  
#compile and lib parameter
CC      := g++
LIBS    :=
LDFLAGS :=
DEFINES :=
INCLUDE :=-I../core/ -I../pb/
CFLAGS  := -std=c++11 -fPIC -g -Wall -O3 $(DEFINES) $(INCLUDE)
CXXFLAGS:= $(CFLAGS) -DHAVE_CONFIG_H
SHARE   := -shared -lpthread -o 
  
#i think you should do anything here
  
#source file
SOURCE  := $(wildcard ./base/*.cpp) $(wildcard ./share/*.cpp)
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
	cp ./$(TARGET) ../lib
	rm -rf *.o
	
