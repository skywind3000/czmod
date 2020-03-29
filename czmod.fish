#!/usr/bin/env fish

set SCRIPTPATH (realpath (dirname (status -f)))
set CZMODPATH "$SCRIPTPATH/czmod"

if [ ! -x "$CZMODPATH" ]
    set SCRIPTPATH (readlink """$argv[1]""")
    set SCRIPTPATH (cd "(dirname "$SCRIPTPATH")" >/dev/null 2>&1; pwd -P)
    set CZMODPATH "$SCRIPTPATH/czmod"
end

if [ ! -x "$CZMODPATH" ]
    set CZMODPATH (which czmod 2> /dev/null)
end

if [ ! -x "$CZMODPATH" ]
    echo "Error: Unable to find czmod executable on your \$PATH"
    exit 1
end

function _zlua_precmd --on-event fish_prompt
    "$CZMODPATH" --add "$PWD" 2> /dev/null &
end
