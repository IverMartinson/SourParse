COMPILER=gcc
FLAGS_ALL=-Wall -Wextra -Wno-unused-variable -Wno-unused-parameter -Wno-sequence-point
FLAGS_EXAMPLE=-Lbuild/ -lsourparse -Wl,-rpath=build/
FLAGS_LIB=-shared -fPIC

main.bin: sourparse.so
	$(COMPILER) $(FLAGS_ALL) src/launch_program/main.c -o build/main.bin $(FLAGS_EXAMPLE) 

sourparse.so:
	$(COMPILER) $(FLAGS_ALL) src/library/sourparse.c -o build/libsourparse.so $(FLAGS_LIB) 

clean:
	rm build/*
