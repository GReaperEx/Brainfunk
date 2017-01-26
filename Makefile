CFLAGS=-O3 --std=gnu++11 -I "include/"
LFLAGS=-s
OBJD = obj
OBJ = $(OBJD)/bfk.o $(OBJD)/CVanillaState.o $(OBJD)/CExtendedState.o $(OBJD)/CExtended2State.o $(OBJD)/CExtended3State.o $(OBJD)/CLoveState.o $(OBJD)/CStackedState.o $(OBJD)/CBCDState.o $(OBJD)/CStuckState.o $(OBJD)/CJumpState.o $(OBJD)/CDollarState.o
SRCD = source
INSTALL_PATH=/usr/local

bfk: $(OBJ) $(OBJD)
	g++ $(LFLAGS) -o bfk $(OBJ)

$(OBJD)/CDollarState.o: $(SRCD)/IBasicState.h $(SRCD)/CDollarState.h $(SRCD)/CDollarState.cpp
	g++ $(CFLAGS) -c -o $(OBJD)/CDollarState.o $(SRCD)/CDollarState.cpp

$(OBJD)/CJumpState.o: $(SRCD)/IBasicState.h $(SRCD)/CJumpState.h $(SRCD)/CJumpState.cpp
	g++ $(CFLAGS) -c -o $(OBJD)/CJumpState.o $(SRCD)/CJumpState.cpp

$(OBJD)/CStuckState.o: $(SRCD)/IBasicState.h $(SRCD)/CStuckState.h $(SRCD)/CStuckState.cpp
	g++ $(CFLAGS) -c -o $(OBJD)/CStuckState.o $(SRCD)/CStuckState.cpp

$(OBJD)/CBCDState.o: $(SRCD)/IBasicState.h $(SRCD)/CBCDState.h $(SRCD)/CBCDState.cpp
	g++ $(CFLAGS) -c -o $(OBJD)/CBCDState.o $(SRCD)/CBCDState.cpp

$(OBJD)/CStackedState.o: $(SRCD)/IBasicState.h $(SRCD)/CStackedState.h $(SRCD)/CStackedState.cpp
	g++ $(CFLAGS) -c -o $(OBJD)/CStackedState.o $(SRCD)/CStackedState.cpp

$(OBJD)/CLoveState.o: $(SRCD)/IBasicState.h $(SRCD)/CLoveState.h $(SRCD)/CLoveState.cpp
	g++ $(CFLAGS) -c -o $(OBJD)/CLoveState.o $(SRCD)/CLoveState.cpp

$(OBJD)/CExtended3State.o: $(SRCD)/IBasicState.h $(SRCD)/CExtended3State.h $(SRCD)/CExtended3State.cpp
	g++ $(CFLAGS) -c -o $(OBJD)/CExtended3State.o $(SRCD)/CExtended3State.cpp

$(OBJD)/CExtended2State.o: $(SRCD)/IBasicState.h $(SRCD)/CExtended2State.h $(SRCD)/CExtended2State.cpp
	g++ $(CFLAGS) -c -o $(OBJD)/CExtended2State.o $(SRCD)/CExtended2State.cpp

$(OBJD)/CExtendedState.o: $(SRCD)/IBasicState.h $(SRCD)/CExtendedState.h $(SRCD)/CExtendedState.cpp
	g++ $(CFLAGS) -c -o $(OBJD)/CExtendedState.o $(SRCD)/CExtendedState.cpp

$(OBJD)/CVanillaState.o: $(SRCD)/IBasicState.h $(SRCD)/CVanillaState.h $(SRCD)/CVanillaState.cpp
	g++ $(CFLAGS) -c -o $(OBJD)/CVanillaState.o $(SRCD)/CVanillaState.cpp

$(OBJD)/bfk.o: $(SRCD)/IBasicState.h $(SRCD)/CVanillaState.h $(SRCD)/CExtendedState.h $(SRCD)/CExtended2State.h $(SRCD)/CExtended3State.h $(SRCD)/CLoveState.h $(SRCD)/bfk.cpp
	g++ $(CFLAGS) -c -o $(OBJD)/bfk.o $(SRCD)/bfk.cpp

$(OBJD):
	mkdir $(OBJD)

.PHONY: clean install

clean:
	rm -f bfk $(OBJ)

install: bfk
	install -m 0755 bfk $(INSTALL_PATH)/bin/

remove:
	rm -fv $(INSTALL_PATH)/bin/bfk

