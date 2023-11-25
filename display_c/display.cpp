#include <cstring>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string.h>


int main()
{
  std::ofstream fs;
  std::string mytext {"Peter Pridnig"};
 
 fs.open("/dev/hd44780");
 if (!fs.is_open()) {
	perror("kann /dev/hd44780 nicht oeffnen.\n");
	perror("insmod hd44780.ko.\n");
	return -1;
 }
 
 fs << mytext;

 fs.close();
return 0;
}
