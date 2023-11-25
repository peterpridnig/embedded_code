#include <cstring>
#include <string.h>
#include <stdio.h>


enum DisplayCommands
{
  clear,
  position,
  scroll_left,
  scroll_right,
  none,
  ERROR
};

typedef enum DisplayCommands tDisplayCommand;

int myatoi(char* Num)
 {
   int i, number, exp;
   number=0;
   exp=1;
 
   for (i=strlen(Num)-1; i>=0; i--)
     { number+=exp*(Num[i]-48);
       exp*=10;
     };

   return number;
 }


// parse string c from to max until del and copy to target
void ParseUntilDel(char* c, char* target, int from, int max, char del)
{
  char* t;
  int i;
  i=0;
  
  t=&c[from];
  while (*t!='\0' && (*t != del) && i<max ) {i++; t++;};
  strncpy(target, &c[from], i);
  target[i]='\0';
};

tDisplayCommand ParseCommand(char* c, int* i, int* line, int* col)
{
  char sClear[]="<clr>";
  char sPos[]  ="<pos "; //line,col>
  char sScr[]  ="<scr "; //left|right>
    
  char temp[6];
  char* t, l;
  int lenline, lencol;

  *i=0; *line=-1, *col=-1;

  printf("c=%s\n",c);
 
  if ( strlen(c)>=5 ) { strncpy(temp, c, 5); } else return none;
  temp[5]='\0';
  printf("temp=%s\n",temp);
  
  if ( strcmp(temp,sClear)==0) //clr
    { return clear; }

  if ( strcmp(temp,sPos)==0) //pos
    { ParseUntilDel(c, temp, 5, 5, ',');
      lenline=strlen(temp)+1;
      if (lenline>2) goto error;
      *line=myatoi(temp);
      
      ParseUntilDel(c, temp, 5+lenline, 5, '>');
      lencol=strlen(temp)+1;
      if (lencol>3) goto error;
      *col=myatoi(temp);
      
      *i=5+lenline+lencol; // here the text starts

      return position;
    }

  if ( strcmp(temp,sScr)==0) //scr
    {  ParseUntilDel(c, temp, 5, 6, '>');
      printf("temp=%s %d\n",temp,strlen(temp));
      if (strcmp(temp,"left"))
	{ return scroll_left; } else
	{ return scroll_right; };

    }
 
  return none;

 error: return ERROR;
  
};


  
int main()
{

 //char myDisplayText[]="<clr>";
 //char myDisplayText[]="<pos 2,12>Peter Pridnig";
 char myDisplayText[]="<scr right>";
 //char myDisplayText[]="Seltschach";
 /* grammar
    !<clr>IGNOREDTEXT
    !<pos line,col>TEXT
    !<scr left|right>IGNOREDTEXT
    TEXT
 */

 int line=0;
 int col=0;
 int left=0;
 int textindex=0;

 char Num[]="123";
 
 printf("Command Text: %s\n",myDisplayText);
 
 tDisplayCommand Command = ParseCommand(myDisplayText, &textindex, &line, &col);
 
 if ( Command==clear )//clear
   {
     printf("Command: clear\n");
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
