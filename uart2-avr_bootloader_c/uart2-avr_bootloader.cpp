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
    nriter=-1;
  };
  enum eStatus status;
  uint8_t result;
  int nriter;
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
  void Interact(uint8_t, uint8_t*); //write cmd, read result
  void Interact(uint8_t, uint8_t, uint8_t*); //write cmd, write arg, read result
  void Interact(uint8_t, uint8_t); //write cmd, write val
  void Interact(uint8_t*); //RXD=0, read result
  
public:
  AVRBootloader(string,bool);
  ~AVRBootloader();
  int GetStatus() {return STATUS; } //>0 means OK
  void ApplyResetPulse();
  void EnterBootloader();
  void WriteHex(string);
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
    
    if (result==105)     
      cout << "Bootloader entered successfully" << endl;
    else {
      this->STATUS=-5;
      cout << "Bootloader does not respond." << endl;
    }
    
  } else { this->STATUS=-3; }

}


struct sHexline {
  char delimiter;
  //...
};
  
class cHexFile {
public:
  ifstream fs;
  int FileStatus {0}; //>0 OK
  vector <string> content {};

  cHexFile(string filename) {
    fs.open(filename.c_str());
    if (!fs.is_open()){
      perror("GPIO: read failed to open file ");
    }
    
    //struct sHexLine hexline = { .delimiter="", };
    //this->content.push_back(hexline);
    
  };
  ~cHexFile() {
    fs.close();
  };

  uint8_t hex2dec(string s) {
    if (s.length()!=2) { this->FileStatus=-1; return 0; }
    // 0..9 = 48..57; A..F = 65..70
    uint8_t ln=int(s.at(1)); // lower nibble
    uint8_t un=int(s.at(0)); // upper nibble

    if ((ln<48) || (un<48)) { this->FileStatus=-1; return 0; }
    if ((ln>70) || (un>70)) { this->FileStatus=-1; return 0; }
    if ((ln>57) && (ln<65)) { this->FileStatus=-1; return 0; }
    if ((un>57) && (un<65)) { this->FileStatus=-1; return 0; }    
    
    if (ln <=57) ln-=48; else ln-=55;    
    if (un <=57) un-=48; else un-=55;
    return un*16+ln;

  }
  
  void ReadContent() {

    this->FileStatus=1;
    string sol {':'};
    string line {""};
    
    while (!fs.eof())
      {
	getline(fs,line);

	if (line.length()<1+2+4+2+2)  { FileStatus=-1; break;} //min length
	if (line.substr(0,1) != sol) { FileStatus=-2; break;} //:
	cout << sol << " ";

	uint8_t nrofbytes=this->hex2dec(line.substr(1,2)); 
	cout << "NR=" << line.substr(1,2) << "(" << (int)nrofbytes << "d)";
        if (line.length()<1+2+4+2+(unsigned)nrofbytes*2+2)  { FileStatus=-1; break;} //min length
	
	int address=this->hex2dec(line.substr(3,2))*16+this->hex2dec(line.substr(5,2)); 
	cout << "ADDR=" << line.substr(3,4) << "(" << (int)address << "d)";

	uint8_t recordtype=this->hex2dec(line.substr(7,2));
	cout << "RT="<< line.substr(7,2) << "(" << (int)recordtype << "d)";

	if (recordtype==0) { //Data
	  cout << "DATA=";
	  for (uint8_t i=0;i< nrofbytes; i++) {
	    uint8_t byte=this->hex2dec(line.substr(9+i*2,2));
	    cout << line.substr(9+i*2,2) << "(" << (int)byte << "d),";	    
	  }
	}
	
	if (recordtype==2) { //Extended Segment Address
	  cout << "ESA=";
	  int esa=this->hex2dec(line.substr(9,2))*16+this->hex2dec(line.substr(9+2,2));
	  cout << line.substr(9,4) << "(" << (int)esa << "d),";	    
	}

	uint8_t cksum=this->hex2dec(line.substr(9+nrofbytes*2,2)); 
	cout << "CK=" << line.substr(9+nrofbytes*2,2) << "(" << (int)cksum << "d)";

	cout << " status: " << this->FileStatus << endl;

      }

    /*
      for (string l:content) {
      //cout << l << endl;
      cout << l.substr(0,1) << l.substr(1,2) << " / " << l << endl;
      }
    */

  }
};

void AVRBootloader::WriteHex(string filename) {
  cout << "Write " << filename << endl;

  cHexFile hf(filename);
  hf.ReadContent();
  
  // https://en.wikipedia.org/wiki/Intel_HEX
  
  cout << "Done." << endl;
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
  cout << "  -w --writehex [file]     write hex file"                << endl;  
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
    bool cleanup;
  };
 
  struct CommandSeq mySeq = {
    .verbose=false,
    .tty_device=TTY_DEVICE,
    .reset=false,
    .enter=false,
    .write=false,
    .writearg="",
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
      { "write",       1, 0, 'w' },      
      { "cleanup",           0, 0, 'c' },
      { NULL,                0, 0, 0 },
    };
    int c;
    
    c = getopt_long(argc, argv, "vhrew:D:c", lopts, NULL);

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

  if (mySeq.enter) {
    AVILOG("Write HexFile..."); pAVI->WriteHex(mySeq.writearg); AVISTATUS; AVILOG("done\n");
  }
  
  if (mySeq.cleanup) {
    AVILOG("Unexport RST"); pAVI->UnexportRST(); AVISTATUS; AVILOG("done\n");
  }

  AVILOG("Destruct Object...");  
  delete(pAVI);
  cout << endl << "Exit." << endl;
  
  return 0;

 
}
