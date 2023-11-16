#include"GPIO.h"

using namespace exploringBB;
using namespace std;

#define I2C_DEVICE "/dev/i2c-2"
#define I2C_ADDRESS 0x48
#define RDY_PIN "122"

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
  
  GPIO *RDY;
  void exportGPIO(string gpionr);
  void unexportGPIO(string gpionr);
  int i2c_fd; // i2c filedescriptor    
 
  uint16_t ReadI2CReg(int regnr);  
  uint16_t WriteI2CReg(int regnr, int value);
  
public:
  ADS1015();
  ~ADS1015();
  
  uint16_t ReadConvReg();
  uint16_t ReadConfigReg();
  uint16_t ReadLoThreshReg();
  uint16_t ReadHiThreshReg();

  void WriteConfigReg(int value);
  void WriteLoThreshReg(int value);
  void WriteHiThreshReg(int value);
  
  void Reset();
  void EnableRdy();
  
  void myblink();

};


