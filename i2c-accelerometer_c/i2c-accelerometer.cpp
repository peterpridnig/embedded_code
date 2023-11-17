#include<iostream>
#include<ads1015.h>
#include<unistd.h> //for usleep

using namespace std;


int main(){
  ADS1015* ADC = new ADS1015(true);
  
  cout << "Conv Reg=" << std::hex << ADC->ReadConvReg() << std::dec << endl;
  cout << "Conf Reg=" << std::hex << ADC->ReadConfigReg() << std::dec << endl;
  cout << "Lo   Reg=" << std::hex << ADC->ReadLoThreshReg() << std::dec << endl;
  cout << "Hi   Reg=" << std::hex << ADC->ReadHiThreshReg() << std::dec << endl;

  /*
  // manual conversion
  for (int i {0};i<10;i++){
    ADC->TriggerOneShotConversion(1); //1=x
    usleep(1000); //wait for conversion to finish
    cout << "Conv Result=" << std::hex << ADC->ReadConversionResult() << std::dec << endl;
  }

  // auto conversion with fix wait time  
  for (int i {0};i<10;i++)
   cout << "Conv Result=" << std::hex << ADC->ConvertWait(1) << std::dec << endl;
  */

  // auto conversion with rdy signal    
  for (int i {0};i<100;i++){
    cout << "AIN0=z=" << ADC->ConvertPoll(AIN0) << " AIN1=y=" << ADC->ConvertPoll(AIN1) << " AIN2=x=" << ADC->ConvertPoll(AIN2) << endl;
  }
  
  delete (ADC);
									 
  return 0;
}

