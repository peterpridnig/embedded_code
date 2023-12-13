#!/bin/bash
# ----------------
# --- script
# ----------------

if [[ $# -eq 0 ]] ; then
    echo Usage
    echo 'flash-avr <file.hex>'
    echo
    exit 1
fi

echo Connect usbasp interface
echo Flashing...
avrdude -c usbasp -p t13 -U flash:w:$1:i -F
echo Done.

