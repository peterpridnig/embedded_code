#include <ads1015.h>
#include"GPIO.h"

#include<iostream>
#include<cstring>
#include<fcntl.h>
#include<stdlib.h>
#include<linux/i2c-dev.h>
#include<sys/ioctl.h>
#include<unistd.h> //for usleep
#include<sys/epoll.h>


using namespace exploringBB;
using namespace std;

ADS1015::ADS1015(bool doblink) {
  cout << "Init ADC" << endl;

  this->blink=doblink;
   
  this->exportGPIO(OUT_PIN);
  OUT=new GPIO(stoi(OUT_PIN)); //red led
  OUT->setDirection(OUTPUT);
  
  i2c_fd = open(I2C_DEVICE, O_RDWR);
  if (i2c_fd < 0) {
    perror(I2C_DEVICE);
    exit(-1);
   }
  
  if (ioctl(i2c_fd, I2C_SLAVE, I2C_ADDRESS) == -1) {
    perror("ioctl I2C_SLAVE");
    exit(-1);
  }

  this->Reset(); //write reset config

  this->exportGPIO(RDY_PIN);
  RDY=new GPIO(stoi(RDY_PIN));
  RDY->setDirection(INPUT);
  RDY->setEdgeType(FALLING); //wait for falling edge
  this->EnableRdy(); //enable rdy signal and rdy input gpio
  
}


ADS1015::~ADS1015() {
  delete OUT;
  this->unexportGPIO(OUT_PIN);
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
  for (int i {0};i<1;i++) {
   OUT->setValue(HIGH);
   usleep(50000);
   OUT->setValue(LOW);
   usleep(50000);
  }
}


void ADS1015::EnableRdy()
{
  this->WriteLoThreshReg(0x0000);
  this->WriteHiThreshReg(0xffff);
  
  char value[4];
	int ret;
	int n;

	this->ep = epoll_create(1);
	if (ep == -1) {
		perror("Can't create epoll");
		exit(1);
	}

	char VALUE_PATH[29] {};
	strcat(VALUE_PATH,GPIO_PATH);
	strcat(VALUE_PATH,"gpio");
	strcat(VALUE_PATH,RDY_PIN);
	strcat(VALUE_PATH,"/value");
	
	//cout << "value path: " << VALUE_PATH << endl;
		
	//f = open("/sys/class/gpio/gpio15/value", O_RDONLY | O_NONBLOCK);
        ep_f = open(VALUE_PATH, O_RDONLY | O_NONBLOCK);	  
	if (ep_f == -1) {
		perror("Can't open rdy-gpio");
		exit(1);
	}

	
	n = read(ep_f, &value, sizeof(value));
	if (n > 0) {
	  //printf("Initial value value=%c\n", value[0]);
		lseek(ep_f, 0, SEEK_SET);
	}
	
	
	ev.events = EPOLLPRI;
	ev.data.fd = ep_f;
	ret = epoll_ctl(ep, EPOLL_CTL_ADD, ep_f, &ev);
	if (ret == -1) {
		perror("Can't register target file descriptor with epoll");
		exit(1);
	}

}


void ADS1015::TriggerOneShotConversion(int AIN)
{
  //config reg MSBs
  // 15: write/1= start a single conversion
  // OS  read /0= device currently performing conversion
  //          /1= not performing conversion
  // 14:12 : 0x100 AIN0
  // MUX     0x101 AIN1
  //         0x110 AIN2
  //         0x111 AIN3
  // 11:9 :  000 FSR=6.1 V
  // PGA     001 FSR=4.096
  //         010     2.048
  //         011     1.024
  //         100     0.512
  //         101     0.256
  //         110     0.256
  //         111     0.256
  // 8 :     0: cont-conv mode

  //config reg LSBs
  // MODE    1: single-shot
  // 7:5 :   100: 1600 SPS (default)
  // DR      101: 2400 SPS
  // 4 :     0 traditional
  // COMPMD  1 window
  // 3 :     0 active lo (default)
  // COMPPOL 1 active hi
  // 2 :     0 (default)
  // COMPLAT
  // 1:0 :   11 (default
  // QUE
  
  // a xy    b 
  // 1100 0101 1000 0000
  // C    1    8    0
  // a= start conv
  // b=single conv
  // xy=AIN0,1,2

  // FS= 000 = 6.1V
  // Vin =     1.7V Dout = 1.7/6.1*4096/2=1138/2=569
  // Vin =     1.3V Dout = 1.3/6.1*4096/2       =436

  // FS= 010 = 2.048V <===
  // Vin =     1.7V Dout = 1.7/2.048*4096/2=1138/2=1700
  // Vin =     1.3V Dout = 1.3/2.048*4096/2       =1300

  // AIN3=00     AIN0=01     AIN1=10     AIN2=11
  /*
  if (AIN==0) {AIN=1;}
  else if (AIN==1) {AIN=2;}
  else if (AIN==2) {AIN=3;}
  else if (AIN==3) {AIN=0;}
  */
  //AIN = (AIN+1) & 0x3; //attention, next sample is from the previous conversion!
  
  this->WriteConfigReg(0xC580 | ( (AIN & 3) << 12));

}

int ADS1015::ReadConversionResult() { return this->ReadConvReg() >> 4; }


int ADS1015::ConvertWait(int AIN) {

    this->TriggerOneShotConversion(AIN); //1=x
    usleep(1000); //wait for conversion to finish
    int result= this->ReadConversionResult();
    if (this->blink) { this->myblink(); }
    return result;
}


int ADS1015::ConvertPoll(int AIN) {
  
 char value[4];
 int result;

 OUT->setValue(LOW); //reset LED
 int n=read(ep_f, &value, sizeof(value)); //prepare poll
 if (n > 0) { lseek(ep_f, 0, SEEK_SET); }
 
 this->TriggerOneShotConversion(AIN);
 
 int ret = epoll_wait(ep, &events, 1, -1);
 if (ret > 0) {
   result=ReadConversionResult();
   if (this->blink) { this->myblink(); }
   lseek(ep_f, 0, SEEK_SET);
 } 

 return result;
}

