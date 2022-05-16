default: run

compile:
	gcc userspace.c -o userspace

run: compile
	./userspace 'Hello World!'
	dmesg | tail -n 5