#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

using namespace std;


class cResult {

public:
  cResult() {
    number=0;
    timestamp=0.0;
  };
  int number;
  float timestamp;
};


struct sResult
{
  int number;
  float timestamp;
};
 

void *threadFunction(void *value){
  int *x=(int *)value;
  while (*x<5){
    usleep(2000);
    (*x)++;
  }

  /*
  struct sResult * pNewResult;
  pNewResult = (sResult *)malloc(1 * sizeof(struct sResult));
  pNewResult->number=(*x);
  pNewResult->timestamp=0.1;
  return pNewResult;
  */


  cResult *oResult = new (cResult);
  oResult->number=(*x);
  oResult->timestamp=0.1;
  return oResult;
}



int main(){
  int x=0, y=0;
 
  pthread_t thread;
  if (pthread_create(&thread, NULL, &threadFunction, &x)){ //x == thread function input argument
    cout << "Failed to create thread" << endl;
    return 1;
  }


  while (y<5){
    cout << "The value of x=" << x << "and y=" << y++ << endl;
    usleep(1000);

  }


  void *result;
  pthread_join(thread, &result); //result = thread function return value
  cResult *z=(cResult *)result;
  cout << "Final: x=" << x << ", y=" << y << endl;
  cout << "ThreadResult: number=" << z->number << " Timestamp=" << z->timestamp << endl;
  free(z);
  return 0;
}
