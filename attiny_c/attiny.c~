#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<termios.h>
#include<string.h>
#include<stdlib.h>
#define  GPIO_NR   "122"
#define  LED_PATH  "/sys/class/gpio/gpio122/"

// echo 122 > /sys/class/gpio/export
// echo out > /sys/class/gpio/gpio122/direction
// echo 1 > /sys/class/gpio/gpio122/value
// echo 122 > /sys/class/gpio/unexport

void DisplayHelp(int client);

// Sends a message to the client and displays the message on the console
int message(int client, char *message){
   int size = strlen(message);
   printf("Server>>>%s\n", (message+1));   // print message with new line
   if (write(client, message, size)<0){
      perror("Error: Failed to write to the client\n");
      return -1;
   }
   write(client, "\rEBB>", 5);           // display a simple prompt
   return 0;                               // \r for a carriage return
}

void exportLED(){
   FILE* fp;   // create a file pointer fp
   fp = fopen("/sys/class/gpio/export","w"); // open file for writing
   fprintf(fp, "%s", GPIO_NR);  // send the value to the file
   fclose(fp);  // close the file using the file pointer
}

void unexportLED(){
   FILE* fp;   // create a file pointer fp
   fp = fopen("/sys/class/gpio/unexport","w"); // open file for writing
   fprintf(fp, "%s", GPIO_NR);  // send the value to the file
   fclose(fp);  // close the file using the file pointer
}

void makeLED(char filename[], char value[]){
   FILE* fp;   // create a file pointer fp
   char  fullFileName[100];  // to store the path and filename
   sprintf(fullFileName, LED_PATH "%s", filename); // write path and filename
   fp = fopen(fullFileName, "w+"); // open file for writing
   fprintf(fp, "%s", value);  // send the value to the file
   fclose(fp);  // close the file using the file pointer
}

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
   else if(strcmp(command, "help")==0){
      val = message(client, "\r[Display help]");
      DisplayHelp(client);
   }
  else if(strcmp(command, "acc")==0){
      val = message(client, "\r[Starting accelerometer]");
      system("insmod hd44780.ko && ./accelerometer &");
  }
 
   else if(strcmp(command, "quit")==0){    // shutting down server!
      val = message(client, "\r[goodbye]");
   }
   else { val = message(client, "\r[Unknown command]"); }
   return val;
}

void DisplayHelp(int client) {
  message(client, "\r commands: \r   help \r   LED on \r   acc \r   LED off \r   quit");
}

int main(int argc, char *argv[]){
   int client, count=0;
   unsigned char c;
   char *command = malloc(255);
   printf("export led\n");
   exportLED();
   printf("make led\n");
   makeLED("direction", "out");            // the LED is an output

   if ((client = open("/dev/ttyS1", O_RDWR | O_NOCTTY | O_NDELAY))<0){
      perror("UART: Failed to open the file.\n");
      return -1;
   }
   struct termios options;
   tcgetattr(client, &options);
   options.c_cflag = B115200 | CS8 | CREAD | CLOCAL;
   options.c_iflag = IGNPAR | ICRNL;
   tcflush(client, TCIFLUSH);
   fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);  // make reads non-blocking
   tcsetattr(client, TCSANOW, &options);
   if (message(client, "\n\rEBB Serial Server running")<0){
      perror("UART: Failed to start server.\n");
      return -1;
   }
   DisplayHelp(client);

   // Loop forever until the quit command is sent from the client or
   //  Ctrl-C is pressed in the server's terminal window
   do {
      if(read(client,&c,1)>0){
          write(STDOUT_FILENO,&c,1);
          command[count++]=c;
          if(c=='\n'){
             command[count-1]='\0';  // replace /n with /0
             processCommand(client, command);
             count=0;                // reset the command string
          }
      }
      if(read(STDIN_FILENO,&c,1)>0){ // can send from stdin to client
          write(client,&c,1);
      }
   }
   while(strcmp(command,"quit")!=0);
   close(client);
   unexportLED();
   return 0;
}
