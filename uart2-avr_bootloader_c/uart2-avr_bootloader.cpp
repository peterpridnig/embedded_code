/* **********************
 * AVR Bootloader
 * pridnig, 27.12.2023
 * **********************
 */

#include<iostream>
#include<fstream>
#include<cstring>
#include<iomanip>
#include<vector>

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
#include"HEX.h"

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


class AVRBootloader {
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
  void Interact(uint8_t, uint8_t, uint8_t*, uint8_t*); //write hi, write lo, read result, read result2
  void Interact(uint8_t*, uint8_t*); //read result, read result2  
  void Interact(uint8_t, uint8_t); //write cmd, write val
  void Interact(uint8_t); //write cmd
  void Interact(uint8_t*); //RXD=0, read result

  
public:
  AVRBootloader(string,bool);
  ~AVRBootloader();
  int GetStatus() {return STATUS; } //>0 means OK
  void ApplyResetPulse();
  void EnterBootloader();
  void WriteFlash(string);
  void ReadFlash();
  void ProgStart();
  void UnexportRST();

};


AVRBootloader::AVRBootloader(string tty_device, bool verbose=false) {
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


AVRBootloader::~AVRBootloader() {
  if (RST) delete RST;
  if (client) close(client);
}

void AVRBootloader::ApplyResetPulse() {

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
  
  AVRBootloader* p = static_cast<AVRBootloader*>(myobj);
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
  
  AVRBootloader* p = static_cast<AVRBootloader*>(myobj);
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


void AVRBootloader::Interact(uint8_t cmd, uint8_t* result) { //write cmd; read result
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

void AVRBootloader::Interact(uint8_t cmd, uint8_t arg, uint8_t* result) { //write cmd; write arg; read result
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

void AVRBootloader::Interact(uint8_t hi, uint8_t lo, uint8_t* result, uint8_t* result2) { //write hi, write lo, read result, read result2
  *result=0;
  *result2=0;
 
  if (pthread_create(&this->thread, NULL, &threadedRead2x,  static_cast<void*>(this) )) {
    perror("GPIO: Failed to create the read thread");
    STATUS=-1;
  }

  if (write(this->client,&hi,1)<0) {
    perror("UART: write fail.\n");
    this->STATUS=-8;
  }
  usleep(100);

  if (write(this->client,&lo,1)<0) {
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


void AVRBootloader::Interact(uint8_t* result, uint8_t* result2) { //read result, read result2
  *result=0;
  *result2=0;
 
  if (pthread_create(&this->thread, NULL, &threadedRead2x,  static_cast<void*>(this) )) {
    perror("GPIO: Failed to create the read thread");
    STATUS=-1;
  }

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


void AVRBootloader::Interact(uint8_t cmd, uint8_t arg) { //write cmd; write arg
   
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

void AVRBootloader::Interact(uint8_t cmd) { //write cmd
   
  if (write(this->client,&cmd,1)<0) {
    perror("UART: write fail.\n");
    this->STATUS=-6;
  }
  usleep(100); 

}



void AVRBootloader::Interact(uint8_t* result) { //TXD=HI; read result
  *result=0;
 
  if (pthread_create(&this->thread, NULL, &threadedRead,  static_cast<void*>(this) )) {
    perror("GPIO: Failed to create the read thread");
    STATUS=-1;
  }
  
  ioctl(client, TIOCCBRK);
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


void AVRBootloader::EnterBootloader() {
  uint8_t result {0};

  cout << "Enter Bootloader: ";
 
  if (RST) {
    if (this->verbose) cout << "RST LOW" << endl; 
    RST->setValue(LOW);
    usleep(100000); // 100ms

    if (this->verbose) cout << "TXD LOW" << endl; 
    ioctl(client, TIOCSBRK); //TXD=0
    usleep(100000); // 100ms
    
    if (this->verbose) cout << "RST HIGH" << endl;     
    RST->setValue(HIGH);
    usleep(100000); // 100ms
     
    if (this->verbose) cout << "TXD HIGH" << endl;
    
    this->Interact(&result);
    if (this->STATUS>0) 
      cout << "response=" << (int)result << endl;
    else
      cout << "FAIL" << endl;

    usleep(100000); // 100 ms
    
    tcflush(client, TCIFLUSH); // flush "hi"
    usleep(100000); // 100 ms
    
    if (result==105)     
      cout << "Bootloader entered successfully" << endl;
    else {
      this->STATUS=-5;
      cout << "Bootloader does not respond." << endl;
    }
    
  } else { this->STATUS=-3; }

}


void AVRBootloader::WriteFlash(string filename) {
  cout << "Read hexfile " << filename << endl;
  cHexFile hf(filename, this->verbose);
  hf.ReadHexFileContent();
  cout << "Done" << endl;

  hf.DisplayPages();
  //cout << "Highest Page#" << hf.Page.at(hf.Page.size()) << endl;
  
  hf.InstallBootloader();
  hf.DisplayPages();

  cout << "Write to AVR and readback:" << endl;
  uint8_t result {0};
  uint8_t result2 {0};
 
  for (sPage p:hf.Page) {

    cout << "Page#" << setw(2) << p.pagenr;
    
    this->Interact(201);
    if (this->STATUS<=0) { cout << "FAIL" << endl; break; };
    usleep(10000);
    
    this->Interact(p.addrhi);
    if (this->STATUS<=0) { cout << "FAIL" << endl; break; };
    usleep(10000);

    this->Interact(p.addrlo);
    if (this->STATUS<=0) { cout << "FAIL" << endl; break; };
    usleep(10000);

    cout << " Addr Hi=" << setw(1) << (int)p.addrhi << " Lo=" << setw(3) << (int)p.addrlo << " [";

    stringstream ss;
    ss << std::hex;
      
    for (uint8_t i=0;i< PAGE_SIZE; i+=2) {
      this->Interact(p.data[i],p.data[i+1], &result, &result2);
      if (this->STATUS>0)
	ss << (int)result << " " << (int)result2 << " ";
      //cout << "response=" << (int)result << "," << (int)result2 << endl;
      else
	cout << "FAIL" << endl;
      usleep(10000);
    }
    cout << ss.str() << "]" << endl;

    usleep(100000);
  }
  
  cout << "Done." << endl;
  usleep(100000);
 
  cout << "Read back memory pages from AVR:" << endl;
  
  for (sPage p:hf.Page) {
    
    cout << "Page#" << setw(2) << p.pagenr;
     
    this->Interact(202);
    if (this->STATUS<=0) { cout << "FAIL" << endl; break; };
    usleep(10000);
    
    this->Interact(p.addrhi);
    if (this->STATUS<=0) { cout << "FAIL" << endl; break; };
    usleep(10000);

    this->Interact(p.addrlo);
    if (this->STATUS<=0) { cout << "FAIL" << endl; break; };
    usleep(10000);
    
    cout << " Addr Hi=" << setw(1) << (int)p.addrhi << " Lo=" << setw(3) << (int)p.addrlo << " [";
    
    stringstream ss;
    ss << std::hex;

    for (uint8_t i=0;i< PAGE_SIZE; i+=2) {
      this->Interact(&result, &result2);
      if (this->STATUS>0) ss << (int)result << " " << (int)result2 << " ";
      //cout << "response=" << (int)result << "," << (int)result2 << endl;
      else cout << "FAIL" << endl;
      usleep(100);
    }
    cout << ss.str() << "]" << endl;
    
  }
  
  cout << "Done." << endl;
}


void AVRBootloader::ReadFlash() {
  cout << "Read FlashMemory from AVR: "<< endl;
 
  for (int pagenr {0};pagenr < NR_OF_PAGES;pagenr++) {
    
    cout << "Page#" << setw(2) << pagenr;
     
    this->Interact(202);
    if (this->STATUS<=0) { cout << "FAIL" << endl; break; };
    usleep(10000);

    int z = (pagenr << PAGE_SIZE_BITS) + 0x0;
    uint8_t addrhi =(z >> 8) & 255 ;
    uint8_t addrlo = z & 255;
    
    this->Interact( addrhi   ); //ZHI
    if (this->STATUS<=0) { cout << "FAIL" << endl; break; };
    usleep(10000);

    this->Interact( addrlo ); //ZLO
    if (this->STATUS<=0) { cout << "FAIL" << endl; break; };
    usleep(20000);
    
    cout << " Addr Hi=" << setw(1) << (int)addrhi << " Lo=" << setw(3) << (int)addrlo << " [";
    
    stringstream ss;
    ss << std::hex;

    uint8_t result {0};
    uint8_t result2  {0};
    
    for (uint8_t i=0;i< PAGE_SIZE; i+=2) {
      this->Interact(&result, &result2);
      if (this->STATUS>0) ss << (int)result << " " << (int)result2 << " ";
      //cout << "response=" << (int)result << "," << (int)result2 << endl;
      else cout << "FAIL" << endl;
      usleep(1000);
    }
    cout << ss.str() << "]" << endl;
    
  }
  
  cout << "Done." << endl;
}


void AVRBootloader::ProgStart() {
  cout << "Start Program: "<< endl;
 
  this->Interact(203);
  if (this->STATUS<=0) cout << "FAIL" << endl; else cout << "Done" << endl;
  usleep(10000);

}


void AVRBootloader::UnexportRST() {

  if (RST) {
    if (RST->IsExportedGPIO())
      RST->unexportGPIO(); else { this->STATUS=-6; }
  }
}



void print_usage(const string prog)
{
  cout << "Usage:" << prog << " [-vhDrlwoepbc]"                      << endl << endl;
  cout << "  -v --verbose             be wordy "                     << endl;
  cout << "  -h --help                display usage"                 << endl;
  cout << "  -D --device [dev]        device to use (default "       << TTY_DEVICE << ")" << endl;
  cout << "  -r --reset               apply reset pulse"             << endl;
  cout << "  -e --enter               enter bootloader"              << endl;
  cout << "  -w --writeflash [file]   write hex file to flash"       << endl;
  cout << "  -f --readflash           read flash memoy"              << endl;
  cout << "  -p --progstarth          start programy"                << endl;  
  cout << "  -c --cleanup             unexport RST pin"              << endl;  
  exit(1);
}


int main(int argc, char *argv[])
{
  
  struct CommandSeq {
    bool verbose;
    string tty_device;
    bool reset;
    bool enter;
    bool write;
    string writearg;
    bool read;
    bool progstart;
    bool cleanup;
  };
 
  struct CommandSeq mySeq = {
    .verbose=false,
    .tty_device=TTY_DEVICE,
    .reset=false,
    .enter=false,
    .write=false,
    .writearg="",
    .read=false,
    .progstart=false,
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
      { "enter",       0, 0, 'e' },
      { "writeflash",  1, 0, 'w' },
      { "readflash",   0, 0, 'f' },
      { "progstart",   0, 0, 'p' },      
      { "cleanup",     0, 0, 'c' },
      { NULL,          0, 0, 0 },
    };
    int c;
    
    c = getopt_long(argc, argv, "vhrew:fpD:c", lopts, NULL);

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

    case 'e':
      cout << "option e" << endl;
      mySeq.enter=true;
      break;

    case 'w':
      cout << "option w" << endl;
      mySeq.write=true;
      mySeq.writearg=optarg;
      break;

    case 'f':
      cout << "option f" << endl;
      mySeq.read=true;
      break;

    case 'p':
      cout << "option p" << endl;
      mySeq.progstart=true;
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
  AVRBootloader *pAVI = new AVRBootloader(mySeq.tty_device, mySeq.verbose);
  AVISTATUS; AVILOG("done\n");

  if (mySeq.reset) {
    AVILOG("Apply Reset Pulse..."); pAVI->ApplyResetPulse(); AVISTATUS; AVILOG("done\n");
  }

  if (mySeq.enter) {
    AVILOG("Enter Bootloader..."); pAVI->EnterBootloader(); AVISTATUS; AVILOG("done\n");
  }

  if (mySeq.write) {
    AVILOG("Write HexFile to Flash..."); pAVI->WriteFlash(mySeq.writearg); AVISTATUS; AVILOG("done\n");
  }

  if (mySeq.read) {
    AVILOG("Read Flash Memory..."); pAVI->ReadFlash(); AVISTATUS; AVILOG("done\n");
  }

  if (mySeq.progstart) {
    AVILOG("Start Program..."); pAVI->ProgStart(); AVISTATUS; AVILOG("done\n");
  }
  
  if (mySeq.cleanup) {
    AVILOG("Unexport RST"); pAVI->UnexportRST(); AVISTATUS; AVILOG("done\n");
  }

  AVILOG("Destruct Object...");  
  delete(pAVI);
  cout << endl << "Exit." << endl;
  
  return 0;

 
}
