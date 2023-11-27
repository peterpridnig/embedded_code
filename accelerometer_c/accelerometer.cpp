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
#define FIXED_INT(x) std::setw(5) << std::fixed << std::setprecision(0)<<(x)

enum States {
  IDLE, //->CAL
  CALIBRATION1, //->CAL2
  CALIBRATION2, //->CAL3
  CALIBRATION3, //->CONV
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
    switch (this->state)
      { case IDLE: this->state=CALIBRATION1; break;
      case CALIBRATION1: this->state=CALIBRATION2; break;
      case CALIBRATION2: this->state=CALIBRATION3; break;
      case CALIBRATION3: this->state=CONVERTING; break;
      case CONVERTING: this->state=IDLE; break;
      case ERROR:  this->state=IDLE; break;
      default: this->state=IDLE;
      };
  }
};


int ButtonPressed=0;

int callbackFunction(int var){
  /*
    struct timespec ts;
  
    if (!timespec_get(&ts, TIME_UTC)) {
    perror("timestamp");
    return -1;
    }
  
    cout << "BBB Button Pressed! seconds: " << ts.tv_sec << " ns: " << ts.tv_nsec << " " <<  var << endl;
  */
  ButtonPressed=1;
  return var;
}


void DisplayIdle(std::ofstream* ds)
{
  (*ds) << "<clr>" << std::endl;
  (*ds) << "<pos 0,3>Accelerometer" << endl;
  (*ds) << "<pos 1,3>    demo" << endl;
  (*ds) << "<pos 2,3>Peter Pridnig" << endl;
  (*ds) << "<pos 3,3> 24.11.2023" << endl;
  
}

void DisplayCalibration1(std::ofstream* ds)
{
  (*ds) << "<clr>" << std::endl;
  (*ds) << "<pos 0,4>CAL x=y=0deg" << endl;
  (*ds) << "<pos 1,1>x-axis:      lsb12" << std::endl;
  (*ds) << "<pos 2,1>y-axis:      lsb12" << std::endl;
  //display << "<pos 3,2>z-axis:     lsb12" << std::endl;
  (*ds) << "<pos 3,4>PRESS BUTTON" << endl;
}

void DisplayCalibration2(std::ofstream* ds)
{
  (*ds) << "<clr>" << std::endl;
  (*ds) << "<pos 0,4>CAL y=45deg" << endl;
  (*ds) << "<pos 1,1>delta:       lsb12" << std::endl;
  (*ds) << "<pos 2,1>y-axis:      lsb12" << std::endl;
  //display << "<pos 3,2>z-axis:     lsb12" << std::endl;
  (*ds) << "<pos 3,4>PRESS BUTTON" << endl;
}

void DisplayCalibration3(std::ofstream* ds, int* x0, int* y0, double* delta)
{
  (*ds) << "<clr>" << std::endl;
  (*ds) << "<pos 0,5>CAL RESULT" << endl;
  (*ds) << "<pos 1,2>x0:  " << FIXED_INT(*x0) << " lsb12" << std::endl;
  (*ds) << "<pos 2,2>y0:  " << FIXED_INT(*y0)  << " lsb12" << std::endl;
  //display << "<pos 3,2>z-axis:     lsb12" << std::endl;
  (*ds) << "<pos 3,2>delta: " << (int)(*delta) << " lsb12" << endl;
}

void DisplayConverting(std::ofstream* ds)
{
  (*ds) << "<clr>" << std::endl;
  (*ds) << "<pos 0,5>CONVERTING" << endl;
  (*ds) << "<pos 1,2>x-axis:     deg" << std::endl;
  (*ds) << "<pos 2,2>y-axis:     deg" << std::endl;
  //display << "<pos 3,2>z-axis:     deg" << std::endl;
}

void DisplayError(std::ofstream* ds)
{
  (*ds) << "<clr>" << std::endl;
  (*ds) << "<pos 0,7>ERROR" << endl;
}

int main()
{
  std::ofstream display;

  cStateMachine SM {};
  
  ADS1015* adc = new ADS1015(true);
 
  int x=0;
  int y=0;
  //int z=0;

  int x0=0;
  int y0=0;
  //int z0=0;
  
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


  /*
  display << "<clr>" << std::endl;
 
  display << "<pos 1,2>x-axis:     deg" << std::endl;
  display << "<pos 2,2>y-axis:     deg" << std::endl;
  //display << "<pos 3,2>z-axis:     deg" << std::endl;
  std::cout <<  std::setprecision(2);

  x0 = adc->ConvertPoll(AIN2);
  y0 = adc->ConvertPoll(AIN1);
  //z0 = adc->ConvertPoll(AIN0);

  delta=1700-1380;
  */
  
  DisplayIdle(&display);

  while (1) {
    
    if (SM.GetState()==CONVERTING) {
      x = adc->ConvertPoll(AIN2);
      y = adc->ConvertPoll(AIN1);
      //z = adc->ConvertPoll(AIN0);
   
      xdeg = asin( (double)(x-x0)/delta  ) / myPI * 180;
      ydeg = asin( (double)(y-y0)/delta  ) / myPI * 180;
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
    }

    if (SM.GetState()==CALIBRATION1) {
      x0 = adc->ConvertPoll(AIN2);
      y0 = adc->ConvertPoll(AIN1);
      //z0 = adc->ConvertPoll(AIN0);
   
      display << "<pos 1,9>" <<  x0 << std::endl;
      display << "<pos 2,9>" <<  y0 << std::endl;
      //display << "<pos 3,9>" <<  FIXED_FLOAT(abs(zdeg)) << std::endl;
    }

    if (SM.GetState()==CALIBRATION2) {
      //x = adc->ConvertPoll(AIN2);
      y = adc->ConvertPoll(AIN1);
      //z = adc->ConvertPoll(AIN0);
   
      display << "<pos 1,9>" <<  abs(y-y0) << std::endl;
      display << "<pos 2,9>" <<  y0 << std::endl;
      //display << "<pos 3,9>" <<  FIXED_FLOAT(abs(zdeg)) << std::endl;
      
      delta = abs(y-y0)/sin(M_PI/4); //extrapolate to 90deg
    }
    

    
    usleep( 500000 ); //50ms

    if (ButtonPressed) {
      ButtonPressed=0;
      display << "<pos 0,0>" << "Button pressed" << endl;
      SM.AdvanceState();
      switch (SM.GetState()) {
      case IDLE:         DisplayIdle(&display);         break;
      case CALIBRATION1: DisplayCalibration1(&display); break;
      case CALIBRATION2: DisplayCalibration2(&display); break;
      case CALIBRATION3: DisplayCalibration3(&display, &x0, &y0, &delta); break;	
      case CONVERTING:   DisplayConverting(&display);   break;
      case ERROR:        DisplayError(&display);        break;
      default:           DisplayIdle(&display); 
      }
    }
  }

  display.close();

  delete (adc);
 
  btnGPIO->unexportGPIO();
  free(btnGPIO);
 
  return 0;
}
