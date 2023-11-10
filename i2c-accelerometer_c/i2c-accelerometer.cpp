#include<iostream>
#include<unistd.h> //for usleep
#include"GPIO.h"
#include<fcntl.h>
#include<stdlib.h>
using namespace exploringBB;
using namespace std;

/* Address of the ADC */
#define I2C_ADDRESS 0x48

enum AnalogInput {AIN0, AIN1, AIN2, AIN3};

class ADC {
private:
  GPIO *RDY;
  void exportGPIO(string gpionr);
  void unexportGPIO(string gpionr);
/*
int GPIO::I2CstreamOpen(){
	stream.open((path + "value").c_str());
	return 0;
}
int GPIO::streamWrite(GPIO_VALUE value){
	stream << value << std::flush;
	return 0;
}
int GPIO::streamClose(){
	stream.close();
	return 0;
}
*/
  
public:
  ADC();
  void myblink();
  ~ADC();
};

ADC::ADC() {
  cout << "Init ADC" << endl;
  this->exportGPIO("122");
  RDY=new GPIO(122); //red
  RDY->setDirection(OUTPUT);
}

void ADC::exportGPIO(string gpionr){
  ofstream fs;
//string gpioexport="/sys/class/gpio/export";
//fs.open(gpioexport.c_str());
  fs.open("/sys/class/gpio/export");

  if (!fs.is_open()){
    perror("GPIO: write failed to open file ");
    exit(-1);
  }
  fs << gpionr;
  fs.close();  

}

void ADC::unexportGPIO(string gpionr){
  ofstream fs;
//string gpioexport="/sys/class/gpio/export";
//fs.open(gpioexport.c_str());
  fs.open("/sys/class/gpio/unexport");

  if (!fs.is_open()){
    perror("GPIO: write failed to open file ");
    exit(-1);
  }
  fs << gpionr;
  fs.close();  

}

void ADC::myblink() {
  for (int i {0};i<5;i++) {
   RDY->setValue(HIGH);
   usleep(100000);
   RDY->setValue(LOW);
   usleep(100000);
  }
}

ADC::~ADC() {
  delete RDY;
  this->unexportGPIO("122");
  cout << "Exit ADC" << endl;
}

int main(){
  cout << "Hello world" << endl;

  ADC ADS1015;
  ADS1015.myblink();
  
  return 0;
}

