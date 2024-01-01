#include<iostream>
#include<fstream>
#include<cstring>
#include<iomanip>
#include<vector>

using namespace std;

#define PAGE_SIZE_BITS 5
#define PAGE_SIZE (1 << PAGE_SIZE_BITS)
#define NR_OF_PAGES 32

/*
RAMSIZE  = 1024d = 2^10
PAGESIZE =   32d = 2^5
NRPAGES  =   32d = 2^5 

PAGE #0    00d = #0 x 32 / 0x00 : 32 x 1byte
 0x00 1byte
 0x01 1byte
 0x02 1byte
 ...
 0x1F 1byte

PAGE  #1   32d = #2 x 32 / 0x020 : 32 x 1byte
PAGE  #2   64d = #3 x 32 / 0x040 : 32 x 1byte 
...
PAGE #31 920d = #31 x 32 / 0x3e0 : 32 x 1 byte

hex code contains byte addresses:
ADDR = 0..1023 = 0x000... 0x3FF (11 1111 1111)

determine page number from ADDR: ADDR >> 5
determine address inside PAGE: ADDR bitand 0x1F

e.g. ADDR=0x0020: page nr. = 0x001; addr inside page = 0x00
e.g. ADDR=0x0030: page nr. = 0x001; addr inside page = 0x10

 */

struct sPage {
  int pagenr;       //0..31
  uint8_t addrhi;
  uint8_t addrlo;
  uint8_t data[32]; //0..31
};

class cHexFile {
public:
  ifstream fs;
  int FileStatus {0}; // >0 = OK
  vector <sPage> Page {};
  bool verbose {false};

  
  cHexFile(string, bool);
  ~cHexFile();

  uint8_t hex2dec(string);
  void ReadHexFileContent();
  void DisplayPages();
  void InstallBootloader();
  
};






