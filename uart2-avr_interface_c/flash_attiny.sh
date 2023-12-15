#!/bin/bash
# ----------------
# --- script
# ----------------

if [[ $# -eq 0 ]] ; then
    echo Usage
    echo '(1) Connect usbasp interface'
    echo '(2) sh ./flash-avr.sh <file.hex>'
    echo
    exit 1
fi

echo Flashing...
avrdude -c usbasp -p t13 -U flash:w:$1:i -F
echo Done.

