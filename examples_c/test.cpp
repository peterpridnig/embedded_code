#include <iostream>
#include <string>
#include <stdio.h>

using namespace std;

int main() {
  string my_str;

  cout << "Enter a pet name: ";
  /*
  cin >> ws;
  getline(cin, my_str, '\n');
  while(my_str.length()==0){
    getline(cin>>ws,my_str, '\n');
  cout << "length:" << my_str.length() << endl;
  }
  */

  char mychar;
  mychar=getchar();
  printf("%c\n",mychar);
  
  cout << "My pet's name is " + my_str + "!";
}
