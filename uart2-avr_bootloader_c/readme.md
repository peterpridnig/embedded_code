# BOOTLOADER
pridnig, 29.12.2023

sh flash_avr.sh Init.hex
line#29 .equ   OSC = 74

./uart2-avr_bootloader --help ... display help

./uart2-avr_bootloader --reset ... applay reset, expect LED blink
./uart2-avr_bootloader --enter ... enter bootloader, expect "105"

./uart2-avr_bootloader -writehex and.hex .. write hexfile


