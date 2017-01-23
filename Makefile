CFLAGS=-O3 --std=gnu++11 -I "include/"
LFLAGS=-s
OBJD = obj
OBJ = $(OBJD)/bfk.o $(OBJD)/CVanillaState.o $(OBJD)/CExtendedState.o
SRCD = source
INSTALL_PATH=/usr/local

bfk: $(OBJ)
	g++ $(LFLAGS) -o bfk $(OBJ)

$(OBJD)/CExtendedState.o: $(SRCD)/IBasicState.h $(SRCD)/CExtendedState.h $(SRCD)/CExtendedState.cpp $(OBJD)
	g++ $(CFLAGS) -c -o $(OBJD)/CExtendedState.o $(SRCD)/CExtendedState.cpp

$(OBJD)/CVanillaState.o: $(SRCD)/IBasicState.h $(SRCD)/CVanillaState.h $(SRCD)/CVanillaState.cpp $(OBJD)
	g++ $(CFLAGS) -c -o $(OBJD)/CVanillaState.o $(SRCD)/CVanillaState.cpp

$(OBJD)/bfk.o: $(SRCD)/IBasicState.h $(SRCD)/CVanillaState.h $(SRCD)/CExtendedState.h $(SRCD)/bfk.cpp $(OBJD)
	g++ $(CFLAGS) -c -o $(OBJD)/bfk.o $(SRCD)/bfk.cpp

$(OBJD):
	mkdir $(OBJD)

.PHONY: clean install

clean:
	rm -f bfk

install: bfk
	install -m 0755 bfk $(INSTALL_PATH)/bin/

remove:
	rm -fv $(INSTALL_PATH)/bin/bfk

