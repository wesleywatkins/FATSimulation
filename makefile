p3: main.o proj3.o customs.o helpers.o
	tar -xvf fat32.tar.gz && gcc -g -o p3 main.o proj3.o customs.o helpers.o -lm

main.o: main.c
	gcc -c -g main.c -lm

proj3.o: proj3.c
	gcc -c -g proj3.c -lm

customs.o: customs.c
	gcc -c -g customs.c -lm

helpers.o: helpers.c
	gcc -c -g helpers.c -lm

clean:
	rm *.o p3 fat32.img
