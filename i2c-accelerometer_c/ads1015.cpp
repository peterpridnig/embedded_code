#include<iostream>
#include<unistd.h> //for usleep
#include"GPIO.h"
#include<fcntl.h>
#include<stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

using namespace exploringBB;
using namespace std;

#define I2C_DEVICE "/dev/i2c-2"
#define I2C_ADDRESS 0x48

enum AnalogInput {AIN0, AIN1, AIN2, AIN3};

/*
struct sData {
  char msb;
  char lsb;
};

union uData {
  int          msblsb;
  struct sData data;
};
*/

class ADS1015 {
private:
  int ConfigRegData =0x0;
  int HiRegData = 0x0;
  int LoThreshRegData = 0x0;
  
  GPIO *RDY;
  void exportGPIO(string gpionr);
  void unexportGPIO(string gpionr);
  int i2c_fd; // i2c filedescriptor    
 
  uint16_t ReadI2CReg(int regnr);  
  uint16_t WriteI2CReg(int regnr, int value);
  
public:
  ADS1015();
  uint16_t ReadConvReg();
  uint16_t ReadConfigReg();
  uint16_t ReadLoThreshReg();
  uint16_t ReadHiThreshReg();

  void WriteConfigReg(int value);
  void WriteLoThreshReg(int value);
  void WriteHiThreshReg(int value);
  
  void Reset(); 
  void myblink();
  ~ADS1015();
};


ADS1015::ADS1015() {
  cout << "Init ADC" << endl;
  
  this->exportGPIO("122");
  RDY=new GPIO(122); //red
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
  
  
}


void ADS1015::exportGPIO(string gpionr){
  ofstream fs;
  fs.open("/sys/class/gpio/export");

  if (!fs.is_open()){
    perror("GPIO: write failed to open file ");
    exit(-1);
  }

  fs << gpionr;
  fs.close();  
}


void ADS1015::unexportGPIO(string gpionr){
  ofstream fs;
  fs.open("/sys/class/gpio/unexport");

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

ADS1015::~ADS1015() {
  delete RDY;
  this->unexportGPIO("122");
  close(i2c_fd);
  cout << "Exit ADC" << endl;
}

int main(){
  ADS1015 ADC;
  ADC.myblink();
  
  ADC.Reset();
  
  cout << "Conv Reg=" << std::hex << ADC.ReadConvReg() << std::dec << endl;
  cout << "Conf Reg=" << std::hex << ADC.ReadConfigReg() << std::dec << endl;
  cout << "Lo   Reg=" << std::hex << ADC.ReadLoThreshReg() << std::dec << endl;
  cout << "Hi   Reg=" << std::hex << ADC.ReadHiThreshReg() << std::dec << endl;

  ADC.WriteConfigReg(0x1234);
  cout << "Conf Reg=" << std::hex << ADC.ReadConfigReg() << std::dec << endl;
  
  return 0;
}

