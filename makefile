all: main.o qbbbe-enc.exe

main.o: main.c
	gcc main.c -o main.o -c

qbbbe-enc.exe: main.o
	gcc main.o -o qbbbe-enc.exe

clean:
	rm qbbbe-enc.exe
	rm main.o