CFLAGS=-O3 --std=gnu++11 -I "include/"
LFLAGS=-s
SRCD = source
SRC = $(wildcard $(SRCD)/*.cpp)
OBJD = obj
OBJ = $(patsubst $(SRCD)/%.cpp,$(OBJD)/%.o,$(SRC))
TSTD = tests
INSTALL_PATH=/usr/local

bfk: $(OBJ) $(OBJD)
	@echo Linking: $@
	@g++ $(LFLAGS) -o bfk $(OBJ)

$(OBJD)/bfk.o: $(SRCD)/bfk.cpp $(SRCD)/*.h
	@echo Compiling: $(<F)
	@g++ $(CFLAGS) -c -o $(OBJD)/bfk.o $(SRCD)/bfk.cpp

$(OBJD)/%.o: $(SRCD)/%.cpp $(SRCD)/%.h $(SRCD)/IBasicState.h
	@echo Compiling: $(<F)
	@g++ $(CFLAGS) -c -o $@ $<

$(OBJD):
	@mkdir $(OBJD)

.PHONY: clean clean-test install remove test re-test help

clean:
	@rm -f bfk $(OBJD)/*.o $(TSTD)/*.out $(TSTD)/*.exe

clean-test:
	@rm -f $(TSTD)/*.out $(TSTD)/*.exe

install: bfk
	@install -m 0755 bfk $(INSTALL_PATH)/bin/

remove:
	@rm -fv $(INSTALL_PATH)/bin/bfk

test: bfk
	@sync bfk
	@$(MAKE) --silent -C $(TSTD)/

re-test:
	@$(MAKE) --silent clean-test
	@$(MAKE) --silent test

help:
	@echo "make            : Compiled the program"
	@echo "make bfk        : Compiles the program"
	@echo "make test       : Compiles the program and runs tests"
	@echo "make re-test    : Compiles the program and re-runs tests"
	@echo "make install    : Compiles the program and installs it on the system"
	@echo "make remove     : Uninstalls the program"
	@echo "make clean      : Erases any compilation or testing generated files"
	@echo "make clean-test : Erases only testing generated files"

