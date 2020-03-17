#! /bin/sh

CC="$(which musl-gcc 2> /dev/null)"

if [ ! -x "$CC" ]; then
	echo "Error: musl-gcc not find, install it with:"
	echo "sudo apt-get install musl-tools"
	exit 1
fi

SCRIPTPATH="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"

cd "$SCRIPTPATH"

$CC -O3 -o czmod czmod.c system/imembase.c system/iposix.c -static -s

if [ "$?" -eq 0 ]; then
	echo "success, binary 'czmod' is ready"
else
	exit 2
fi

