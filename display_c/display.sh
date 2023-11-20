#!/bin/bash
# ----------------
# --- script
# ----------------

if [[ $# -eq 0 ]] ; then
    echo Usage
    echo 'led_blink RS|ENABLE|D4|D5|D6|D7'
    echo
    exit 1
fi

TARGET=/sys/class/gpio

ctrl_c() {
    echo $GPIO > $TARGET/unexport
    echo unexporting and exiting...
    exit
}

# RS
# Header: P8
# Pin:    #13
# BB_Pinmux.ods:
# => P8_13 / Addr 0x024
#
# cat /sys/kernel/debug/pinctrl/44e10800.pinmux-pinctrl-single/pins | grep 44e10824
# => pin 9 (PIN9) 23:gpio-96-127 44e10824 00000027 pinctrl-single
# i.e. PIN9
#
# cat /sys/kernel/debug/pinctrl/44e10800.pinmux-pinctrl-single/gpio-ranges | grep gpio-96-127
# => 22: gpio-96-127 GPIOS [118 - 119] PINS [8 - 9]
# i.e. GPIO119


# ENABLE
# Header: P8
# Pin:    #12
# BB_Pinmux.ods:
# => P8_12 / Addr 0x030
#
# cat /sys/kernel/debug/pinctrl/44e10800.pinmux-pinctrl-single/pins | grep 44e10830
# => pin 12 (PIN12) 12:gpio-0-31 44e10830 00000027 pinctrl-single 
# i.e. PIN12
#
# cat /sys/kernel/debug/pinctrl/44e10800.pinmux-pinctrl-single/gpio-ranges | grep gpio-0-31
# => 12: gpio-0-31 GPIOS [12 - 27] PINS [12 - 27]
# i.e. GPIO12


# D4
# Header: P8
# Pin:    #11
# BB_Pinmux.ods:
# => P8_11 / Addr 0x034
#
# cat /sys/kernel/debug/pinctrl/44e10800.pinmux-pinctrl-single/pins | grep 44e10834
# => pin 13 (PIN13) 13:gpio-0-31 44e10834 00000027 pinctrl-single
# i.e. PIN13
#
# cat /sys/kernel/debug/pinctrl/44e10800.pinmux-pinctrl-single/gpio-ranges | grep gpio-0-31
# => 12: gpio-0-31 GPIOS [12 - 27] PINS [12 - 27]
# i.e. GPIO13


# D5
# Header: P8
# Pin:    #8
# BB_Pinmux.ods:
# => P8_8 / Addr 0x094
#
# cat /sys/kernel/debug/pinctrl/44e10800.pinmux-pinctrl-single/pins | grep 44e10894
# => pin 37 (PIN37) 3:gpio-32-63 44e10894 00000037 pinctrl-single
# i.e. PIN37
#
# cat /sys/kernel/debug/pinctrl/44e10800.pinmux-pinctrl-single/gpio-ranges | grep gpio-32-63
# => 0: gpio-32-63 GPIOS [32 - 49] PINS [34 - 51]
# i.e. GPIO35


# D6
# Header: P8
# Pin:    #10
# BB_Pinmux.ods:
# => P8_10 / Addr 0x098
#
# cat /sys/kernel/debug/pinctrl/44e10800.pinmux-pinctrl-single/pins | grep 44e10898
# => pin 38 (PIN38) 4:gpio-32-63 44e10898 00000037 pinctrl-single
# i.e. PIN38
#
# cat /sys/kernel/debug/pinctrl/44e10800.pinmux-pinctrl-single/gpio-ranges | grep gpio-32-63
# => 0: gpio-32-63 GPIOS [32 - 49] PINS [34 - 51]
# i.e. GPIO36


# D7
# Header: P8
# Pin:    #9
# BB_Pinmux.ods:
# => P8_9 / Addr 0x09c
#
# cat /sys/kernel/debug/pinctrl/44e10800.pinmux-pinctrl-single/pins | grep 44e1089c
# => pin 39 (PIN39) 5:gpio-32-63 44e1089c 00000037 pinctrl-single 
# i.e. PIN39
#
# cat /sys/kernel/debug/pinctrl/44e10800.pinmux-pinctrl-single/gpio-ranges | grep gpio-32-63
# => 0: gpio-32-63 GPIOS [32 - 49] PINS [34 - 51]
# i.e. GPIO37


echo exporting...
# default
GPIO=119

case "$1" in
    RS) GPIO=119
	 ;;
    ENABLE) GPIO=12
	   ;;
    D4) GPIO=13
	;;
    D5) GPIO=35
	;;
    D6) GPIO=36
	;;
    D7) GPIO=37
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
