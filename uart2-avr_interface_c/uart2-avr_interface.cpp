#include<iostream>
#include<fstream>
#include<cstring>
#include<iomanip>

#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<termios.h>
#include<string.h>
#include<stdlib.h>
#include<sys/ioctl.h>
#include<unistd.h> //for usleep
#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include <getopt.h>

#include"GPIO.h"

using namespace exploringBB;
using namespace std;

#define RST_PIN "17" //P9_23 green
#define TTY_DEVICE "/dev/ttyS2" //ttyS2
#define AVISTATUS  if (mySeq.verbose) cout << "AVI Status: " << pAVI->GetStatus() << endl;
#define AVILOG(logtext) if (mySeq.verbose) cout << logtext << endl;
#define MAX_LOOP 1000

// RST
// Header: P9
// Pin:    #23
// BB_Pinmux.ods:
// => P9_23 / Addr 0x044
//
// cat /sys/kernel/debug/pinctrl/44e10800.pinmux-pinctrl-single/pins | grep 44e10844
// => pin 17 (PIN17) 17:gpio-0-31 44e10844 00000027 pinctrl-single 
// i.e. PIN17
//
// cat /sys/kernel/debug/pinctrl/44e10800.pinmux-pinctrl-single/gpio-ranges | grep gpio-0-31
// => 12: gpio-0-31 GPIOS [12 - 27] PINS [12 - 27]
// i.e. GPIO17


// echo 17 > /sys/class/gpio/export
// echo out > /sys/class/gpio/gpio17/direction
// echo 1 > /sys/class/gpio/gpio17/value


void print_usage(const string prog)
{
  cout << "Usage:" << prog << " [-vhDr]\n" << endl;
  cout << "  -v --verbose" << endl;
  cout << "  -h --help     display usage" << endl;
  cout << "  -D --device   device to use (default " << TTY_DEVICE << ")" << endl;
  cout << "  -r --reset    apply reset pulse" << endl;
  cout << "  -l --lifesign request lifesign" << endl;
  cout << "  -c --cleanup  unexport RST pin" << endl;  
  exit(1);
}

class cResult {

public:
  cResult() {
    result=0;
    nriter=0.0;
  };
  uint8_t result;
  int nriter;
};


class AVRInterface {
private:
  GPIO *RST {nullptr};    // RST pin
  int client {0};         // client filedescriptor
  struct termios options;
  int STATUS {0};
  pthread_t thread;
  friend void* threadedRead(void *value);
  cResult* Write_Read(int);

public:
  AVRInterface(string);
  ~AVRInterface();
  int GetStatus() {return STATUS; } //>0 means OK
  void ApplyResetPulse();
  cResult* RequestLifeSign();
  cResult* ReadOscCal();
  void UnexportRST();

};



void* threadedRead(void *value) {
  uint8_t *x=new(uint8_t);
  *x=0;
  
  AVRInterface* p = static_cast<AVRInterface*>(value);

  int i=0;
  while (i++ < MAX_LOOP) {
    if (read(p->client,x,1) >0 ) break;
    usleep(100);
  }

  cResult *pResult = new (cResult);
  pResult->result=*x;
  pResult->nriter=i;
  
  return pResult;

}

AVRInterface::AVRInterface(string tty_device) {
  this->STATUS=1;
 
  RST=new GPIO(stoi(RST_PIN)); // rst pin
  
  if (RST) { 
    if ( !RST->IsExportedGPIO() ) {
      RST->exportGPIO();
      RST->setDirection(OUTPUT); }

  } else { this->STATUS=-1; }

  char* dev_name = new char[tty_device.length()+1];
  strcpy(dev_name, tty_device.c_str());
  
  if ((client = open(dev_name, O_RDWR | O_NOCTTY | O_NDELAY))<0){
    perror("UART: Failed to open the file.\n");
    this->STATUS=-2;
  }
  else
    { // man termios
      tcgetattr(client, &options);
      options.c_cflag = B9600 | CS8 | CREAD | CLOCAL;
      options.c_iflag = IGNPAR | ICRNL;
      tcflush(client, TCIFLUSH);
      fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);  // make reads non-blocking
      options.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
      options.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
      options.c_oflag &= ~OPOST;
      options.c_cc[VMIN] = 1;
      options.c_cc[VTIME] = 1;
      tcsetattr(client, TCSANOW, &options);
      usleep(100);
    }
}

AVRInterface::~AVRInterface() {
  if (RST) delete RST;
  if (client) close(client);
}

void AVRInterface::ApplyResetPulse() {
  this->STATUS=1;
 
  if (RST) {
    RST->setValue(LOW);
    usleep(100000); // 100ms rst pulse duratin
    RST->setValue(HIGH);
    usleep(100000); // 100ms waiting time for "hi"
    tcflush(client, TCIFLUSH); // flush "hi"
  } else { this->STATUS=-1; }

}



cResult* AVRInterface::Write_Read(int writeval) {
  this->STATUS=1;
 
  if (pthread_create(&this->thread, NULL, &threadedRead,  static_cast<void*>(this) )) {
    perror("GPIO: Failed to create the read thread");
    STATUS=-1;
  }

  //int writeval=1; 
  write(this->client,&writeval,1);
  usleep(100); 
  
  void *v;
  pthread_join(this->thread, &v);
  cResult *r=(cResult *)v;
  return r;
}

cResult* AVRInterface::RequestLifeSign() {
  return this->Write_Read(1);
}

cResult* AVRInterface::ReadOscCal() {
  return this->Write_Read(251);
}

void AVRInterface::UnexportRST() {
  this->STATUS=1;
  if (RST) {
    if (RST->IsExportedGPIO())
	      RST->unexportGPIO(); else { this->STATUS=-1; }
  }
}



void avr_readoscal(const int &client)
{
  int writeval=251; 
  uint8_t readval=0;   

  write(client,&writeval,1);
  if(read(client,&readval,1)>0) { // read performed w/o error
    printf("RDOSCAL Reg write=%d read=%d\n",writeval,readval);
  }
  usleep(10000); //5m
}

void avr_writeoscal(const int &client, int value)
{
  int writeval=254; 

  write(client,&writeval,1);
  usleep(100); // 100us
  write(client,&value,1);
  usleep(10000); //5m
}

void avr_readbytelength(const int &client)
{
  int writeval=250;
  uint8_t value=0;
  uint8_t readval=0;   

  write(client,&writeval,1);
  usleep(100); // 100us
  write(client,&value,1);
  usleep(100); // 100us
  if(read(client,&readval,1)>0) { // read performed w/o error
    printf("RDBYTELENGTH Reg write=%d read=%d (target=225)\n",writeval,readval);
  }
  usleep(10000); //5m
}

void avr_readoscaleeprom(const int &client)
{
  int writeval=252; 
  uint8_t readval=0;   

  write(client,&writeval,1); 
  if(read(client,&readval,1)>0) { // read performed w/o error
    printf("RDOSCALEEPROM Reg write=%d read=%d\n",writeval,readval);
  }
  usleep(10000); //5m
}

void avr_writeoscaleeprom(const int &client, int value)
{
  int writeval=253; 

  write(client,&writeval,1);
  usleep(100); // 100us
  write(client,&value,1);
  usleep(10000); //5m
}


void avr_setddrb(const int &client, int value)
{
  int writeval=17; 

  write(client,&writeval,1);
  usleep(100); // 100us
  write(client,&value,1);
  usleep(10000); //5m
}


void avr_setportb(const int &client, int value)
{
  int writeval=16; 

  write(client,&writeval,1);
  usleep(100); // 100us
  write(client,&value,1);
  usleep(10000); //5m
}


int main(int argc, char *argv[])
{
  
  struct CommandSeq {
    bool verbose;
    string tty_device;
    bool reset;
    bool lifesign;
    bool cleanup;
  };
 
  struct CommandSeq mySeq = {
    .verbose=false,
    .tty_device=TTY_DEVICE,
    .reset=false,
    .lifesign=false,
    //...
    .cleanup=false,
  };
    
  while (1) {
    static const struct option lopts[] = {
      { "verbose", 0, 0, 'v' },
      { "help",    0, 0, 'h' },
      { "device",  1, 0, 'D' }, // 1,0 = 1 optarg
      { "reset",   0, 0, 'r' }, // 0,0 = 0 optarg
      { "lifesign",0, 0, 'l' },
      { "cleanup", 0, 0, 'c' },      
      { NULL, 0, 0, 0 },
    };
    int c;

    c = getopt_long(argc, argv, "vhrD:lc", lopts, NULL);

    if (c == -1)
      break;

    switch (c) {
    case 'v':
      cout << "prepare option v" << endl;
      mySeq.verbose=true;
      break;
      
    case 'h':
      cout << "proceed option h" << endl;
      print_usage(argv[0]);
      break;
      
    case 'D':
      cout << "prepare option D, optarg = " << optarg << endl;
      mySeq.tty_device=optarg;
      break;
      
    case 'r':
      cout << "prepare option r" << endl;
      mySeq.reset=true;
      break;
      
    case 'l':
      cout << "prepare option l" << endl;
      mySeq.lifesign=true;
      break;
      
    case 'c':
      cout << "prepare option c" << endl;
      mySeq.cleanup=true;
      break;
      
    default:
      cout << "option unrecognized: " << c << endl;
      print_usage(argv[0]);
      break;
    }
  }

  AVILOG("Initialize Object...");  
  AVRInterface *pAVI = new AVRInterface(mySeq.tty_device);
  AVISTATUS; AVILOG("done\n");

  cResult* result;
  
  if (mySeq.reset) {
    AVILOG("Apply Reset Pulse..."); pAVI->ApplyResetPulse(); AVISTATUS; AVILOG("done\n");
  }

  if (mySeq.lifesign) {
    AVILOG("Request Lifesign...");
    result=pAVI->RequestLifeSign();
    AVISTATUS;
    cout << "Response="<< (int)(result->result) << " Nr of interations=" << result->nriter << endl;
    delete result;
    AVILOG("done\n");
  }
  
  if (mySeq.cleanup) {
    AVILOG("Unexport RST"); pAVI->UnexportRST(); AVISTATUS; AVILOG("done\n");
  }

  AVILOG("Destruct Object...");  
  delete(pAVI);
  cout << "done." << endl;
  
  return 0;



    // new device procedure
    // need to write a reasonable default value into the osccal register inside the hex code!
    // osc adjust procedure:
    // flash interface.hex
    // microcom -s 9600 /dev/ttyS2
    // expect "Hi"
    // 1. observe AVR TX out via LogicAnalyzer
    // 2. calc/guess proper osccal value X
    // 3. @Interface.asm: enable osccal load at prog start
    //    ldi	A,X
    //	out	osccal,A
    // 4. flash avr
    // 5a. call CAL routine (writes optimum X to EEPROM)
    // 5b. write X manually to EEPROM

  /*
  int rep=3;
  //printf("WROSCAL\n"); for (int i=0; i<rep; i++) avr_writeoscal(client,74); //74
  printf("RDOSCAL\n");  for (int i=0; i<rep; i++) avr_readoscal(client);
  printf("LENGTH\n");  for (int i=0; i<rep; i++) avr_readbytelength(client); // target 225
  //printf("WROSCALEEPROM\n");  for (int i=0; i<rep; i++) avr_writeoscaleeprom(client,73); //osc must be trimmed
  printf("RDOSCALEEPROM\n");  for (int i=0; i<rep; i++) avr_readoscaleeprom(client);

  avr_setddrb(client, 255);

  for (rep=0; rep<10; rep++) {
    avr_setportb(client, 255);
    usleep(100000);
    avr_setportb(client, 0);
    usleep(100000); 
  }

  /
    usleep(100);
    ioctl(client, TIOCSBRK);
    usleep(5000);
    ioctl(client, TIOCCBRK);
    usleep(100);
  /
  close(client);
  return 0;
  */
}
