#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<termios.h>
#include<string.h>
#include<stdlib.h>
#include<sys/ioctl.h>
#include<unistd.h> //for usleep
#include<stdint.h>

// echo 17 > /sys/class/gpio/export
// echo out > /sys/class/gpio/gpio17/direction
// echo 1 > /sys/class/gpio/gpio17/value

void avr_lifesign(const int &client)
{
  int writeval=1; 
  uint8_t readval=0;   

  write(client,&writeval,1);
  if(read(client,&readval,1)>0) { // read performed w/o error
    if (readval==100) { // life sign OK
      printf("Lifesign PASS: write=%d read=%d\n",writeval,readval);
    } else {
      printf("Lifesign FAIL: write=%d read=%d\n",writeval,readval);
    } 
  }
  else {
    printf("Write FAIL!\n");
  }
  usleep(10000); //5m
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



int main(int argc, char *argv[]){
  int client=0;
  //char message[]="Peter Pridnig";
  unsigned char c;
	
  if ((client = open("/dev/ttyS2", O_RDWR | O_NOCTTY | O_NDELAY))<0){
    perror("UART: Failed to open the file.\n");
    return -1;
  }
  struct termios options;
  //https://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c
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

  int rep=3;

  //printf("WROSCAL\n"); for (int i=0; i<rep; i++) avr_writeoscal(client,74); //74
  printf("RDOSCAL\n");  for (int i=0; i<rep; i++) avr_readoscal(client);
  printf("LIFE\n");  for (int i=0; i<rep; i++) avr_lifesign(client); // expected 100
  printf("LENGTH\n");  for (int i=0; i<rep; i++) avr_readbytelength(client); // target 225
  //printf("WROSCALEEPROM\n");  for (int i=0; i<rep; i++) avr_writeoscaleeprom(client,74); //osc must be trimmed
  printf("RDOSCALEEPROM\n");  for (int i=0; i<rep; i++) avr_readoscaleeprom(client);
  
  /*
    int myval=1; // life sign

    uint8_t val=0;   

    for (uint8_t j=60; j<70; j++)
      {
	printf("try %d\n",j);
	write(client,&myval,1);
	//usleep(100);
	if(read(client,&val,1)>0) {
	  //if (val>=192) val-=128;
	  printf("write=%d read=%d\n",myval,val);
	}
   
	usleep(500000); //500ms
      }

  */
   
    close(client);
    return 0;
}
