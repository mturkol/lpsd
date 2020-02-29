#include <stdio.h>
#include "debug.h"

/*
	write array data with length to file fn
*/
void ddump(double *data, int length, char *fn) {
	FILE *fp;
	int i;
	fp=fopen(fn,"w");
	for (i=0;i<length;i++) {
		fprintf(fp,"%g\n",data[i]);
	}
	fclose(fp);
}

/*
	write array data with length to file fn
*/
void idump(int *data, int length, char *fn) {
	FILE *fp;
	int i;
	fp=fopen(fn,"w");
	for (i=0;i<length;i++) {
		fprintf(fp,"%d\n",data[i]);
	}
	fclose(fp);
}
