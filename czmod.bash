#! /usr/bin/bash

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

CZPATH="$( cd "$(dirname "$CZMODPATH")" > /dev/null 2>&1 ; pwd -P )"
PATH="$CZPATH:$PATH"

case "$PROMPT_COMMAND" in
	*_zlua?--add*)
		PROMPT_COMMAND="${PROMPT_COMMAND/_zlua?--add/czmod --add}"
		;;
	*czmod?--add*)
		;;
	*_zlua_precmd*)
		;;
	*)
		echo "z.lua is not initialized"
		;;
esac

_zlua_precmd() {
    [ "$_ZL_PREVIOUS_PWD" = "$PWD" ] && return
    _ZL_PREVIOUS_PWD="$PWD"
    (czmod --add "$PWD" 2> /dev/null &)
}


