#!/bin/bash
# ----------------
# --- script
# ----------------

#if [[ $# -eq 0 ]] ; then
#    echo Usage
#    echo 'led_blink red|green|pin'
#    echo
#    exit 1
#fi

TARGET=/sys/class/gpio

ctrl_c() {
    echo $GPIO > $TARGET/unexport
    echo unexporting and exiting...
    exit
}

# BUTTON
# Header: P8
# Pin:    #18
# BB_Pinmux.ods:
# => P8_18 = GPIO2_1 / Addr 0x08C
#
# cat /sys/kernel/debug/pinctrl/44e10800.pinmux-pinctrl-single/pins | grep 44e1088c
# => pin 35 (PIN35) 1:gpio-32-63 44e1088c 00000017 pinctrl-single
# i.e. PIN35
#
# cat /sys/kernel/debug/pinctrl/44e10800.pinmux-pinctrl-single/gpio-ranges | grep gpio-32-63
# => 0: gpio-32-63 GPIOS [32 - 49] PINS [34 - 51]
# i.e. GPIO33


echo exporting...
GPIO=33

echo $GPIO > $TARGET/export
echo in   > $TARGET/gpio$GPIO/direction 
echo "press [CTRL+C] to stop.."

trap ctrl_c INT

echo reading...

while true
do
    cat $TARGET/gpio$GPIO/value
    sleep 0.1
done
