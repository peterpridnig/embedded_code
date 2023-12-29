# BOOTLOADER
pridnig, 29.12.2023

## INIT
bringup INTERFACE

sh flash_avr.sh Init.hex
line#29 .equ   OSC = 74 (ensure same content as in Interface.hex)
sh read-efuses_avr.sh ( ensure eb(hi) 6a(lo) )

press RESET button; expect blinking LED

## DOWNLOAD
./uart2-avr_bootloader --help ... display help

./uart2-avr_bootloader --reset ... applay reset, expect LED blink
./uart2-avr_bootloader --enter ... enter bootloader, expect "105"

./uart2-avr_bootloader --writehex <hexfile.hex> .. write hexfile

### download blink2.hex
./uart2-avr_bootloader --writehex blink2.hex .. write hexfile


