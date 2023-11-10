#!/bin/sh
#arm-cortex_a8-linux-gnueabihf-g++ -Wall -lg i2c-accelerometer.cpp GPIO.cpp -static -o i2c-accelerometer
arm-buildroot-linux-gnueabihf-g++ -Wall -lg i2c-accelerometer.cpp GPIO.cpp -o i2c-accelerometer
scp i2c-accelerometer root@10.0.0.117:/root
