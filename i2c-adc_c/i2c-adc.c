/* Copyright (C) 2017, Chris Simmonds (chris@2net.co.uk) */

/*
 * Sample program to read ADC i2c registers
 */

/* Set the 16-bit address to read  */
  /* address byte 1 : 0 = conv reg      */
  /* address byte 1 = 1 = conf reg      read = 0x85 0x83*/
  /* address byte 1 : 2 = lo thresh reg read = 0x80 0x00*/
  /* address byte 1 : 3 = hi thresh reg read = 0x7F 0xFF*/


#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

/* Address of the ADC */
#define I2C_ADDRESS 0x48

int f;

int ReadReg(int regnr)
{

  int n;
  char buf[10];
  
  buf[0] = regnr;
	
  n = write(f, buf, 1);
  if (n == -1) {
    perror("write");
    return 1;
  }

  /* Now read 2 bytes from that address */
  n = read(f, buf, 2);
  if (n == -1) {
    perror("read");
    return 1;
  }

  printf("Read Reg Nr. %d:0x%x 0x%x\n",
	 regnr, buf[0], buf[1]);

  return 0;
}


int WriteReg(int regnr, char msb, char lsb)
{

  int n;
  char buf[10];

  buf[0] = regnr;
  buf[1] = msb;
  buf[2] = lsb;
  
  n = write(f, buf, 3);
  if (n == -1) {
    perror("write");
    return 1;
  }
  
  printf("Written Reg Nr. %d:0x%x 0x%x\n",
	 regnr, buf[1], buf[2]);

  return 0;
}

int Convert(int AIN)
{
  //config reg MSBs
  // 15: write/1= start a single conversion
  // OS  read /0= device currently performing conversion
  //          /1= not performing conversion
  // 14:12 : 0x100 AIN0
  // MUX     0x101 AIN1
  //         0x110 AIN2
  //         0x111 AIN3
  // 11:9 :  000 FSR=6.1 V
  // PGA     001 FSR=4.096
  //         010     2.048
  //         011     1.024
  //         100     0.512
  //         101     0.256
  //         110     0.256
  //         111     0.256
  // 8 :     0: cont-conv mode

  //config reg LSBs
  // MODE    1: single-shot
  // 7:5 :   100: 1600 SPS (default)
  // DR      101: 2400 SPS
  // 4 :     0 traditional
  // COMP    1 window
  // 3 :     0 active lo (default)
  // COMPPOL 1 active hi
  // 2 :     0 (default)
  // COMPLAT
  // 1:0 :   11 (default
  
  // a xy    b 
  // 1100 0101 1000 0011
  // C    1    8    3
  // a= start conv
  // b=single conv
  // xy=AIN0,1,2

  // FS= 000 = 6.1V
  // Vin =     1.7V Dout = 1.7/6.1*4096/2=1138/2=569
  // Vin =     1.3V Dout = 1.3/6.1*4096/2       =436

  // FS= 010 = 2.048V <===
  // Vin =     1.7V Dout = 1.7/2.048*4096/2=1138/2=1700
  // Vin =     1.3V Dout = 1.3/2.048*4096/2       =1300

  // AIN3=00     AIN0=01     AIN1=10     AIN2=11
  /*
  if (AIN==0) {AIN=1;}
  else if (AIN==1) {AIN=2;}
  else if (AIN==2) {AIN=3;}
  else if (AIN==3) {AIN=0;}
  */
  AIN = (AIN+1) & 0x3;
  
  int n;
  char buf[10];
  //  WriteReg(1,0xC7,0x83);
  buf[0] = 1; //config reg
  buf[1] = 0xC5 | ( (AIN & 3) << 4);
  buf[2] = 0x83;
  
  n = write(f, buf, 3);
  if (n == -1) {
    perror("write");
    return 1;
  }

  buf[0] = 0; //conv reg
	
  n = write(f, buf, 1);
  if (n == -1) {
    perror("write");
    return -1;
  }

  /* read 2 bytes from conv address */
  n = read(f, buf, 2);
  if (n == -1) {
    perror("read");
    return -1;
  }
  
  //printf("0x%x 0x%x %d\n", buf[0], buf[1], ((buf[0]<<8)+buf[1])>>4);

  return ((buf[0]<<8)+buf[1])>>4;
    
}


int main(void)
{
  /* Open the adapter and set the address of the I2C device */
  f = open("/dev/i2c-2", O_RDWR);
  if (f < 0) {
    perror("/dev/i2c-0:");
    return 1;
  }

  /* Set the address of the i2c slave device */
  if (ioctl(f, I2C_SLAVE, I2C_ADDRESS) == -1) {
    perror("ioctl I2C_SLAVE");
    return 1;
  }

  ReadReg(0); //conv
  ReadReg(1); //conf
  ReadReg(2);
  ReadReg(3);

 

  /*
  WriteReg(1,0x44,0x80);
  ReadReg(1); //conf
    
  while(1){
    ReadReg(0);} //conv
  */	

  //printf("0x%x", 0xC5 | ( (2 & 3) << 4));

	 
  while(1){
    printf("AIN0=%d AIN1=%d AIN2=%d AIN3=%d \n",Convert(0), Convert(1), Convert(2), Convert(3));
    //                                          AIN3=00     AIN0=01     AIN1=10     AIN2=11

  }
  
  close(f);
  return 0;
}
