#!/bin/sh
all:
	find . -name "*.sh" | xargs chmod 764
	cd core && make
	cd pb && ./compile.sh && make
	cd public && make
	cd server && make
	cd all-game && make
	
clean:
	cd all-game && make clean
	cd server && make clean
	cd public && make clean
	cd pb && make clean
	cd core && make clean
	
