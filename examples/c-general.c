##################
### mytest
##################


# # comment
#
# target:  dependency1 dependency2 ... dependencyn
#       <tab> command
# 
# # (note: the <tab> in the command line is necessary for make to work)
 

# the compiler: gcc for C program, define as g++ for C++
CC = gcc
 

# compiler flags:
#  -g     - this flag adds debugging information to the executable file
#  -Wall  - this flag is used to turn on most compiler warnings
# CFLAGS  = -g -Wall



# The build target
TARGET = mytest


all: $(TARGET)

 

$(TARGET): $(TARGET).c

                $(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

 

clean:

                $(RM) $(TARGET)

 

 

#GCC                     = gcc

#EXE             = mytest

#

#all : mytest

#

##mytest : $(GCC) -g -o $(EXE)

#

#clean :

#             rm $(EXE)

 

 

 

 

 

 

#include <stdint.h>

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

 

// https://www.geeksforgeeks.org/gdb-step-by-step-introduction/

 

static char hello_world[]="Hello World\n";

char name[]="Peter";

 

int *ptr;

int n=10;

int i;

 

struct person

{

  int age;

  float weight;

};

 

static struct person Dagmar = {

  .age=58,

  .weight=62.0,

 

};

 

void UpdateAge(struct person *MyPerson, int delta)

{

  MyPerson->age = MyPerson->age + delta;

}

 

struct person * CreatePerson(int age, int weight)

{

  struct person * pNewPerson;

  pNewPerson = (struct person*)malloc(1 * sizeof(struct person));

  if (pNewPerson==NULL) {

    printf("Memory not allocated.\n");

    return NULL;

  }

  pNewPerson->age=age;

  pNewPerson->weight=weight;

  return pNewPerson;

}

 

typedef int *pint;

pint myintptr=NULL;

 

 

 

/* main.c */

int main(int argc, char *argv[]) {

 

  printf("Strings\n\n");

 

  printf("%s\n",hello_world);

  //int l1=strlen(hello_world);

  int l2=strlen(name);

  //printf("Len l1=%d\n",l1);

  printf("Len l2=%d\n",l2);

 

  ptr = (int*)malloc(n * sizeof(int));

  if (ptr==NULL) {

    printf("Memory not allocated.\n");

    exit(0);

  }

  else {

    for (i=0; i<n; i++) {

      ptr[i]=i+1;

    }

 

    for (i=0; i<n; i++) {

      printf("%d, ",ptr[i]);

    }

 

    printf("\n\n");

    free(ptr);

 

  }

 

 

  printf("Structures\n\n");

       

 struct person *pperson, person1;

  pperson = &person1;

  (*pperson).age=49;

  pperson->weight=75.0;

 

  printf("Person, age=%d, weight=%f\n",pperson->age,pperson->weight);

  UpdateAge(pperson,1);

  printf("Person, age=%d, weight=%f\n",pperson->age,pperson->weight);

 

  printf("Person, age=%d, weight=%f\n",Dagmar.age,Dagmar.weight);

 

  pperson=CreatePerson(10,20);

  printf("Person, age=%d, weight=%f\n",pperson->age,pperson->weight);

  free(pperson);

 

  printf("Integers\n\n");

 

  myintptr=malloc(1 * sizeof(int));

  (*myintptr)=1973;

  printf("Int ptr = %d\n",*myintptr);

  free(myintptr);

 

 

 

 

 

 

}

 

 

 

 
	
