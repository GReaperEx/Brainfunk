FLAGS=-O3 -s --std=gnu++11
INSTALL_PATH=/usr/local

bfk: bfk.cpp
	g++ $(FLAGS) -o bfk bfk.cpp

.PHONY: clean install

clean:
	rm -f bfk

install:
	install -m 0755 $(INSTALL_PATH)/bin

