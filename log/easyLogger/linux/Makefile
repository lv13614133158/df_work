CC = gcc

ROOTPATH=..
INCLUDE =  -I$(ROOTPATH)/easylogger/inc -I$(ROOTPATH)/easylogger/plugins/file -I ./inc
LIB=-lpthread

OBJ += $(wildcard *.c)
OBJ += $(wildcard $(ROOTPATH)/easylogger/src/*.c)
OBJ += $(wildcard $(ROOTPATH)/easylogger/plugins/file/elog_file.c)
OBJ += $(wildcard easylogger/port/*.c)

CFLAGS = -O0 -g3 -Wall
target = a.out

all:$(OBJ)
	$(CC) $(OBJ)  $(INCLUDE) -o $(target) $(LIB)


clean:
	rm $(target)