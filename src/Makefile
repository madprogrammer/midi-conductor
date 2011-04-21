CC = c++
OUT = midi
SRC = Midi.cpp File.cpp Device.cpp Memory.cpp main.cpp
OBJS = Midi.o File.o Memory.o Device.o main.o
CFLAGS = -I../include -g -Wall -O2
LDFLAGS = -lm -lpthread

all: $(OUT)
$(OUT): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $(OUT)
$(OBJS):
	$(CC) $(CFLAGS) -c $(SRC)

clean:
	rm -f *.o $(OUT)
