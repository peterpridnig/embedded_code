# INTERFACE
pridnig, 29.12.2023

## OSCCAL

sh flash_avr.sh Interface.hex
line#21: .equ OSC = 74
sh write-fuses_avr.sh

./uart2-avr_interface -h ... help

microcom -s 9600 /dev/ttyS2
press RESET button; expect "Hi" (observe TX with LogicAnalyzer)

./uart2-avr_interface --reset --lifesign ... request lifesign, expect "100"
./uart2-avr_interface --osccalread       ... read OSCCAL, expect ".equ OSC"
./uart2-avr_interface --bytelengthread   ... read bytelength, expect 225
./uart2-avr_interface --osccalwrite 74   ... fine tune
./uart2-avr_interface --fuseread         ... read fuse bytes, expect eb(hi) 6a(lo)


## PORT CONFIG DIRECTION

./uart2-avr_interface --setddrb  9 ... set PB3=PB0=output, PB4=input

### LED CONTROL
./uart2-avr_interface --setportb 9 ... set PB3=PB4=HI
./uart2-avr_interface --setportb 0 ... set PB3=PB4=LO

### SWITCH READ
press SWITCH
./uart2-avr_interface --readportb 0 ... expect p4b=lo
release SWITCH
./uart2-avr_interface --readportb 0 ... expect p4b=hi
