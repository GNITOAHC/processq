gcc -std=c99 -fsyntax-only -Wall -Wextra -Wpedantic -Ithird_party/libargparse/include \
    src/main.c \
    src/cmd/*.c \
    render/*.c \
    core/*.c \
    actions/list/list.c actions/stop/stop.c actions/submit/submit.c
