OUT  = midi
SRC  = Midi.cpp File.cpp Device.cpp Memory.cpp main.cpp
OBJS = $(SRC:.cpp=.o)

CXXFLAGS   = -I../include -g -Wall -O2
CXXLDFLAGS = -lm -lpthread

all: $(OUT)

$(OUT): $(OBJS)
	$(CXX) $^ $(CXXLDFLAGS) -o $(OUT)

clean:
	rm -f $(OBJS) $(OUT)
