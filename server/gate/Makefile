######################################
#
#
######################################
  
#target you can change test to what you want
#共享库文件名，lib*.so
TARGET  := gate
  
#compile and lib parameter
#编译参数
CC      := g++
LIBS    := -L./ -L/usr/local/lib/ -L/usr/local/lib64/ -L/usr/lib64/mysql/ -L../../lib/ -lcore -llua -ldl -lgflags -lpublic -lpbproto -lcurl -lhiredis
LDFLAGS :=
DEFINES :=
INCLUDE := -I/usr/local/include/-I/usr/include/mysql/ -I../ -I../../ 
CFLAGS  := -std=c++11 -fPIC -g -Wall -O3 $(DEFINES) $(INCLUDE)
CXXFLAGS:= $(CFLAGS) -DHAVE_CONFIG_H
SHARE   := -shared -lpthread -o 
  
#i think you should do anything here
#下面的基本上不需要做任何改动了
  
#source file
#源文件，自动找所有.c和.cpp文件，并将目标定义为同名.o文件
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
	cp ./$(TARGET) ../../lib
	rm -rf *.o
