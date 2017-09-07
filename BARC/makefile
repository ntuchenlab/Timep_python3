$(info ncurses may need this: sudo apt-get install libncurses5-dev libncursesw5-dev )

main: BARS.o RPI.o rc_timer.o Hab.o AA.o
	gcc -Wall -O3 -o BARS BARS.o RPI.o rc_timer.o Hab.o AA.o
BARS.o: BARS.c
	gcc BARS.c -c
RPI.o: RPI.c
	gcc RPI.c -c
rc_timer.o: rc_timer.c
	gcc rc_timer.c -c
Hab.o: Hab.c
	gcc Hab.c -c
AA.o: AA.c
	gcc AA.c -c
clean:
	rm -rf BARS.o RPI.o rc_timer.o Hab.o AA.o
