#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<termios.h>
#include<string.h>
#include<stdlib.h>
#include<sys/ioctl.h>
#include<unistd.h> //for usleep
#include<stdint.h>




int main(int argc, char *argv[]){
   int client=0;
   char message[]="Peter Pridnig";
   unsigned char c;
	
   if ((client = open("/dev/ttyS2", O_RDWR | O_NOCTTY | O_NDELAY))<0){
      perror("UART: Failed to open the file.\n");
      return -1;
   }
   struct termios options;
   //https://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c
   tcgetattr(client, &options);
   options.c_cflag = B115200 | CS8 | CREAD | CLOCAL;
   options.c_iflag = IGNPAR | ICRNL;
   tcflush(client, TCIFLUSH);
   fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);  // make reads non-blocking
   options.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
   options.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
   options.c_oflag &= ~OPOST;
   options.c_cc[VMIN] = 1;
   options.c_cc[VTIME] = 1;
   tcsetattr(client, TCSANOW, &options);

 
   int size = strlen(message);
   printf("Testmessage: %s\n", message);   // print message with new line
   if (write(client, message, size+1)<0){
     perror("UART: Failed to start server.\n");
   }
   
   usleep(100);
   uint8_t val =100;
   //char *val="0X90";
   write(client,&val,1);

   //microcom -s 115200 /dev/ttyS2

   do {
     if(read(client,&c,1)>0){
       printf("%c\n",c);
       
       write(client,&c,1);
       usleep(100);
       write(client,&c,1);

       usleep(100);
       ioctl(client, TIOCSBRK);
       usleep(5000);
       ioctl(client, TIOCCBRK);
       usleep(100);
   

     }
     if(read(STDIN_FILENO,&c,1)>0){ // can send from stdin to client
       write(client,&c,1); }

   }
   while(c!='q');

   
   close(client);
   return 0;
}
