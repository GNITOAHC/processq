#!/bin/bash
mkdir -p obj bin

CC=${CC:-gcc}
VERSION_FLAG=${VERSION:+-DVERSION=\"$VERSION\"}

$CC -c src/cmd/list/list.c -o obj/list.o
$CC -c src/cmd/submit/submit.c -o obj/submit.o
$CC -c src/cmd/stop/stop.c -o obj/stop.o
$CC -c src/cmd/restart/restart.c -o obj/restart.o
# $CC -c src/cmd/status/status.c -o build/status.o

$CC -c core/daemonize.c -o obj/daemonize.o
$CC -c core/state.c -o obj/state.o

$CC -c render/colors.c -o obj/colors.o
$CC -c render/icons.c -o obj/icons.o

$CC -c actions/list/list.c -o obj/action_list.o
$CC -c actions/submit/submit.c -o obj/action_submit.o
$CC -c actions/stop/stop.c -o obj/action_stop.o

$CC $VERSION_FLAG -c src/main.c -o obj/main.o

$CC obj/*.o -o bin/main