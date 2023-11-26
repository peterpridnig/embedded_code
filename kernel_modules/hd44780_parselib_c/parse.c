#include <string.h>
#include <stdio.h>
#include <parselib.h>
  
int main()
{
 //char myDisplayText[]="<clr>Peter";
 //char myDisplayText[]="<pos 2,12>Peter Pridnig";
 //char myDisplayText[]="<scr left>";
 //char myDisplayText[]="<scr right>";
 char myDisplayText[]="Seltschach";

 /* grammar
    <clr>DISPLAYTEXT            |
    <pos line,col>DISPLAYTEXT   |
    <scr left|right>IGNOREDTEXT |
    DISPLAYTEXT
 */

 int line=0;
 int col=0;
 int textindex=0;

 printf("Parser Start:\n");
   
 tDisplayCommand Command;
 Command = ParseCommand(myDisplayText, &textindex, &line, &col);

 printf("Parser Result:\n");
 
 if ( Command==clear )//clear
   {
     printf("Command: clear\n");
     printf("Text to display: %s\n", &myDisplayText[textindex]);
   };

 if ( Command==position )//position
   {
     printf("Command: position\n");
     printf("line=%d col=%d\n",line,col);
     printf("Text to display: %s\n", &myDisplayText[textindex]);
    };
 
  if ( Command==scroll_left )
   {
     printf("Command: scroll left\n");
    };

  if ( Command==scroll_right )
   {
     printf("Command: scroll right\n");
    };
 
 if ( Command==none )//none
   {
     printf("none\n");
     printf("Text to display: %s\n", &myDisplayText[textindex]);
   };

 if ( Command==ERROR )//error
   {
     printf("ERROR\n");
   };



 

 return 0;
}
