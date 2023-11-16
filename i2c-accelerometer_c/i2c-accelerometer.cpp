#include<iostream>
#include<ads1015.h>
using namespace std;


int main(){
  ADS1015* ADC = new ADS1015;
  ADC->myblink();
  
  //  ADC->Reset();
  //  ADC->EnableRdy();  
  
  cout << "Conv Reg=" << std::hex << ADC->ReadConvReg() << std::dec << endl;
  cout << "Conf Reg=" << std::hex << ADC->ReadConfigReg() << std::dec << endl;
  cout << "Lo   Reg=" << std::hex << ADC->ReadLoThreshReg() << std::dec << endl;
  cout << "Hi   Reg=" << std::hex << ADC->ReadHiThreshReg() << std::dec << endl;

  ADC->WriteConfigReg(0x1234);
  cout << "Conf Reg=" << std::hex << ADC->ReadConfigReg() << std::dec << endl;

  delete (ADC);
									 
  return 0;
}

