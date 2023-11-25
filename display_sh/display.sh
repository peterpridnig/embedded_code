#!/bin/bash
# ----------------
# --- script
# ----------------

if [[ $# -eq 0 ]] ; then
    echo Usage
    echo 'display clear|message <text>'
    echo
    exit 1
fi

case "$1" in
    clear) TEXT='<clr>'
	 ;;
    message) TEXT=$2
	   ;;
esac

echo $TEXT > /dev/hd44780

