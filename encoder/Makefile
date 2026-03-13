EXEC=tb_peripherals
CC=g++
CFLAGS=-std=c++17


${EXEC}: main.o peripherals.o Engine_test.o
	g++ -o ${EXEC} main.o peripherals.o Engine_test.o -lwiringPi

%.o: %.cpp $(wildcard *.h)
	$(CC) -c $< $(CFLAGS)

.PHONY: clean
clean:
	rm -rf ./${EXEC}
	rm -rf ./*.o



