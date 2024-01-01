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

#include"HEX.h"
using namespace std;


cHexFile::cHexFile(string filename, bool verbose) {
  this->verbose=verbose;
  fs.open(filename.c_str());
  if (!fs.is_open()){
    perror("GPIO: read failed to open file ");
  }
    
};

cHexFile::~cHexFile() {
  Page.clear();
  fs.close();
};



uint8_t cHexFile::hex2dec(string s) {
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
  
void cHexFile::ReadHexFileContent() {
  // https://en.wikipedia.org/wiki/Intel_HEX
  
  this->FileStatus=1;
  string sol {':'};
  string line {""};
    
  sPage myPage = {.pagenr {0}, .addrhi {0}, .addrlo {0}, .data {255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255} };
  int pagenr, pageaddr;
    
  uint8_t nrofbytes {0};
  uint8_t addrhi, addrlo;
  int address {0};
  uint8_t recordtype {0};
  uint8_t esahi, esalo;
  int esa {0};
  uint8_t byte {0};
  uint8_t checksum {0};
    
  while (!fs.eof())
    {
      getline(fs,line);
      if (fs.eof()) break;
      checksum=0;

      if (line.length()<1+2+4+2+2)  { FileStatus=-1; break;} //min length
      if (line.substr(0,1) != sol) { FileStatus=-2; break;}  //:
      if (this->verbose) cout << sol << " ";

      nrofbytes=this->hex2dec(line.substr(1,2)); checksum+=nrofbytes;
      if (this->verbose) cout << "NR=" << line.substr(1,2) << "(" << (int)nrofbytes << "d)";
      if (line.length()<1+2+4+2+(unsigned)nrofbytes*2+2)  { FileStatus=-1; break;} //min length

      addrhi=this->hex2dec(line.substr(3,2)); checksum+=addrhi;
      addrlo=this->hex2dec(line.substr(5,2)); checksum+=addrlo;
      address=addrhi*256+addrlo;
      if (this->verbose) cout << "ADDR=" << line.substr(3,4) << "(" << (int)address << "d)";

      recordtype=this->hex2dec(line.substr(7,2)); checksum+=recordtype;
      if (this->verbose) cout << "RT="<< line.substr(7,2) << "(" << (int)recordtype << "d)";

      if (recordtype==2) { //Extended Segment Address
	if (this->verbose) cout << "ESA=";
	esahi=this->hex2dec(line.substr(9,2)); checksum+=esahi;
	esalo=this->hex2dec(line.substr(9+2,2)); checksum+=esalo;
	esa=16*(esahi*256+esalo);
	if (this->verbose) cout << line.substr(9,4) << "(" << (int)esa << "d),";
      }
	
      if (recordtype==0) { //Data
	if (this->verbose) cout << "DATA=";

	pagenr = (esa+address) >> PAGE_SIZE_BITS;
	pageaddr = ( (esa+address) & (PAGE_SIZE-1) ); //address inside page

	//cout << endl;
	//cout << "pagenr: " << pagenr << " pageaddr" << pageaddr << "size: " << PAGE_SIZE << endl;
	  
	if (pagenr > myPage.pagenr) {
	  Page.push_back(myPage); // save current page
	  myPage.pagenr=pagenr;   // create new empty page
	  int z = (myPage.pagenr << PAGE_SIZE_BITS) + 0x00000;
	  myPage.addrhi =(z >> 8) & 255 ;
	  myPage.addrlo = z & 255;
	  for (size_t i=0;i<PAGE_SIZE;i++) myPage.data[i]=255;
	}

	for (uint8_t i=0;i< nrofbytes; i++) {
	  byte=this->hex2dec(line.substr(9+i*2,2)); checksum+=byte;
	  myPage.data[pageaddr+i]=byte;
	  if (this->verbose) cout << line.substr(9+i*2,2) << "(" << (int)byte << "d),";	    
	}
	  
      }

      uint8_t cksum=this->hex2dec(line.substr(9+nrofbytes*2,2)); checksum+=cksum;
      if (this->verbose) cout << "CK=" << line.substr(9+nrofbytes*2,2) << "(" << (int)cksum << "d)";
      if (this->verbose) cout << "CHECKS=" << (int)checksum;
      if (checksum) { FileStatus=-2; break;} 

      if (this->verbose) cout << " status: " << this->FileStatus << endl;

    }

  Page.push_back(myPage); // save current page


}


void cHexFile::DisplayPages() {

  cout << "Memory pages: " << endl;
  
  for (sPage p:Page) {
    cout << "Page#" << setw(2) << p.pagenr << " Addr Hi=" << setw(1) << (int)p.addrhi << " Lo=" << setw(3) << (int)p.addrlo <<" [";
    stringstream ss;
    ss << std::hex;
    for (uint8_t i=0;i< PAGE_SIZE; i++) ss << (int)p.data[i] << " ";
    cout << ss.str() << "]" << endl;
  }

  cout << "Done" << endl;

}


void cHexFile::InstallBootloader() {

/* Init.hex
:02 0000 00 "7F C1" BE 
.org    $0000
	rjmp $0180 = "7FC1"
7FC1 = C 17F i.e. C=jump to 17F+1 =0180

:02 02FC 00 "91CE" A1
.org $017E (=2FC >> 1)
      rjmp $0010 = "91CE" => "00C0"
91CE = C E91 i.e. C=jump to inv(E91)=inv(1110 1001 0001)=0001 0110 1110 = -0x16E

e.g blink2.hex
:nr addr rt 
:10 0000 00 "00C0" ...
rjmp Anfang = "00C0" => "7FC1"
C000 = C 000 i.e. C=jump to 000+1=0x001

substitute page content:
Page# 0 Addr Hi=0 Lo=  0 [<7f> <c1> ...
Page#23 Addr Hi=2 Lo=252 [0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 <yz> <cx> 0 0 ]
xyz = -(0x17E - 0x001) = inv(0x17D)

*/

  cout << "Installing Bootloader links: " << endl;

    // remember rjmp Anfang
  uint8_t ab {Page.at(0).data[0]};
  uint8_t cd {Page.at(0).data[1]};
  // cd ab = 1100 dab
  //         rjump 0xdab

  // rjmp $0180
  Page.at(0).data[0]=127; //7f
  Page.at(0).data[1]=193; //c1
  // 7F C1 i.e.
  // C1 7F = 1100 0001 0111 1111
  //       = rjump 0x17E+1=0x180

  // init.asm
  //.org $017E
  //    rjmp -(0x17E-0xdab)
  //    rjmp inv(0x17E-0xdab)
  //    C    xyz
  // yz Cx

  int delta = ( 1*256+7*16+14 - ( (cd & 15)*16 + ab + 1) );
  cout << "delta=" << delta << endl;
  delta=~(delta);

  uint8_t yz = delta & 255;
  uint8_t cx = (12 << 4) + ((delta >> 8) & 15);

  cout << " cx=" << (int)cx << " yz=" << (int)yz << endl;
  
  sPage myPage = {.pagenr {23}, .addrhi {2}, .addrlo {224}, .data {255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,yz,cx,255,255} };

  Page.push_back(myPage);
        
  cout << "Done" << endl;
    
}

