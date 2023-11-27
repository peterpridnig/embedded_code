#include <cstring>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <iomanip>
#include <ads1015.h>
#include <unistd.h> //for usleep
#include <cmath>

#include <time.h>
#include <unistd.h>
#include <signal.h>

#define myPI 3.141592653589
#define FIXED_FLOAT(x) std::setw(5) << std::fixed << std::setprecision(1)<<(x)


enum States {
  IDLE, //->CAL
  CALIBRATION, //->CONV
  CONVERTING, //-> IDLE
  ERROR //->IDLE
};

class cStateMachine {
public:
  cStateMachine() {
    this->state=IDLE;
  }
  States state;
  States GetState() {return this->state; }
  void AdvanceState() {
    if (this->state==IDLE        ) {this->state=CALIBRATION; return 0;}
    if (this->state==CALIBRATION ) {this->state=CONVERTING;  return 0;}
    if (this->state==CONVERTING  ) {this->state=IDLE;        return 0;}
    if (this->state==ERROR       ) {this->state=IDLE;        return 0;}
  }
};


int ButtonPressed=0;

int callbackFunction(int var){

  struct timespec ts;
  
  if (!timespec_get(&ts, TIME_UTC)) {
    perror("timestamp");
    return -1;
  }
  
  cout << "BBB Button Pressed! seconds: " << ts.tv_sec << " ns: " << ts.tv_nsec << " " <<  var << endl;
  ButtonPressed=1;
  return var;
}


int main()
{
  std::ofstream display;

  cStateMachine SM {};
  
  ADS1015* adc = new ADS1015(true);
 
  double x=0;
  double y=0;
  //double z=0;

  double x0=0;
  double y0=0;
  //double z0=0;
  
  double delta=0;

  double xdeg=0;
  double ydeg=0;
  //double zdeg=0;

  GPIO* btnGPIO = new GPIO(33);
  btnGPIO->exportGPIO();
  btnGPIO->setDirection(INPUT);
  btnGPIO->setEdgeType(FALLING);
  btnGPIO->setDebounceTime(200);
  btnGPIO->waitForEdge(&callbackFunction);
  
  display.open("/dev/hd44780");
  if (!display.is_open()) {
    perror("cannot open /dev/hd44780\n");
    perror("perform insmod hd44780.ko\n");
    return -1;
  }


  
  display << "<clr>" << std::endl;
 
  display << "<pos 1,2>x-axis:     deg" << std::endl;
  display << "<pos 2,2>y-axis:     deg" << std::endl;
  //display << "<pos 3,2>z-axis:     deg" << std::endl;
  std::cout <<  std::setprecision(2);

  x0 = adc->ConvertPoll(AIN2);
  y0 = adc->ConvertPoll(AIN1);
  //z0 = adc->ConvertPoll(AIN0);

  delta=1700-1380;
  

 while (1) {
   x = adc->ConvertPoll(AIN2);
   y = adc->ConvertPoll(AIN1);
   //z = adc->ConvertPoll(AIN0);
   
   xdeg = asin( (x-x0)/delta  ) / myPI * 180;
   ydeg = asin( (y-y0)/delta  ) / myPI * 180;
   //zdeg = asin( (z-z0)/delta  ) / myPI * 180;

   if (x-x0 >  delta) x=x0+delta;
   if (x-x0 < -delta) x=x0-delta;
   if (y-y0 >  delta) y=y0+delta;
   if (y-y0 < -delta) y=y0-delta;
   //if (z-z0 >  delta) z=z0+delta;
   //if (z-z0 < -delta) z=z0-delta;
   
   display << "<pos 1,9>" <<  FIXED_FLOAT(abs(xdeg)) << std::endl;
   display << "<pos 2,9>" <<  FIXED_FLOAT(abs(ydeg)) << std::endl;
   //display << "<pos 3,9>" <<  FIXED_FLOAT(abs(zdeg)) << std::endl;
									     
   usleep( 500000 ); //50ms

   if (ButtonPressed) {
     //SM.AdvanceState();
     ButtonPressed=0; display << "<pos 0,0>" << "Button pressed" << endl;
   }
 }

 display.close();

 delete (adc);
 
 btnGPIO->unexportGPIO();
 free(btnGPIO);
 
 return 0;
}
