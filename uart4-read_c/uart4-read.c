#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<termios.h>
#include<string.h>
#include<stdlib.h>

/*
// Checks to see if the command is one that is understood by the server
int processCommand(int client, char *command){
   int val = -1;
   if (strcmp(command, "LED on")==0){
      val = message(client, "\r[Turning the LED on]");
      makeLED("value", "1");        // turn the physical LED on
   }
   else if(strcmp(command, "LED off")==0){
      val = message(client, "\r[Turning the LED off]");
      makeLED("value", "0");        // turn the physical LED off
   }
   else if(strcmp(command, "quit")==0){    // shutting down server!
      val = message(client, "\r[goodbye]");
   }
   else { val = message(client, "\r[Unknown command]"); }
   return val;
}
*/


int processCommand(char *command){
  char substring[6];
  char refstring[6]="GPRMC";
  int val=-1;
  
  strncpy(substring, command+1, 5);
  printf("compare substring -%s- with -%s- =%d \n",substring,refstring,strcmp(substring, refstring));
    
  if (strcmp(substring, refstring)==0){
    printf("found!\n");
    val = 1;
   }
  
  return val;  
}


int main(int argc, char *argv[]){
   int client, count=0;
   unsigned char c;
   char *command = malloc(255);

   if ((client = open("/dev/ttyS4", O_RDWR | O_NOCTTY | O_NDELAY))<0){
      perror("UART: Failed to open the file.\n");
      return -1;
   }
   struct termios options;
   tcgetattr(client, &options);
   options.c_cflag = B9600 | CS8 | CREAD | CLOCAL;
   options.c_iflag = IGNPAR | ICRNL;
   tcflush(client, TCIFLUSH);
   fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);  // make reads non-blocking
   tcsetattr(client, TCSANOW, &options);
   //   if (message(client, "\n\rEBB Serial Server running")<0){
   //      perror("UART: Failed to start server.\n");
   //      return -1;
   //   }

   // Loop forever until the quit command is sent from the client or
   //  Ctrl-C is pressed in the server's terminal window
   do {
     if(read(client,&c,1)>0){
       command[count++]=c;

       if(c=='\n'){ //EOL
	 command[count-1]='\0';  // replace /n with /0
	 //write(STDOUT_FILENO,"\n",1);
	 //	 if (processCommand(command)==1){
	 //	   printf("FOUND GPRMC!\n");
	 //	 }
	 count=0;                // reset the command string
       } else {
         write(STDOUT_FILENO,&c,1);
       }
       
     }
   }
   while(strcmp(command,"quit")!=0);
   close(client);
   return 0;
}
