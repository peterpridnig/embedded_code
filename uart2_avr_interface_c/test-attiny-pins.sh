#!/bin/bash
# ----------------
# --- script
# ----------------

if [[ $# -eq 0 ]] ; then
    echo Usage
    echo 'test-display RST'
    echo
    exit 1
fi

TARGET=/sys/class/gpio

ctrl_c() {
    echo $GPIO > $TARGET/unexport
    echo unexporting and exiting...
    exit
}

# RST
# Header: P9
# Pin:    #23
# BB_Pinmux.ods:
# => P9_23 / Addr 0x044
#
# cat /sys/kernel/debug/pinctrl/44e10800.pinmux-pinctrl-single/pins | grep 44e10844
# => pin 17 (PIN17) 17:gpio-0-31 44e10844 00000027 pinctrl-single 
# i.e. PIN17
#
# cat /sys/kernel/debug/pinctrl/44e10800.pinmux-pinctrl-single/gpio-ranges | grep gpio-0-31
# => 12: gpio-0-31 GPIOS [12 - 27] PINS [12 - 27]
# i.e. GPIO17


echo exporting...
# default
GPIO=17

case "$1" in
    RST) GPIO=17
	 ;;
esac

echo $GPIO > $TARGET/export
echo out   > $TARGET/gpio$GPIO/direction 
echo "press [CTRL+C] to stop.."

trap ctrl_c INT

echo blinking...

while true
do
    echo 1 > $TARGET/gpio$GPIO/value
    echo ON
    sleep 1
    echo 0 > $TARGET/gpio$GPIO/value
    echo OFF
    sleep 1
done
