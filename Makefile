CFLAGS=-O3 --std=gnu++11 -I "include/"
LFLAGS=-s
SRCD = source
SRC = $(wildcard $(SRCD)/*.cpp)
OBJD = obj
OBJ = $(patsubst $(SRCD)/%.cpp,$(OBJD)/%.o,$(SRC))
INSTALL_PATH=/usr/local

bfk: $(OBJ) $(OBJD)
	g++ $(LFLAGS) -o bfk $(OBJ)

$(OBJD)/bfk.o: $(SRCD)/*.h $(SRCD)/bfk.cpp
	g++ $(CFLAGS) -c -o $(OBJD)/bfk.o $(SRCD)/bfk.cpp

$(OBJD)/%.o: $(SRCD)/%.cpp $(SRCD)/%.h $(SRCD)/IBasicState.h
	g++ $(CFLAGS) -c -o $@ $<

$(OBJD):
	mkdir $(OBJD)

.PHONY: clean install remove

clean:
	rm -f bfk $(OBJ)

install: bfk
	install -m 0755 bfk $(INSTALL_PATH)/bin/

remove:
	rm -fv $(INSTALL_PATH)/bin/bfk

