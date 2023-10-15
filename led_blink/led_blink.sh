#!/bin/bash
# ----------------
# --- script
# ----------------

if [[ $# -eq 0 ]] ; then
    echo Usage
    echo 'led_blink red|green'
    echo
    exit 1
fi

TARGET=/sys/class/gpio

ctrl_c() {
    echo $GPIO > $TARGET/unexport
    echo unexporting and exiting...
    exit
}

# RED
# Header: P8
# Pin:    #14
# BB_Pinmux.ods:
# => P8_14 = GPIO0_26 / Addr 0x028
#
# cat /sys/kernel/debug/pinctrl/44e10800.pinmux-pinctrl-single/pins | grep 44e10828
# => pin 10 (PIN10) 26:gpio-96-127 44e10828 00000027 pinctrl-single 
# i.e. PIN10
#
# cat /sys/kernel/debug/pinctrl/44e10800.pinmux-pinctrl-single/gpio-ranges | grep gpio-96-127
# => 26: gpio-96-127 GPIOS [122 - 123] PINS [10 - 11]
# i.e. GPIO122

# GREEN
# Header: P8
# Pin:    #16
# BB_Pinmux.ods:
# => P8_16 = GPIO1_14 / Addr 0x038
#
# cat /sys/kernel/debug/pinctrl/44e10800.pinmux-pinctrl-single/pins | grep 44e10838
# => pin 14 (PIN14) 14:gpio-0-31 44e10838 00000027 pinctrl-single
# i.e. PIN14
#
# cat /sys/kernel/debug/pinctrl/44e10800.pinmux-pinctrl-single/gpio-ranges | grep gpio-0-31
# => 12: gpio-0-31 GPIOS [12 - 27] PINS [12 - 27]
# i.e. GPIO14

echo exporting...
GPIO=122

case "$1" in
    red) GPIO=122
    ;;
    green) GPIO=14
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
