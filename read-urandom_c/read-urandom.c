/* Copyright (C) 2017, Chris Simmonds (chris@2net.co.uk) */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(void)
{
	int f;
	unsigned int rnd;
	int n;
	int i;

	f = open("/dev/urandom", O_RDONLY);
	if (f < 0) {
		perror("Failed to open urandom");
		return 1;
	}

	for (i=1;i<10;i++) {
	  n = read(f, &rnd, sizeof(rnd)); 
	  if (n != sizeof(rnd)) {
	    perror("Problem reading urandom");
	    return 1;
	  }
	  printf("Random number = 0x%x\n", rnd);
	}
	
	close(f);
	return 0;
}

