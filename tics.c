#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "errors.h"
#include "config.h"
#include "tics.h"

static char tics[TICLEN];	/* contains gnuplot commands for nice tics */

void label(int i, int last)
{
	if (abs(i)>1)
		sprintf(&tics[strlen(tics)],"\"10^{%d}\" 1e%d", i, i);
	else {
		if (i==-1) sprintf(&tics[strlen(tics)],"\"0.1\"0.1");
		if (i==0) sprintf(&tics[strlen(tics)],"\"1\"1");
		if (i==1) sprintf(&tics[strlen(tics)],"\"10\"10");
	}	
	if (last)
		sprintf(&tics[strlen(tics)],")");
	else
		sprintf(&tics[strlen(tics)],",\\\n");
}

void sublabel(int i)
{
	int j;
	for (j = 2; j <= 9; j++)
		sprintf(&tics[strlen(tics)],"\"\" %de%d,\\\n", j, i);

}

/*
	creates a string with the gnuplot commands for nice tics
	
	arguments:
		axis "x" or "y"
		min min exponent
		max max exponent
	result
		string with gnuplot command for nice tics
*/
void maketics(char *s, char axis, int min, int max)
{
	int i;
	memset(tics,0,TICLEN);
	sprintf(tics, "set %ctics (\\\n", axis);
	for (i = min; i < max; i++) {
		label(i, 0);
		sublabel(i);
	}
	label(max, 1);
		
	strcpy(s,tics);
}

/*
int main(void) {
	char s[TICLEN];
	maketics(s,'y',-3,1);
	printf("%s\n",s);
	return(0);
}
*/
