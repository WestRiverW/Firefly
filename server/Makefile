#!/bin/sh
all:
	cd .. && find . -name "*.sh" | xargs chmod 764
	cd pb && ./compile.sh
	
	cd ../core && make
	cd public && make
	cd pb && make
	
	cd db && make
	cd web && make
	cd hall && make
	cd game && make
	cd gate && make
	cd center && make
	
veryclean:
	cd ../core && make clean
	cd public && make clean
	cd pb && make clean
	
	cd db && make clean
	cd web && make clean
	cd hall && make clean
	cd game && make clean
	cd gate && make clean
	cd center && make clean
	
