#include<iostream>

using namespace std;

#include <math.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

/* Address of the PORT EXTENDER */
#define I2C_ADDRESS 0x27

int f;

int ReadPortExtender()
{
  int n;
  char buf[10];
  
  n = read(f, buf , 1);
  if (n == -1) {
    perror("read");
    return 1;
  }

  printf("Read Result: 0x%x\n",
	 buf[0] );

  return 0;
}


int WritePortExtender(char lsb)
{
  int n;
  char buf[10];
  
  buf[0] = lsb;
  
  n = write(f, buf, 1);
  if (n == -1) {
    perror("write");
    return 1;
  }
  
  printf("Written: 0x%x\n",
	 buf[0] );

  return 0;
}

int main(int argc, char *argv[]) {


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

  // Port Extender Pins
  // [GND VCC VO RS RW E] >>DB0 1 2 3 4 5 6 7<< [A K]
  
  ReadPortExtender();
  //WritePortExtender(255-32);  // DB5 = LO (=LED)
  //WritePortExtender(255-128); // DB7 = LO
  
  for (int i=0; i<100; i++) {
    WritePortExtender(0); // DB5=LO LED on
    ReadPortExtender();  
    cout << "LED ON" << endl;
    usleep( 500000 ); //500ms
    WritePortExtender(32); // DB5=HI LED off
    ReadPortExtender();  
    usleep( 500000 ); //500ms
  }
  ReadPortExtender();  
  
  close(f);
  return 0;
}
