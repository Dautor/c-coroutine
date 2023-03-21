#/bin/sh
set -e
clfags='-Wall -Wextra -Weverything -pedantic -Werror -fsanitize=address -fsanitize=undefined -fsanitize=memory'
clang $cflags -c src/coroutine.c       -o build/coroutine.o
clang $cflags -c example/main.c -I src -o build/main.o
clang $cflags build/*.o -o build/example

