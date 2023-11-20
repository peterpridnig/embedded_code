#include"GPIO.h"
#include<sys/epoll.h>

using namespace exploringBB;
using namespace std;

#define I2C_DEVICE "/dev/i2c-2"
#define I2C_ADDRESS 0x48
//#define OUT_PIN "122" //P8_14 red
#define OUT_PIN "14" //P8_16 green
#define RDY_PIN "15" //P8_15

/*
RDY input
Header: P8
Pin:    #15
BB_Pinmux.ods:
=> P8_15 = GPIO1_51 / Addr 0x03C
#
cat /sys/kernel/debug/pinctrl/44e10800.pinmux-pinctrl-single/pins | grep 44e1083c
=> pin 15 (PIN15) 15:gpio-0-31 44e1083c 00000027 pinctrl-single
i.e. PIN15
#
cat /sys/kernel/debug/pinctrl/44e10800.pinmux-pinctrl-single/gpio-ranges | grep gpio-0-31
=> 12: gpio-0-31 GPIOS [12 - 27] PINS [12 - 27]
i.e. GPIO15
*/

enum AnalogInput {AIN0, AIN1, AIN2, AIN3};

/*
struct sData {
  char bit15=1;
  char bit1412=0x100;

};

union uData {
  int          msblsb;
  struct sData data;
};
*/

class ADS1015 {
private:
  

  void exportGPIO(string gpionr);
  void unexportGPIO(string gpionr);

  bool blink;
  GPIO *OUT;
  
  int i2c_fd; // i2c filedescriptor    
  uint16_t ReadI2CReg(int regnr);  
  uint16_t WriteI2CReg(int regnr, int value);

  GPIO *RDY;
  void EnableRdy();
  int ep;
  int ep_f;
  struct epoll_event ev, events;
  
  
public:
  ADS1015(bool doblink);
  ~ADS1015();
  
  uint16_t ReadConvReg();
  uint16_t ReadConfigReg();
  uint16_t ReadLoThreshReg();
  uint16_t ReadHiThreshReg();

  void WriteConfigReg(int value);
  void WriteLoThreshReg(int value);
  void WriteHiThreshReg(int value);
  
  void Reset();
  
  void TriggerOneShotConversion(int AIN);
  int  ReadConversionResult();
  int  ConvertWait(int AIN);
  int  ConvertPoll(int AIN);
  
  void myblink();

};


