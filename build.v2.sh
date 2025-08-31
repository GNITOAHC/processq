mkdir -p test

gcc -c core/daemonize.c -o test/daemonize.o
gcc -c core/state.c -o test/state.o

gcc -c actions/submit/submit.c -o test/submit.o
gcc -c actions/list/list.c -o test/list.o
gcc -c actions/stop/stop.c -o test/stop.o
gcc -c main.v2.c -o test/main.o

gcc test/*.o -o test/main
