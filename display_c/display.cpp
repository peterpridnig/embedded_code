#include <cstring>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string.h>


int main()
{
  std::ofstream fs;
  std::string mytext {"Peter Pridnig"};
  std::string CmdClear {"<clear>"};
  
 char cmdtext[] {"<clear> <setpos 2,05>"};
 char myCmdClear[] {"<clear>"};
 char myCmdSetPos[] {"<setpos"};
 char X[2];
 char Y[1];
 
 fs.open("/dev/hd44780");
 if (!fs.is_open()) {
	perror("kann /dev/hd44780 nicht oeffnen.\n");
	perror("insmod hd44780.ko.\n");
	return -1;
 }
 
 fs << mytext;

 char* p;

 p = strstr(cmdtext, myCmdClear );
 if (p) { std::cout << "found clear" << std::endl; }

 p = strstr(cmdtext, myCmdSetPos);
  if (p) {
    std::cout << "found setpos" << std::endl;
    strcpy(Y,p[8]);
    strcpy(X,p[10]);
    printf("Y=%d X=%c", atoi(Y),atoi(X));
  }
 
    
 fs.close();
return 0;
}
