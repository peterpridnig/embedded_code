#include <ads1015.h>
#include"GPIO.h"

#include<iostream>
#include<unistd.h> //for usleep
#include<fcntl.h>
#include<stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

using namespace exploringBB;
using namespace std;

ADS1015::ADS1015() {
  cout << "Init ADC" << endl;
   
  this->exportGPIO(RDY_PIN);
  RDY=new GPIO(stoi(RDY_PIN)); //red
  RDY->setDirection(OUTPUT);

  i2c_fd = open(I2C_DEVICE, O_RDWR);
  if (i2c_fd < 0) {
    perror(I2C_DEVICE);
    exit(-1);
   }
  
  if (ioctl(i2c_fd, I2C_SLAVE, I2C_ADDRESS) == -1) {
    perror("ioctl I2C_SLAVE");
    exit(-1);
  }


  this->Reset();
  this->EnableRdy();
  
}

ADS1015::~ADS1015() {
  delete RDY;
  this->unexportGPIO(RDY_PIN);
  close(i2c_fd);
  cout << "Exit ADC" << endl;
}

void ADS1015::exportGPIO(string gpionr){
  ofstream fs;
  //fs.open("/sys/class/gpio/export");
  string EXPORT_FILE  = "export";
  fs.open(GPIO_PATH+EXPORT_FILE);
  
  if (!fs.is_open()){
    perror("GPIO: write failed to open file ");
    exit(-1);
  }

  fs << gpionr;
  fs.close();  
}


void ADS1015::unexportGPIO(string gpionr){
  ofstream fs;
  //fs.open("/sys/class/gpio/unexport");
  string UNEXPORT_FILE  = "unexport";
  fs.open(GPIO_PATH+UNEXPORT_FILE);

  if (!fs.is_open()){
    perror("GPIO: write failed to open file ");
    exit(-1);
  }

  fs << gpionr;
  fs.close();  
}


uint16_t ADS1015::ReadI2CReg(int regnr)
{
  int n;
  char buf[10];
  
  buf[0] = regnr;

  /* write reg number */
  n = write(i2c_fd, buf, 1);
  if (n == -1) {
    perror("write i2c_fd");
    return 1;
  }

  /* Now read 2 bytes from that address: msb and lsb */
  n = read(i2c_fd, buf, 2);
  if (n == -1) {
    perror("read i2c_fd");
    return 1;
  }

  return (buf[0] << 8) + buf[1];

}


uint16_t ADS1015::ReadConvReg() { return this->ReadI2CReg(0); };
uint16_t ADS1015::ReadConfigReg() { return this->ReadI2CReg(1); };
uint16_t ADS1015::ReadLoThreshReg()   { return this->ReadI2CReg(2); };
uint16_t ADS1015::ReadHiThreshReg()   { return this->ReadI2CReg(3); };


uint16_t ADS1015::WriteI2CReg(int regnr, int value)
{

  int n;
  char buf[10];

  buf[0] = regnr;
  buf[1] = value >> 8;
  buf[2] = value & 0xFF;
  
  n = write(i2c_fd, buf, 3);
  if (n == -1) {
    perror("write");
    exit(-1);
  }
  
  return 0;
}

void ADS1015::WriteConfigReg(int value)   { this->WriteI2CReg(1,value); };
void ADS1015::WriteLoThreshReg(int value) { this->WriteI2CReg(2,value); };
void ADS1015::WriteHiThreshReg(int value) { this->WriteI2CReg(3,value); };


void ADS1015::Reset()
{
  //this->WriteI2CReg(1,0x0006);

  this->WriteConfigReg(0x8583);
  this->WriteLoThreshReg(0x8000);
  this->WriteHiThreshReg(0x7fff);

}


void ADS1015::myblink() {
  for (int i {0};i<2;i++) {
   RDY->setValue(HIGH);
   usleep(100000);
   RDY->setValue(LOW);
   usleep(100000);
  }
}

void ADS1015::EnableRdy()
{
  this->WriteLoThreshReg(0x0000);
  this->WriteHiThreshReg(0xffff);

}


