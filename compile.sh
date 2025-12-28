gcc -c src/cmd/list/list.c -o obj/list.o
gcc -c src/cmd/submit/submit.c -o obj/submit.o
gcc -c src/cmd/stop/stop.c -o obj/stop.o
gcc -c src/cmd/restart/restart.c -o obj/restart.o
# gcc -c src/cmd/status/status.c -o build/status.o

gcc -c core/daemonize.c -o obj/daemonize.o
gcc -c core/state.c -o obj/state.o

gcc -c render/colors.c -o obj/colors.o
gcc -c render/icons.c -o obj/icons.o

gcc -c actions/list/list.c -o obj/action_list.o
gcc -c actions/submit/submit.c -o obj/action_submit.o
gcc -c actions/stop/stop.c -o obj/action_stop.o

gcc -c src/main.c -o obj/main.o

gcc obj/*.o -o bin/main
