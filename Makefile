CFLAGS=-O3 --std=gnu++11 -I "include/"
LFLAGS=-s
SRCD = source
SRC = $(wildcard $(SRCD)/*.cpp)
OBJD = obj
OBJ = $(patsubst $(SRCD)/%.cpp,$(OBJD)/%.o,$(SRC))
TSTD = tests
TST = $(patsubst %.bf,%,$(wildcard $(TSTD)/*.bf))
INSTALL_PATH=/usr/local

bfk: $(OBJ) $(OBJD)
	g++ $(LFLAGS) -o bfk $(OBJ)

$(OBJD)/bfk.o: $(SRCD)/*.h $(SRCD)/bfk.cpp
	g++ $(CFLAGS) -c -o $(OBJD)/bfk.o $(SRCD)/bfk.cpp

$(OBJD)/%.o: $(SRCD)/%.cpp $(SRCD)/%.h $(SRCD)/IBasicState.h
	g++ $(CFLAGS) -c -o $@ $<

$(OBJD):
	mkdir $(OBJD)

.PHONY: clean install remove test help

clean:
	rm -f bfk $(OBJD)/*.o $(TSTD)/*.out a.out

install: bfk
	install -m 0755 bfk $(INSTALL_PATH)/bin/

remove:
	rm -fv $(INSTALL_PATH)/bin/bfk

test: bfk
	@for testfile in $(TST); do \
		echo Testing: $$(basename "$$testfile"); \
		./bfk $$(cat $$testfile'.use') $$testfile'.bf' < $$testfile'.in' > $$testfile'.out' && sync && \
		diff $$testfile'.val' $$testfile'.out'; \
		echo Testing: $$(basename "$$testfile")'-c'; \
		./bfk -c $$(cat $$testfile'.use') $$testfile'.bf' || continue; \
		sync && ./a.out < $$testfile'.in' > $$testfile'.out' && sync && \
		diff $$testfile'.val' $$testfile'.out'; \
	done

help:
	@echo "make        : "
	@echo "make bfk    : Compiles the program"
	@echo "make test   : Compiles the program and runs tests"
	@echo "make install: Compiles the program and installs it on the system"
	@echo "make remove : Uninstalls the program"
	@echo "make clean  : Erases any compilation or testing generated files"
