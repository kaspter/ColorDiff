
CC = clang++
CFLAGS = -g -O0 -Wall -Wextra
CFLAGS += `pkg-config --cflags opencv`
LDFLAGS +=`pkg-config --libs opencv`


TOOLS = HCO

CROSS_COMPILE ?= arm-none-eabi-

.PHONY: all clean

all: $(TOOLS)


clean:
	@rm -vf $(TOOLS) *.o


HCO: main.cpp Color.cpp ColorUtil.cpp MathUtil.cpp HoughCircle.cpp FindSquares.cpp
	$(CC) $(CFLAGS) -o $@ $(filter %.cpp,$^) $(LDFLAGS)
