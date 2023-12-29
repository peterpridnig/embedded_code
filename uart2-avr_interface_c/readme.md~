# INTERFACE
pridnig, 29.12.2023

## OSCCAL

sh flash_avr.sh Interface.hex
line#21: .equ OSC = 74

./uart2-avr_interface -h ... help

microcom -s 9600 /dev/ttyS2
press RESET button; expect "Hi" (observe TX with LogicAnalyzer)

./uart2-avr_interface --reset --lifesign ... request lifesign, expect "100"
./uart2-avr_interface --osccalread       ... read OSCCAL, expect ".equ OSC"
./uart2-avr_interface --bytelengthread   ... read bytelength, expect 225
./uart2-avr_interface --osccalwrite 74   ... fine tune

## PORT CONFIG DIRECTION

./uart2-avr_interface --setddrb  8 ... set PB3 to output, PB4 to input

### LED CONTROL
./uart2-avr_interface --setportb 8 ... set PB3 HI
./uart2-avr_interface --setportb 0 ... set PB3 LO

### SWITCH READ
press SWITCH
./uart2-avr_interface --readportb 0 ... expect p4b=lo
release SWITCH
./uart2-avr_interface --readportb 0 ... expect p4b=hi
