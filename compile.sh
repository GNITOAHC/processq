#!/bin/bash
mkdir -p obj bin

rm obj/*.o

CC=${CC:-gcc}
VERSION_FLAG=${VERSION:+-DVERSION=\"$VERSION\"}

THIRD_PARTY_INCLUDE="-Ithird_party/libargparse/include"
THIRD_PARTY_LIB="-Lthird_party/libargparse -largparse"

cd third_party/libargparse && make && cd ../..

$CC $THIRD_PARTY_INCLUDE -c src/cmd/amalgamation.c -o obj/cmd.o

$CC -c core/daemonize.c -o obj/daemonize.o
$CC -c core/state.c -o obj/state.o

$CC -c render/colors.c -o obj/colors.o
$CC -c render/icons.c -o obj/icons.o

$CC -c actions/list/list.c -o obj/action_list.o
$CC -c actions/submit/submit.c -o obj/action_submit.o
$CC -c actions/stop/stop.c -o obj/action_stop.o

$CC $VERSION_FLAG -c src/main.c $THIRD_PARTY_INCLUDE -o obj/main.o

$CC obj/*.o $THIRD_PARTY_LIB -o bin/main
