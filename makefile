DEBUG_FLAG=0

DEBUG=-g -Wall
RELEASE=-O2 -s

INCLUDE=-I . -I ./data -I ./loss -I./optimizer
VPATH=.:./data:./loss:./optimizer

CFLAGS=-c $(INCLUDE) -Wno-write-strings
LDFLAGS=-lpthread

ifeq ($(DEBUG_FLAG),1)
	CFLAGS += $(DEBUG)
else
	CFLAGS += $(RELEASE)
endif

OBJS=test.o Params.o
TARGET=test

all:$(TARGET)

$(TARGET):$(OBJS)
	g++ $^ -o $@ $(LDFLAGS)

%.o:%.cpp
	g++ $< -o $@ $(CFLAGS)

.PHONY:clean
clean:
	-rm -f *.o $(TARGET) $(addsuffix .exe, $(TARGET))
