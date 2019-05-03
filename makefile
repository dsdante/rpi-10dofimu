accel: common.o rpi.o imu10dof.o
	gcc -std=c11 $(CFLAGS) -o accel main.c common.o rpi.o imu10dof.o

imu10dof.o: imu10dof.c imu10dof.h rpi.o
	gcc -std=c11 $(CFLAGS) -c imu10dof.c

rpi.o: rpi.c rpi.h common.o
	gcc -std=c11 $(CFLAGS) -c rpi.c

common.o: common.c common.h 
	gcc -std=c11 $(CFLAGS) -c common.c

all: accel

clean:
	rm -f *.o *.out accel
