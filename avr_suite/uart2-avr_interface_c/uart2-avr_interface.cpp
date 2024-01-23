/* **********************
 * AVR Interface
 * pridnig, 26.12.2023
 * **********************

 // TRIM new device osccal procedure
 // need to write a reasonable default value into the osccal register inside the hex code!
 // osc adjust procedure:

 // A)  
 // 1. calc/guess proper osccal value X
 // 2. @Interface.asm: enable osccal load at prog start
 // skip OsccalKorr
 //    ldi	A,X
 //    out	osccal,A
 // 3. flash interface.hex (sh flash_avr.sh Interface.hex)
 // 4. observe AVR TX out via LogicAnalyzer
 //    aim towards pulsewidth=105us
 // 5. microcom -s 9600 /dev/ttyS2
 //    press reset and expect "Hi"
 // Goto 1 UNTIL PASS
 
 // B) not used; re-flash overvrites EEPROM!
 // 1. optimize speed
 // 2. write X manually to EEPROM
 // 3. @Interface.asm: enable OscKorr
 //    rjmp	OscKorr		;skip OsccalSet
 // 4. flash interface.hex
 */

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

enum eStatus {
  IDLE,
  RUNNING,
  PASS,
  FAIL };


class cResult {

public:
  cResult() {
    status=IDLE;
    result=0;
    result2=0;
    nriter=-1;
    nriter2=-1;
  };
  enum eStatus status;
  uint8_t result;
  uint8_t result2;
  int nriter;
  int nriter2;
};


class AVRInterface {
private:
  bool verbose {true};
  GPIO *RST {nullptr};    // RST pin
  int client {0};         // client filedescriptor
  struct termios options;
  int STATUS {0};
  pthread_t thread;
  friend void* threadedRead(void *value);
  friend void* threadedRead2x(void *value);
  void Interact(uint8_t, uint8_t*); //write cmd, read result
  void Interact(uint8_t, uint8_t, uint8_t*); //write cmd, write arg, read result
  void Interact(uint8_t, uint8_t*, uint8_t*); //write cmd, read result, read result2
  void Interact(uint8_t, uint8_t); //write cmd, write val
  

public:
  AVRInterface(string,bool);
  ~AVRInterface();
  int GetStatus() {return STATUS; } //>0 means OK
  void ApplyResetPulse();
  void RequestLifeSign();
  void FuseRead();
  void OscCalRead();
  void OscCalWrite(uint8_t);
  void OscCalReadEEProm();
  void OscCalWriteEEProm(uint8_t);
  void ByteLengthRead();
  void SetDdrB(uint8_t);
  void SetPortB(uint8_t);
  void ReadPortB();
  void UnexportRST();

};


AVRInterface::AVRInterface(string tty_device, bool verbose=false) {
  this->STATUS=1;
  this->verbose=verbose;

  // reset pin
  RST=new GPIO(stoi(RST_PIN));
  if (RST) { 
    if ( !RST->IsExportedGPIO() ) {
      RST->exportGPIO();
      RST->setDirection(OUTPUT); }

  } else { this->STATUS=-1; }

  // tty device
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

  if (RST) {
    if (this->verbose) cout << "RST LOW" << endl; 
    RST->setValue(LOW);
    usleep(100000); // 100ms rst pulse duration
    if (this->verbose) cout << "RST HIGH" << endl;     
    RST->setValue(HIGH);
    usleep(100000); // 100ms waiting time for "hi"
    tcflush(client, TCIFLUSH); // flush "hi"
    cout << "Reset Pulse applied" << endl;
  } else { this->STATUS=-3; }

}


void* threadedRead(void *myobj) {
  
  AVRInterface* p = static_cast<AVRInterface*>(myobj);
  cResult *pResult = new (cResult);
  if (pResult) {
    pResult->status=RUNNING;
			    
    int i=0;
    while (i++ < MAX_LOOP) {
      if (read(p->client,&pResult->result,1) >0 ) break;
      usleep(100);
    }

    if (i<MAX_LOOP) {
      pResult->status=PASS;
      pResult->nriter=i;
    } else{
      pResult->status=FAIL;
      pResult->result=0;
      pResult->nriter=i;
    }
    return pResult;
    
  } else return nullptr;
  
}


void* threadedRead2x(void *myobj) {
  
  AVRInterface* p = static_cast<AVRInterface*>(myobj);
  cResult *pResult = new (cResult);
  if (pResult) {
    pResult->status=RUNNING;
			    
    int i=0;
    while (i++ < MAX_LOOP) {
      if (read(p->client,&pResult->result,1) >0 ) break;
      usleep(100);
    }

    if (i<MAX_LOOP) {
      pResult->status=PASS;
      pResult->nriter=i;
    } else{
      pResult->status=FAIL;
      pResult->result=0;
      pResult->nriter=i;
    }

    i=0;

    while (i++ < MAX_LOOP) {
      if (read(p->client,&pResult->result2,1) >0 ) break;
      usleep(100);
    }

    if (i<MAX_LOOP) {
      pResult->status=PASS;
      pResult->nriter2=i;
    } else{
      pResult->status=FAIL;
      pResult->result2=0;
      pResult->nriter2=i;
    }
    
    return pResult;
    
  } else return nullptr;
  
}


void AVRInterface::Interact(uint8_t cmd, uint8_t* result) { //write cmd; read result
  *result=0;
 
  if (pthread_create(&this->thread, NULL, &threadedRead,  static_cast<void*>(this) )) {
    perror("GPIO: Failed to create the read thread");
    STATUS=-1;
  }

  if (write(this->client,&cmd,1)<0) {
    perror("UART: write fail.\n");
    this->STATUS=-8;
  }
  usleep(100); 
  
  void *v {nullptr};
  pthread_join(this->thread, &v);

  if (v) {
    cResult* myresult=(cResult *)v;
    *result=myresult->result;
    if (myresult->status==FAIL) this->STATUS=-4;
    if (this->verbose) {
      if (myresult->status==PASS) 
	cout << "Read is PASS (response " << (int)myresult->result << " after "<< myresult->nriter << " Iterations)" << endl;
      else 
	cout << "Read is FAIL (response " << (int)myresult->result << " after "<< myresult->nriter << " Iterations)" << endl;
    }
    delete myresult;
    
  }
  else {
    if (this->verbose) cout << "Read is FAIL (no result)" << endl;
    this->STATUS=-5;
  }

}

void AVRInterface::Interact(uint8_t cmd, uint8_t arg, uint8_t* result) { //write cmd; write arg; read result
  *result=0;
 
  if (pthread_create(&this->thread, NULL, &threadedRead,  static_cast<void*>(this) )) {
    perror("GPIO: Failed to create the read thread");
    STATUS=-1;
  }

  if (write(this->client,&cmd,1)<0) {
    perror("UART: write fail.\n");
    this->STATUS=-8;
  }
  usleep(100);

  if (write(this->client,&arg,1)<0) {
    perror("UART: write fail.\n");
    this->STATUS=-8;
  }
  usleep(100);
 
  
  void *v {nullptr};
  pthread_join(this->thread, &v);

  if (v) {
    cResult* myresult=(cResult *)v;
    *result=myresult->result;
    if (myresult->status==FAIL) this->STATUS=-4;
    if (this->verbose) {
      if (myresult->status==PASS) 
	cout << "Read is PASS (response " << (int)myresult->result << " after "<< myresult->nriter << " Iterations)" << endl;
      else 
	cout << "Read is FAIL (response " << (int)myresult->result << " after "<< myresult->nriter << " Iterations)" << endl;
    }
    delete myresult;
    
  }
  else {
    if (this->verbose) cout << "Read is FAIL (no result)" << endl;
    this->STATUS=-5;
  }

}


void AVRInterface::Interact(uint8_t cmd, uint8_t* result, uint8_t* result2) { //write cmd, read result, read result2
  *result=0;
  *result2=0;
 
  if (pthread_create(&this->thread, NULL, &threadedRead2x,  static_cast<void*>(this) )) {
    perror("GPIO: Failed to create the read thread");
    STATUS=-1;
  }

  if (write(this->client,&cmd,1)<0) {
    perror("UART: write fail.\n");
    this->STATUS=-8;
  }
  usleep(100);

  void *v {nullptr};
  pthread_join(this->thread, &v);

  if (v) {
    cResult* myresult=(cResult *)v;
    *result=myresult->result;
    *result2=myresult->result2;
    
    if (myresult->status==FAIL) this->STATUS=-4;
    if (this->verbose) {
      if (myresult->status==PASS) 
	cout << "Read is PASS (response " << (int)myresult->result << " after "<< myresult->nriter << " Iterations" << (int)myresult->result2 << " after "<< myresult->nriter2 << " Iterations)"<< endl;
      else 
	cout << "Read is FAIL (response " << (int)myresult->result << " after "<< myresult->nriter << " Iterations)" << (int)myresult->result2 << " after "<< myresult->nriter2 << " Iterations)"<< endl;
    }
    delete myresult;
    
  }
  else {
    if (this->verbose) cout << "Read is FAIL (no result)" << endl;
    this->STATUS=-5;
  }

}



void AVRInterface::Interact(uint8_t cmd, uint8_t arg) { //write cmd; write arg
   
  if (write(this->client,&cmd,1)<0) {
    perror("UART: write fail.\n");
    this->STATUS=-6;
  }
  usleep(100); 

  if (write(this->client,&arg,1)<0) {
    perror("UART: write fail.\n");
    this->STATUS=-7;
  }
  usleep(100); 

}


void AVRInterface::RequestLifeSign() {
  uint8_t result {0};

  cout << "Request LifeSign: ";
  
  this->Interact(1, &result);
  if (this->STATUS>0) 
    cout << "response=" << (int)result << endl;
  else
    cout << "FAIL" << endl;

}


void AVRInterface::OscCalRead() {
  uint8_t result {0};
  
  cout << "OscCal Read: ";

  this->Interact(251, &result);
  if (this->STATUS>0) 
    cout << "response=" << (int)result << endl;
  else
    cout << "FAIL" << endl;

}


void AVRInterface::OscCalWrite(uint8_t arg) {
  
  cout << "OscCal Write: ";

  this->Interact(254, arg);
  if (this->STATUS>0) 
    cout << "done" << endl;
  else
    cout << "FAIL" << endl;

}


void AVRInterface::OscCalReadEEProm() {
  uint8_t result {0};
  
  cout << "OscCal Read EEProm: ";

  this->Interact(252, &result);
  if (this->STATUS>0) 
    cout << "response=" << (int)result << endl;
  else
    cout << "FAIL" << endl;

}


void AVRInterface::OscCalWriteEEProm(uint8_t arg) {
  
  cout << "OscCal Write EEProm: ";

  this->Interact(253, arg);
  if (this->STATUS>0) 
    cout << "done" << endl;
  else
    cout << "FAIL" << endl;

}


void AVRInterface::ByteLengthRead() {
  uint8_t result {0};
  uint8_t empty {0};
  cout << "ByteLength Read: " << endl;

  this->Interact(250, empty, &result);
  if (this->STATUS>0) 
    cout << "response=" << (int)result << " target=225" << endl;
  else
    cout << "FAIL" << endl;

}

void AVRInterface::FuseRead() {
  uint8_t result {0};
  uint8_t result2 {0};
  
  cout << "Fuse Read: " << endl;
  
  stringstream ss;
  ss << std::hex;
    
  this->Interact(249, &result, &result2);
  if (this->STATUS>0) {
    ss << (int)result2 << "(hi) " << (int)result << "(lo)";
    cout << "response=" << ss.str() << endl;
  }
  else
    cout << "FAIL" << endl;

}

void AVRInterface::SetDdrB(uint8_t arg) {
  
  cout << "Set DDRB: ";

  this->Interact(17, arg);
  if (this->STATUS>0) 
    cout << "done" << endl;
  else
    cout << "FAIL" << endl;

}

void AVRInterface::SetPortB(uint8_t arg) {
  
  cout << "Set PortB: ";

  this->Interact(16, arg);
  if (this->STATUS>0) 
    cout << "done" << endl;
  else
    cout << "FAIL" << endl;

}

string Val2Dig(uint8_t digval, uint8_t weight)
{ string HI {"hi"};
  string LO {"lo"};
  if (digval & weight) return HI; else return LO;
  
}

void AVRInterface::ReadPortB() {
  uint8_t result {0};
  uint8_t empty {0};
  
  cout << "Read PortB: ";

  this->Interact(32, empty, &result);
  if (this->STATUS>0) {
    cout << "response=" << (int)result << endl;
    cout << "pb4 pb3 pb2 pb1 pb0" << endl;
    cout << Val2Dig(result,16) << "  " << Val2Dig(result,8) << "  " << "--" << "  " << "--" << "  " << Val2Dig(result,1) << endl;
  }
  else
    cout << "FAIL" << endl;

}


void AVRInterface::UnexportRST() {

  if (RST) {
    if (RST->IsExportedGPIO())
	      RST->unexportGPIO(); else { this->STATUS=-6; }
  }
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


void print_usage(const string prog)
{
  cout << "Usage:" << prog << " [-vhDrlwoepbc]"                      << endl << endl;
  cout << "  -v --verbose             be wordy "                     << endl;
  cout << "  -h --help                display usage"                 << endl;
  cout << "  -D --device [dev]        device to use (default "       << TTY_DEVICE << ")" << endl;
  cout << "  -r --reset               apply reset pulse"             << endl;
  cout << "  -l --lifesign            request lifesign"              << endl;
  cout << "  -f --fuseread            read fuses"                    << endl;  
  cout << "   OSCILLATOR"                                            << endl;
  cout << "  -w --osccalwrite [val]   write osccal value"            << endl;
  cout << "  -o --osccalread          read osccal value"             << endl;
  cout << "  -e --osccalwriteeeprom   write osccal value to eeprom"  << endl;
  cout << "  -p --osccalreadeeprom    read osccal value from eeprom" << endl;
  cout << "  -b --bytelengthread      read bytelength"               << endl;
  cout << "   PORT CONTROL"                                          << endl;
  cout << "  -d --setddrb [val]       set portb ddr (1=out/0=in)"    << endl;
  cout << "  -t --setportb [val]      set portb (1=hi/0=lo)"         << endl;
  cout << "  -a --readportb           read portb (1=hi/0=lo)"        << endl;
  cout << "                           -------------------"           << endl;
  cout << "                           pb4 pb3 pb2 pb1 pb0"           << endl;
  cout << "                           swi led rxd txd led"           << endl;
  cout << "                            1)  2) -na -na  3)"           << endl;
  cout << "                        1) pulldn 2) orange  3) red"      << endl;  
  cout << "                        ---------------------------"      << endl;  
  cout << "  -c --cleanup             unexport RST pin"              << endl;  
  exit(1);
}


int main(int argc, char *argv[])
{
  
  struct CommandSeq {
    bool verbose;
    string tty_device;
    bool reset;
    bool lifesign;
    bool fuseread;
    bool osccalwrite;
    uint16_t osccalarg;
    bool osccalread;
    bool osccalwriteeeprom;
    uint16_t osccaleepromarg;
    bool osccalreadeeprom;
    bool bytelengthread;
    bool setddrb;
    uint16_t setddrbarg;
    bool setportb;
    uint16_t setportbarg;
    bool readportb;
    bool cleanup;
  };
 
  struct CommandSeq mySeq = {
    .verbose=false,
    .tty_device=TTY_DEVICE,
    .reset=false,
    .lifesign=false,
    .fuseread=false,
    .osccalwrite=false,
    .osccalarg=0,
    .osccalread=false,
    .osccalwriteeeprom=false,
    .osccaleepromarg=0,
    .osccalreadeeprom=false,
    .bytelengthread=false,
    .setddrb=false,
    .setddrbarg=0,
    .setportb=false,
    .setportbarg=0,
    .readportb=false,
    //...
    .cleanup=false,
  };

  cout << "Parsing Options" << endl;
  
  while (1) {
    static const struct option lopts[] = {
      { "verbose",     0, 0, 'v' },
      { "help",        0, 0, 'h' },
      { "device",      1, 0, 'D' }, // 1,0 = 1 optarg
      { "reset",       0, 0, 'r' }, // 0,0 = 0 optarg
      { "lifesign",    0, 0, 'l' },
      { "fuseread",          0, 0, 'f' },
      { "osccalwrite", 1, 0, 'w' },      
      { "osccalread",  0, 0, 'o' },
      { "osccalwriteeeprom", 1, 0, 'e' },
      { "osccalreadeeprom",  0, 0, 'p' },
      { "bytelengthread",    0, 0, 'b' },
      { "setddrb",           1, 0, 'd' },
      { "setportb",          1, 0, 't' },
      { "readportb",         0, 0, 'a' },
      { "cleanup",           0, 0, 'c' },
      { NULL,                0, 0, 0 },
    };
    int c;
    
    c = getopt_long(argc, argv, "vhrD:lw:oe:pbfd:t:ac", lopts, NULL);

    if (c == -1)
      break;

    switch (c) {
    case 'v':
      cout << "option v" << endl;
      mySeq.verbose=true;
      break;
      
    case 'h':
      cout << "option h" << endl;
      print_usage(argv[0]);
      break;
      
    case 'D':
      cout << "option D, arg = " << optarg << endl;
      mySeq.tty_device=optarg;
      break;
      
    case 'r':
      cout << "option r" << endl;
      mySeq.reset=true;
      break;
      
    case 'l':
      cout << "option l" << endl;
      mySeq.lifesign=true;
      break;

    case 'f':
      cout << "option f" << endl;
      mySeq.fuseread=true;
      break;
      
    case 'w':
      cout << "option w, arg=" << optarg << endl;
      mySeq.osccalarg=atoi(optarg);
      mySeq.osccalwrite=true;
      break;
      
    case 'o':
      cout << "option o" << endl;
      mySeq.osccalread=true;
      break;

    case 'e':
      cout << "option e, arg=" << optarg << endl;
      mySeq.osccaleepromarg=atoi(optarg);
      mySeq.osccalwriteeeprom=true;
      break;
      
    case 'p':
      cout << "option p" << endl;
      mySeq.osccalreadeeprom=true;
      break;

    case 'b':
      cout << "option b" << endl;
      mySeq.bytelengthread=true;
      break;

    case 'd':
      cout << "option d" << endl;
      mySeq.setddrb=true;
      mySeq.setddrbarg=atoi(optarg);
      break;
      
    case 't':
      cout << "option e" << endl;
      mySeq.setportb=true;
      mySeq.setportbarg=atoi(optarg);
      break;
      
    case 'a':
      cout << "option a" << endl;
      mySeq.readportb=true;
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
  
  cout << "Done" << endl << endl;

  if (argc==1) print_usage(argv[0]);
  
  AVILOG("Initialize Object...");  
  AVRInterface *pAVI = new AVRInterface(mySeq.tty_device, mySeq.verbose);
  AVISTATUS; AVILOG("done\n");

  if (mySeq.reset) {
    AVILOG("Apply Reset Pulse..."); pAVI->ApplyResetPulse(); AVISTATUS; AVILOG("done\n");
  }

  if (mySeq.lifesign) {
    AVILOG("Request Lifesign..."); pAVI->RequestLifeSign(); AVISTATUS; AVILOG("done\n");
  }

  if (mySeq.fuseread) {
    AVILOG("FuseRead..."); pAVI->FuseRead(); AVISTATUS; AVILOG("done\n");
  }
  
  if (mySeq.osccalwrite) {
    AVILOG("OscCalWrite..."); pAVI->OscCalWrite(mySeq.osccalarg); AVISTATUS; AVILOG("done\n");
  }
  
  if (mySeq.osccalread) {
    AVILOG("OscCalRead..."); pAVI->OscCalRead(); AVISTATUS; AVILOG("done\n");
  }

  if (mySeq.osccalwriteeeprom) {
    AVILOG("OscCalWrite EEPROM..."); pAVI->OscCalWriteEEProm(mySeq.osccaleepromarg); AVISTATUS; AVILOG("done\n");
  }
  
  if (mySeq.osccalreadeeprom) {
    AVILOG("OscCalRead EEPROM..."); pAVI->OscCalReadEEProm(); AVISTATUS; AVILOG("done\n");
  }

  if (mySeq.bytelengthread) {
    AVILOG("ByteLengthRead..."); pAVI->ByteLengthRead(); AVISTATUS; AVILOG("done\n");
  }
  
  if (mySeq.setddrb) {
    AVILOG("SetDdrB..."); pAVI->SetDdrB(mySeq.setddrbarg); AVISTATUS; AVILOG("done\n");
  }

  if (mySeq.setportb) {
    AVILOG("SetPortB..."); pAVI->SetPortB(mySeq.setportbarg); AVISTATUS; AVILOG("done\n");
  }

  if (mySeq.readportb) {
    AVILOG("ReadPortB..."); pAVI->ReadPortB(); AVISTATUS; AVILOG("done\n");
  }
 
  if (mySeq.cleanup) {
    AVILOG("Unexport RST"); pAVI->UnexportRST(); AVISTATUS; AVILOG("done\n");
  }

  AVILOG("Destruct Object...");  
  delete(pAVI);
  cout << endl << "Exit." << endl;
  
  return 0;

  
  /*

    usleep(100);
    ioctl(client, TIOCSBRK);
    usleep(5000);
    ioctl(client, TIOCCBRK);
    usleep(100);

  */
}
