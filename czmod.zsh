#! /usr/bin/zsh

SCRIPTPATH="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
CZMODPATH="$SCRIPTPATH/czmod"

if [ ! -x "$CZMODPATH" ]; then
	SCRIPTPATH="$(readlink """$0""")"
	SCRIPTPATH="$( cd "$(dirname "$SCRIPTPATH")" >/dev/null 2>&1 ; pwd -P )"
	CZMODPATH="$SCRIPTPATH/czmod"
fi

if [ ! -x "$CZMODPATH" ]; then
	CZMODPATH="$(which czmod 2> /dev/null)"
fi

if [ ! -x "$CZMODPATH" ]; then
	echo "Error: not find czmod executable in your PATH !!" 
	return
fi

_zlua_precmd() {
	("$CZMODPATH" --add "${PWD:a}" &)
}


