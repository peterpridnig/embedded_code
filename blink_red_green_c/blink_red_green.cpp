/* A Simple GPIO application
* Written by Derek Molloy for the book "Exploring BeagleBone: Tools and
* Techniques for Building with Embedded Linux" by John Wiley & Sons, 2018
* ISBN 9781119533160. Please see the file README.md in the repository root
* directory for copyright and GNU GPLv3 license information.            */

// echo 122 > /sys/class/gpio/export
// echo 14 > /sys/class/gpio/export

// echo 122 > /sys/class/gpio/unexport

#include<iostream>
#include<unistd.h> //for usleep
#include"GPIO.h"
using namespace exploringBB;
using namespace std;

int main(){
  GPIO redGPIO(122), greenGPIO(14);

   // Basic Output - Flash the LED 10 times, once per second
   redGPIO.setDirection(OUTPUT);
   greenGPIO.setDirection(OUTPUT);

   //for (int i=0; i<10; i++){
   while (1){
      redGPIO.setValue(HIGH);
      greenGPIO.setValue(LOW);
      usleep(100000); //micro-second sleep 0.5 seconds
      redGPIO.setValue(LOW);
      greenGPIO.setValue(HIGH);
      usleep(100000);
   }

   return 0;
}
