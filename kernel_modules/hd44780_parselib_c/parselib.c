#include <string.h>
#include <stdio.h>
#include <parselib.h>

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
  char sClear[]="<clr";  //>
  char sPos[]  ="<pos "; //line,col>
  char sScr[]  ="<scr "; //left|right>
    
  char temp[7];
  //char* t, l;
  int lenline, lencol;

  #ifdef DEBUGPRINTF
  printf("my cmd: %s len=%d\n",c,(int)strlen(c));
  #endif
  
  *i=0; *line=-1, *col=-1;
 
  //if ( strlen(c)>=5 ) { strncpy(temp, c, 5); } else { return none; };
  //temp[5]='\0';
  ParseUntilDel(c, temp, 0, 5, '>');
    
  #ifdef DEBUGPRINTF
  printf("temp1: %s len=%d\n",temp,(int)strlen(temp));
  #endif
  
  if ( strcmp(temp,sClear)==0) //clr
    {
      #ifdef DEBUGPRINTF
      printf("found clear\n");
      #endif
      *i=5;
      return clear; }

  if ( strcmp(temp,sPos)==0) //pos
    { ParseUntilDel(c, temp, 5, 5, ',');
      lenline=strlen(temp)+1;
      if (lenline>2) goto error; //no more than 1 char for the linenr
      *line=myatoi(temp);
      
      ParseUntilDel(c, temp, 5+lenline, 5, '>');
      lencol=strlen(temp)+1;
      if (lencol>3) goto error; //no more than 2 chars for the col
      *col=myatoi(temp);
      
      *i=5+lenline+lencol; // here the text starts

      return position;
    }

   if ( strcmp(temp,sScr)==0) //scr
    {  ParseUntilDel(c, temp, 5, 6, '>');
      //temp[strlen(temp)]='\0';
      
      #ifdef DEBUGPRINTF
      printf("temp2=%s len=%d\n",temp,(int)strlen(temp));
      #endif
      
      if (strcmp(temp,"left")==0)
	{ return scroll_left; };

      if (strcmp(temp,"right")==0)
	{ return scroll_right; };

      goto error;

    }
   
  return none;

 error: return ERROR;
  
};

