CFLAGS=-O3 --std=gnu++11
LFLAGS=-s
OBJ = bfk.o CVanillaState.o CExtendedState.o
INSTALL_PATH=/usr/local

bfk: $(OBJ)
	g++ $(LFLAGS) -o bfk $(OBJ)

CExtendedState.o: IBasicState.h CExtendedState.h CExtendedState.cpp
	g++ $(CFLAGS) -c -o CExtendedState.o CExtendedState.cpp

CVanillaState.o: IBasicState.h CVanillaState.h CVanillaState.cpp
	g++ $(CFLAGS) -c -o CVanillaState.o CVanillaState.cpp

bfk.o: IBasicState.h CVanillaState.h CExtendedState.h bfk.cpp
	g++ $(CFLAGS) -c -o bfk.o bfk.cpp

.PHONY: clean install

clean:
	rm -f bfk

install: bfk
	install -m 0755 bfk $(INSTALL_PATH)/bin/

remove:
	rm -fv $(INSTALL_PATH)/bin/bfk

